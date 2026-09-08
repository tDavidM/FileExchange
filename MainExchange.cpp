//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include <dirent.h>
#include "MainExchange.h"
#include <shellapi.h>
#include <FileCtrl.hpp>
#include <sys/stat.h>
#include <utime.h>
#include <stdexcept>

#include "Crypto.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMainForm *MainForm;
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
   : TForm(Owner)
{
   DragAcceptFiles(Handle, true);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::bStartServClick(TObject *Sender)
{
   (void)Sender;

   if (this->IdTCPFileServer->Active) {
      this->IdTCPFileServer->Active = false;
      this->mServerOut->Lines->Add(MSG_SRV_STOP);
      this->bStartServ->Caption = CAP_MAINFORM_START_SVR;
      this->lblServRoot->Caption = CAP_OUTPUT_ROOT;
   } else {
      //ExtractFilePath(this->TxtFich->Text)
      if (SelectDirectory(STR_ROOTDIR_DIAG,"", this->outputPath)) {
         this->IdTCPFileServer->Active = true;
         this->mServerOut->Lines->Add(MSG_SRV_START_PORT + this->edtPort->Text);
         this->mServerOut->Lines->Add(MSG_SRV_START_DIR + this->outputPath);
         if (this->disableFileTime) {
            this->lblServRoot->Caption = CAP_SRV_DIR_IND + this->outputPath;
         } else {
            this->lblServRoot->Caption = this->outputPath;
         }
         this->bStartServ->Caption = CAP_MAINFORM_STOP_SVR;
      } else {
         this->mServerOut->Lines->Add(MSG_SRV_START_ERR);
      }
   }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::bCmdSendClick(TObject *Sender)
{
   (void)Sender;
   bool delaySend = false;
   
   if (this->edtIp->Text == "" ) {
      ShowMessage(MSG_SEND_IP_ERR);
   } else if (this->OpenAny->Execute() == true) { //multiselect
      this->edtPort->Enabled = false;
      fClipboard->bSend->Enabled = false;
      delaySend = this->fileSendList != NULL || this->cbWaitPoke->Checked;

      int cmpSearch = 0;
      for (int i=0; i<this->OpenAny->Files->Count; i++) {
         cmpSearch += this->SearchFiles(this->OpenAny->Files->Strings[i],
                                        ExtractFilePath(ExtractFilePath(this->OpenAny->Files->Strings[0])));
      }

      if (!delaySend) {
         this->fileSendCount = 0;
         this->prgBarSendTotal->Max = cmpSearch;
         this->SendFile();
      } else {
         this->prgBarSendTotal->Max += cmpSearch;
         if (this->cbWaitPoke->Checked) {
            this->mClientOut->Lines->Add(IntToStr(cmpSearch) + MSG_SEND_PENDING_FILE);
            this->fileSendCount = 0;
         }
      }
   }

   if (!delaySend) {
      this->edtPort->Enabled = true;
      fClipboard->bSend->Enabled = true;
      this->fileSendCount = 0;
      this->prgBarSendTotal->Position = 0;
      this->prgBarSendTotal->Max = 100;
   }
}
//---------------------------------------------------------------------------
void TMainForm::DropFiles(TMessage &message)
{
   bool delaySend = false;
   
   if (this->edtIp->Text == "" ) {
      ShowMessage(MSG_SEND_IP_ERR);
   } else {
      this->edtPort->Enabled = false;
      fClipboard->bSend->Enabled = false;
      int fileCount = DragQueryFile((HDROP)message.WParam, 0xFFFFFFFF, NULL, 0);
      delaySend = this->fileSendList != NULL || this->cbWaitPoke->Checked;

      int cmpSearch = 0;
      for (int i=0; i<fileCount; i++) {
         wchar_t buffer[65536];
         DragQueryFileW((HDROP)message.WParam, i, buffer, sizeof(buffer));
         cmpSearch += this->SearchFiles(buffer, ExtractFilePath(buffer));
      }
      DragFinish((HDROP)message.WParam);

      if (!delaySend) {
         this->fileSendCount   = 0;
         this->prgBarSendTotal->Max = cmpSearch;
         this->SendFile();
      } else {
         this->prgBarSendTotal->Max += cmpSearch;
         if (this->cbWaitPoke->Checked) {
            this->mClientOut->Lines->Add(IntToStr(cmpSearch) + MSG_SEND_PENDING_FILE);
            this->fileSendCount = 0;
         }
      }
   }

   if (!delaySend) {
      this->edtPort->Enabled = true;
      fClipboard->bSend->Enabled = true;
      this->fileSendCount = 0;
      this->prgBarSendTotal->Position = 0;
      this->prgBarSendTotal->Max = 100;
   }
}
//---------------------------------------------------------------------------
int TMainForm::SearchFiles(String filePath, String fileRoot)
{
   tFileList *currentFile, *lastFile;

   if(DirectoryExists(filePath)) {
      DIR *currentDir;
      

      if ((currentDir = opendir(AnsiString(filePath).c_str())) == NULL) {
         exit(1);
      }

      int cmp = 0;
      struct dirent *dirEntry;
      while ((dirEntry = readdir(currentDir)) != NULL) {
         if (strcmp(dirEntry->d_name,".") != 0 && strcmp(dirEntry->d_name,"..") != 0) {
            cmp += SearchFiles(filePath + "\\" + dirEntry->d_name, fileRoot);
         }
      }
      closedir(currentDir);
      return cmp;
   } else {
      currentFile = new tFileList;
      currentFile->filePath = filePath;
      currentFile->fileRoot = fileRoot;

      String filePathBuff = filePath;
      HANDLE fileHandle = CreateFileW(filePathBuff.c_str(), GENERIC_READ | FILE_WRITE_ATTRIBUTES, 0, NULL, OPEN_EXISTING, 0, NULL);

      FILETIME modTime;
      GetFileTime(fileHandle, NULL, NULL, &modTime);
      //currentFile->Time = *(uint64_t*)&modTime;
      memcpy(&currentFile->fileTime, &modTime, sizeof(uint64_t));

      CloseHandle(fileHandle);

      int fileID;
      try {
         fileID = FileOpen(filePath, fmOpenRead);
         if ( fileID >= 0) {
            currentFile->fileSize = FileSeek(fileID, (__int64)0, 2);
         } else {
            currentFile->fileSize = 0;
         }
         FileClose( fileID );
      }
      catch(Exception& e)
      {
         currentFile->fileSize = 0;
         FileClose( fileID );
      }

      currentFile->nextFile = NULL;

      if (this->fileSendList != NULL) {
         lastFile = this->fileSendList;
         while(lastFile->nextFile != NULL) {
            lastFile = lastFile->nextFile;
         }
         lastFile->nextFile = currentFile;
      } else {
        this->fileSendList = currentFile;
      }

      Application->ProcessMessages();
      return 1;
   }
}

//---------------------------------------------------------------------------
void TMainForm::SendFile()
{
   bool msgCheckKeyDone = false;
   this->mClientOut->Lines->Add(MSG_SEND_START);

   while(this->fileSendList != NULL)
   {
      TFileStream* fileID = NULL;
      try
      {
         this->IdTCPFileClient->Host = edtIp->Text;
         this->IdTCPFileClient->Connect();
         this->mClientOut->Lines->Add(MSG_SEND_PROCESSING + this->IdTCPFileClient->Socket->Binding->PeerIP + "): " +
                                     ExtractFileName(this->fileSendList->filePath));
         this->prgBarSendPart->Max = this->fileSendList->fileSize/PAYLOADSIZE;
         this->prgBarSendPart->Position = 0;

         Application->ProcessMessages();
         byte fileIV[FILE_IV_SIZE];
         byte fileKey[AES_KEY_SIZE];
         SendMetaInfo(fileIV, fileKey);

         String netData = this->IdTCPFileClient->Socket->ReadLn();
         this->mClientOut->Lines->Add(MSG_SEND_SRV_INFO + netData);
         if (netData != PROTOCOL_MSG_OK){
            this->IdTCPFileClient->Disconnect();
            if (netData == PROTOCOL_MSG_META_HMAC_ERR) {
               if (!msgCheckKeyDone) {
                  ShowMessage(MSG_SEND_CHECK_KEY);
               }
               msgCheckKeyDone = true;
            }
            throw Exception(MSG_SEND_META_ERR);
         }

         fileID = new TFileStream(this->fileSendList->filePath, fmOpenRead);
         int32_t bufferSize = 0;
         int32_t dataSize = 0;
         uint32_t counterHMAC = 0;
         byte bufferHMAC[FILE_DATA_HMAC_SIZE+PAYLOADSIZE+sizeof(counterHMAC)];
         byte fileHMAC[FILE_DATA_HMAC_SIZE];
         memset(fileHMAC, 0x00, FILE_DATA_HMAC_SIZE);

         for (int32_t i=this->fileSendList->fileSize ; i>0 ; i-=dataSize) {
            dataSize = i>=PAYLOADSIZE?PAYLOADSIZE:i;
            bufferSize = dataSize;

            TIdBytes buffer;
            buffer.Length = bufferSize;
            fileID->Read(buffer, dataSize);

            if (this->cbEncrypt->Checked) {
               bufferSize = PAYLOADSIZE;
               buffer.Length = bufferSize;
               byte* bufferPtr = &buffer[0];

               if (dataSize != bufferSize) {
                  memset(bufferPtr+dataSize, 0x00, bufferSize-dataSize);
               }

               RunCipherIV(bufferPtr, PAYLOADSIZE, fileIV, fileKey);

               memcpy(bufferHMAC, fileHMAC, FILE_DATA_HMAC_SIZE);
               memcpy(bufferHMAC+FILE_DATA_HMAC_SIZE, bufferPtr, PAYLOADSIZE);
               memcpy(bufferHMAC+FILE_DATA_HMAC_SIZE+PAYLOADSIZE, reinterpret_cast<void*>(&counterHMAC), sizeof(counterHMAC));

               GetHMAC_Tiger(fileKey, SHA256_HASH_SIZE, bufferHMAC, sizeof(bufferHMAC), fileHMAC);
               counterHMAC++;

               bufferSize += FILE_DATA_HMAC_SIZE;
               buffer.Length = bufferSize;
               bufferPtr = &buffer[0];

               memcpy(bufferPtr+PAYLOADSIZE, fileHMAC, FILE_DATA_HMAC_SIZE);
               //Move(fileHMAC, &buffer[PAYLOADSIZE], FILE_DATA_HMAC_SIZE);
            }

            this->IdTCPFileClient->Socket->Write(buffer, bufferSize);

            this->prgBarSendPart->Position += 1;
            Application->ProcessMessages();

            if (i%32768 == 0) {
               netData = this->IdTCPFileClient->Socket->ReadLn();

               if (netData != PROTOCOL_MSG_OK){
                  this->IdTCPFileClient->Disconnect();
                  throw Exception(MSG_SEND_DATA_ERR);
               }

               this->Invalidate();
               Application->ProcessMessages();
            }
         }
         fileID->Free();
         netData = this->IdTCPFileClient->Socket->ReadLn();

         this->mClientOut->Lines->Add(MSG_SEND_DONE + this->fileSendList->filePath);

         this->prgBarSendPart->Position = 0;
         this->fileSendCount++;
         this->prgBarSendTotal->Position = this->fileSendCount;
         Application->ProcessMessages();

         tFileList *FileDone = this->fileSendList;
         this->fileSendList = this->fileSendList->nextFile;
         delete FileDone;

         this->IdTCPFileClient->Disconnect();
      }
      catch(Exception& e)
      {
         if (fileID != NULL) {
            fileID->Free();
         }
         this->IdTCPFileClient->Disconnect();

         this->mClientOut->Lines->Add(MSG_SEND_ERR + this->fileSendList->filePath);
         this->mClientOut->Lines->Add(e.Message);

         if (this->fileSendList != NULL) {
            tFileList *fileDone = this->fileSendList;
            this->fileSendList = this->fileSendList->nextFile;
            delete fileDone;
         }

         this->prgBarSendPart->Position = 0;
         this->fileSendCount++;
         this->prgBarSendTotal->Position = this->fileSendCount;
         Application->ProcessMessages();
      }
   }
}

//---------------------------------------------------------------------------
void TMainForm::SendMetaInfo(byte* fileIV, byte* fileKey)
{
   byte fileNonce[FILE_NONCE_SIZE];

   if (this->cbEncrypt->Checked) {
      if (!GetSecureRandomBytes(fileNonce, FILE_NONCE_SIZE)) {
         throw Exception(MSG_RNGEN_ERR);
      }
   } else {
      memset(fileNonce, 0x00, FILE_NONCE_SIZE);
   } 

   UTF8String filePath_UTF8 = AnsiReplaceText(this->fileSendList->filePath, this->fileSendList->fileRoot, "");

   if (filePath_UTF8.Length() > MAXFILEPATH) {
      throw Exception(MSG_SEND_PATHLENGTH_ERR);
   }

   if (this->cbEncrypt->Checked) {
      if (!GetSecureRandomBytes(fileIV, FILE_IV_SIZE)) {
         throw Exception(MSG_RNGEN_ERR);
      }
   } else {
      memset(fileIV, 0x00, FILE_IV_SIZE);
   }

   byte metaBuffer[ARGON_SALT_SIZE + SESSION_SALT_SIZE + FILE_NONCE_SIZE +
                   sizeof(uint64_t) + sizeof(uint64_t) + MAXFILEPATH +
                   FILE_IV_SIZE + FILE_META_HMAC_SIZE];
   uint32_t metaSize = 0;

   memcpy(metaBuffer,          this->argonSalt, ARGON_SALT_SIZE);
   metaSize += ARGON_SALT_SIZE;
   memcpy(metaBuffer+metaSize, this->sessionSalt, SESSION_SALT_SIZE);
   metaSize += SESSION_SALT_SIZE;
   memcpy(metaBuffer+metaSize, fileNonce, FILE_NONCE_SIZE);
   metaSize += FILE_NONCE_SIZE;

   byte fileMeta[sizeof(uint64_t) + sizeof(uint64_t) + MAXFILEPATH];
   
   memset(fileMeta, 0x00, sizeof(fileMeta));
   memcpy(fileMeta, &this->fileSendList->fileSize, sizeof(uint64_t));
   memcpy(fileMeta+sizeof(uint64_t), &this->fileSendList->fileTime, sizeof(uint64_t));
   memcpy(fileMeta+sizeof(uint64_t)+sizeof(uint64_t), filePath_UTF8.c_str(), filePath_UTF8.Length());

   memcpy(metaBuffer+metaSize, fileMeta,sizeof(fileMeta));
   metaSize += sizeof(fileMeta);

   memcpy(metaBuffer+metaSize, fileIV, FILE_IV_SIZE);
   metaSize += FILE_IV_SIZE;

   if (this->cbEncrypt->Checked) {
      byte metaKey[SHA256_HASH_SIZE];
      GetHMAC_SHA256(this->rootPRK, SHA256_HASH_SIZE, fileNonce, FILE_NONCE_SIZE, metaKey);

      RunCipherNonce(metaBuffer+ARGON_SALT_SIZE+SESSION_SALT_SIZE+FILE_NONCE_SIZE,
                      metaSize-(ARGON_SALT_SIZE+SESSION_SALT_SIZE+FILE_NONCE_SIZE), fileNonce, metaKey);

      byte metaHMAC[FILE_META_HMAC_SIZE];
      GetHMAC_Tiger(metaKey, SHA256_HASH_SIZE, metaBuffer, metaSize, metaHMAC);
      memcpy(metaBuffer+metaSize, metaHMAC, FILE_META_HMAC_SIZE);
   } else {
      memset(metaBuffer+metaSize, 0x00, FILE_META_HMAC_SIZE);
   }

   if (this->cbEncrypt->Checked) {
      byte fileKeyInput[ARGON_HASH_SIZE+SESSION_SALT_SIZE+sizeof(fileMeta)];

      memcpy(fileKeyInput, this->rootKey, ARGON_HASH_SIZE);
      memcpy(fileKeyInput+ARGON_HASH_SIZE, this->sessionSalt, SESSION_SALT_SIZE);
      memcpy(fileKeyInput+ARGON_HASH_SIZE+SESSION_SALT_SIZE, fileMeta, sizeof(fileMeta));

      SetPasswordHash(fileKeyInput, sizeof(fileKeyInput), this->argonSalt, fileKey);
   }   
         
   //buffer.Length = sizeof(metaBuffer);
   //memcpy(&buffer[0], metaBuffer, sizeof(metaBuffer));
   TIdBytes buffer = RawToBytes(metaBuffer, sizeof(metaBuffer));
   this->IdTCPFileClient->Socket->Write(buffer);
}

//---------------------------------------------------------------------------
void TMainForm::SendMessage(String message)
{
   String IPAddr = "";

   if (this->edtIp->Text != "") {
      IPAddr = edtIp->Text;
   } else {
      for (int i = 0; i<this->lbLocalNet->Count ; i++) {
         if (this->lbLocalNet->Selected[i]) {
            String value = this->lbLocalNet->Items->Strings[i];
            IPAddr = value.SubString(Pos( "(", value)+1, value.Length()-Pos("(",value)-1);
         }
      }
   }

   if (message.Length() == 0 || IPAddr.Length() == 0) {
      return;
   }

   this->mClientOut->Lines->Add(MSG_SEND_MSG);
   try
   {
      this->mClientOut->Lines->Add(MSG_SEND_SRV_INFO + IPAddr);
      this->IdUDPNetClient->Host = IPAddr;
      this->IdUDPNetClient->BroadcastEnabled = false;
      this->IdUDPNetClient->Active = true;
      
      if (cbEncrypt->Checked) {
         byte msgNonce[FILE_NONCE_SIZE];
         byte msgKey[SHA256_HASH_SIZE];
         byte msgHMAC[FILE_META_HMAC_SIZE];

         if (!GetSecureRandomBytes(msgNonce, FILE_NONCE_SIZE)) {
            throw Exception(MSG_RNGEN_ERR);
         }
         GetHMAC_SHA256(this->rootPRK, SHA256_HASH_SIZE, msgNonce, FILE_NONCE_SIZE, msgKey);

         UTF8String message_UTF8 = message;
         message_UTF8 = message_UTF8.SubString(1, MAXMESSAGESIZE);

         byte buffer[FILE_NONCE_SIZE+MAXMESSAGESIZE+FILE_META_HMAC_SIZE];

         memset(buffer, 0x00, sizeof(buffer));
         memcpy(buffer, msgNonce, FILE_NONCE_SIZE);
         memcpy(buffer+FILE_NONCE_SIZE, message_UTF8.c_str(), message_UTF8.Length());

         RunCipherNonce(buffer+FILE_NONCE_SIZE, MAXMESSAGESIZE, msgNonce, msgKey);

         GetHMAC_Tiger(msgKey, SHA256_HASH_SIZE, buffer, FILE_NONCE_SIZE+MAXMESSAGESIZE, msgHMAC);
         memcpy(buffer+FILE_NONCE_SIZE+MAXMESSAGESIZE, msgHMAC, FILE_META_HMAC_SIZE);

         String Data = "";
         for (unsigned int i=0; i<sizeof(buffer); i++) {
            Data += LowerCase(ByteToHex(buffer[i]));
         }

         this->IdUDPNetClient->Send(PROTOCOL_MSG_KEYWORD + Data);
      } else {
         message = message.SubString(1, MAXMESSAGESIZE);
         this->IdUDPNetClient->Send(PROTOCOL_MSG_KEYWORD + message);
      }

      this->IdUDPNetClient->Active = false;
      this->IdUDPNetClient->BroadcastEnabled = true;
      this->IdUDPNetClient->Host = "";
   }
   catch(Exception& e)
   {
      this->mClientOut->Lines->Add(MSG_SEND_MSG_ERR);
      this->mClientOut->Lines->Add(e.Message);
   }
}

//---------------------------------------------------------------------------
void __fastcall TMainForm::SpeedUpdateTimer(TObject *Sender)
{
   (void)Sender;
   float debit;
   double interval = 0.75;//(this->SpeedUpdate->Interval / 1000);

   debit = ((this->prgBarSendPart->Position - this->lastPGSendPos)*PAYLOADSIZE) / interval;
   debit = abs((long)debit);
   this->lastPGSendPos = this->prgBarSendPart->Position;
   if(debit<1024 && debit!=0) {
      lblSendSpeed->Caption=FloatToStr(RoundTo(debit,-2))+CAP_SPEED_B;
   } else {
      debit/=1024;
      if(debit<1024) {
         lblSendSpeed->Caption=FloatToStr(RoundTo(debit,-2))+CAP_SPEED_KB;
      } else {
         debit/=1024;
         if(debit<1024) {
            lblSendSpeed->Caption=FloatToStr(RoundTo(debit,-2))+CAP_SPEED_MB;
         }
      }
   }

   debit = ((this->prgBarRecvPart->Position - this->lastPGRecvPos)*PAYLOADSIZE) / interval;
   debit = abs((long)debit);
   this->lastPGRecvPos = this->prgBarRecvPart->Position;
   if(debit<1024 && debit!=0) {
      lblRecvSpeed->Caption=FloatToStr(RoundTo(debit,-2))+CAP_SPEED_B;
   } else {
      debit/=1024;
      if(debit<1024) {
         lblRecvSpeed->Caption=FloatToStr(RoundTo(debit,-2))+CAP_SPEED_KB;
      } else {
         debit/=1024;
         if(debit<1024) {
            lblRecvSpeed->Caption=FloatToStr(RoundTo(debit,-2))+CAP_SPEED_MB;
         }
      }
   }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormCreate(TObject *Sender)
{
   (void)Sender;
   TCHAR compName[2048];
   DWORD compNameCharCount = sizeof(compName);
   GetComputerName( compName, &compNameCharCount );
   this->edtComputerName->Text = compName;

   this->localNetList = new TStringList;
   this->localNetList->NameValueSeparator = '=';

   this->localNetTTL = new TStringList;
   this->localNetTTL->NameValueSeparator = '=';

   this->disableFileTime = false;

   this->lastPGSendPos = 0;
   this->lastPGRecvPos = 0;

   memset(this->rootPassword, 0x00, TIGER_HASH_SIZE);

   memset(this->sessionSalt, 0x00, SESSION_SALT_SIZE);
   memset(this->argonSalt, 0x00, ARGON_SALT_SIZE);
   memset(this->rootKey, 0x00, ARGON_HASH_SIZE);
   memset(this->rootPRK, 0x00, SHA256_HASH_SIZE);

   memset(this->receiveSessionSalt, 0x00, SESSION_SALT_SIZE);
   memset(this->receiveArgonSalt, 0x00, ARGON_SALT_SIZE);
   memset(this->receiveRootKey, 0x00, ARGON_HASH_SIZE);
   memset(this->receiveRootPRK, 0x00, SHA256_HASH_SIZE);

   try {
      this->IdUDPNetServer->Active = true;
   } catch (Exception& e) {
      this->IdUDPNetServer->Active = false;
   }

   this->Caption = CAP_MAINFORM;

   this->gbServ->Caption = CAP_MAINFORM_GBSERV;
   this->lblLocalIP->Caption = CAP_MAINFORM_LOCIP;
   this->bStartServ->Caption = CAP_MAINFORM_START_SVR;
   this->mServerOut->Hint = STR_MEMO_HINT;
   this->lblServRoot->Caption = CAP_MAINFORM_SRV_ROOT;
   this->lblRecv->Caption = CAP_MAINFORM_RSCV_SPEED;
   this->lblRecvSpeed->Caption = STR_SPEED_INIT;
   this->cbFileTime->Caption = CAP_MAINFORM_FILE_TIME;

   this->gbSend->Caption = CAP_MAINFORM_GBSEND;
   this->lblMsgIp->Caption = CAP_MAINFORM_DESTIP;
   this->bCmdSend->Caption = CAP_MAINFORM_SEND;
   this->mClientOut->Hint = STR_MEMO_HINT;
   this->lblSend->Caption = CAP_MAINFORM_SEND_SPEED;
   this->lblSendSpeed->Caption = STR_SPEED_INIT;
   this->cbWaitPoke->Caption = CAP_MAINFORM_WAIT_POKE;

   this->gbDiscover->Caption = CAP_MAINFORM_NETWORK;
   this->lblMsgPort->Caption = CAP_MAINFORM_PORT;
   this->lblName->Caption = CAP_MAINFORM_NAME;
   this->cbEncrypt->Caption = CAP_MAINFORM_CRYPTO;
   this->bClear->Caption = CAP_MAINFORM_REFRESH;
   this->bPoke->Caption = CAP_MAINFORM_POKE;
   this->bSelect->Caption = CAP_MAINFORM_SELECT;
}
//---------------------------------------------------------------------------

 void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
   (void)Sender;
   (void)Action;

   crypto_wipe(this->rootPassword, TIGER_HASH_SIZE);
   
   crypto_wipe(this->rootKey, ARGON_HASH_SIZE);
   crypto_wipe(this->rootPRK, SHA256_HASH_SIZE);
   crypto_wipe(this->receiveRootKey, ARGON_HASH_SIZE);
   crypto_wipe(this->receiveRootPRK, SHA256_HASH_SIZE);
   
   this->IdTCPFileServer->Active = false;
   this->IdUDPNetServer->Active = false;
   delete this->localNetList;
   delete this->localNetTTL;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::edtPortExit(TObject *Sender)
{
   (void)Sender;

   this->IdUDPNetClient->Active = false;
   this->IdUDPNetServer->Active = false;
   this->IdUDPNetClient->Port   = StrToInt(edtPort->Text)-33;

   this->IdUDPNetServer->DefaultPort = StrToInt(edtPort->Text)-33;
   this->IdUDPNetServer->Active      = true;

   this->IdTCPFileClient->Port = StrToInt(edtPort->Text);

   bool active = this->IdTCPFileServer->Active;
   this->IdTCPFileServer->Active      = false;
   this->IdTCPFileServer->DefaultPort = StrToInt(edtPort->Text);
   this->IdTCPFileServer->Bindings->Clear();
   this->IdTCPFileServer->Bindings->Add()->SetBinding("0.0.0.0", StrToInt(edtPort->Text));
   //this->IdTCPFileServer->Bindings->Add();
   //this->IdTCPFileServer->Bindings->Items[0]->IP="0.0.0.0";
   //this->IdTCPFileServer->Bindings->Items[0]->Port=StrToInt(edtPort->Text);
   if (active) {
      this->IdTCPFileServer->Active = true;
      this->mServerOut->Lines->Add(MSG_SRV_PORT_CHANGE + edtPort->Text);
   }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::UpdateListTimer(TObject *Sender)
{
   (void)Sender;

   try {
      this->IdUDPNetClient->Active = true;
      this->IdUDPNetClient->Broadcast(PROTOCOL_IDENT_KEYWORD + this->IdIPWatch->CurrentIP + ">" + this->edtComputerName->Text, StrToInt(edtPort->Text)-33);
      this->IdUDPNetClient->Active = false;

      for (int i = 0; i<this->localNetTTL->Count ; i++) {
         int TTLCmp = StrToInt(this->localNetTTL->Values[this->localNetTTL->Names[i]]);
         if (TTLCmp <= 0) {
            this->localNetList->Delete(this->localNetList->IndexOfName(this->localNetTTL->Names[i]));
            this->localNetTTL->Delete(i);
            this->lbLocalNet->Clear();
            this->bPoke->Enabled = false;
            for (int j = 0; j<this->localNetList->Count ; j++)
               this->lbLocalNet->Items->Add(this->localNetList->Values[this->localNetList->Names[j]] + " (" + this->localNetList->Names[j] + ")");
         }
         else {
            this->localNetTTL->Values[this->localNetTTL->Names[i]] = IntToStr(TTLCmp-1);
         }
      }

      this->lblLocalIPVal->Caption = this->IdIPWatch->CurrentIP;
   } catch (Exception& e) {
      this->lblLocalIPVal->Caption = this->IdIPWatch->CurrentIP;
   }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::lbLocalNetClick(TObject *Sender)
{
   (void)Sender;

   for (int i = 0; i<this->lbLocalNet->Count ; i++) {
      if (this->lbLocalNet->Selected[i] && this->IdTCPFileServer->Active) {
         this->bPoke->Enabled = true;
      }
   }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::bPokeClick(TObject *Sender)
{
   (void)Sender;
   String IPAddr;

   for (int i = 0; i<this->lbLocalNet->Count ; i++) {
      if (this->lbLocalNet->Selected[i]) {
         String Data= this->lbLocalNet->Items->Strings[i];
         IPAddr = Data.SubString(Pos( "(", Data)+1, Data.Length()-Pos("(",Data)-1);
      }
   }

   try {
      this->UpdateList->Enabled = false;
      this->IdUDPNetClient->BroadcastEnabled = false;
      this->IdUDPNetClient->Host   = IPAddr;
      this->IdUDPNetClient->Active = true;
      this->IdUDPNetClient->Send(PROTOCOL_MSG_KEYWORD + this->IdIPWatch->CurrentIP + PROTOCOL_POKE_KEYWORD);
      this->IdUDPNetClient->Active = false;
      this->IdUDPNetClient->Host = "";
      this->IdUDPNetClient->BroadcastEnabled = true;
      this->UpdateList->Enabled = true;

   } catch (Exception& e) {
        this->UpdateList->Enabled = true;
   }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::IdUDPNetServerUDPRead(TIdUDPListenerThread *AThread, const TIdBytes AData,
          TIdSocketHandle *ABinding)
{
   (void)AThread;
   String rawData = BytesToString(AData);

   if (rawData.Pos(PROTOCOL_IDENT_KEYWORD) == 1) {
      String IPAddr = rawData.SubString(Pos(":",rawData)+1, Pos(">",rawData)-Pos(":",rawData)-1);
      String name   = rawData.SubString(Pos(">",rawData,2)+1,50);

      if (IPAddr != this->IdIPWatch->CurrentIP) {
         if (this->localNetList->IndexOfName(IPAddr) < 0) {
            //Name=Value
            this->localNetList->Add(IPAddr + "=" + name);
            //this->localNetList->Values[IPAddr] = name;
            this->localNetTTL->Add(IPAddr + "=3");
            this->lbLocalNet->Items->Add(name + " (" + IPAddr + ")");
         } else {
            this->localNetTTL->Values[IPAddr] = "3";
            if (this->localNetList->Values[IPAddr] != name) {
               this->localNetList->Values[IPAddr] = name;
               //this->localNetList->Delete(ListPos);
               //this->localNetList->Add(IPAddr + "=" + name);
               this->lbLocalNet->Clear();
               for (int i = 0; i<this->localNetList->Count ; i++)
                  this->lbLocalNet->Items->Add(this->localNetList->Values[this->localNetList->Names[i]] + " (" + this->localNetList->Names[i] + ")");
            }
         }
      }
   } else if (rawData.Pos(PROTOCOL_MSG_KEYWORD) == 1) {
      if (this->fileSendCount == 0 && rawData.SubString(rawData.Length() - 4, 5) == PROTOCOL_POKE_KEYWORD) {
         this->mServerOut->Lines->Add(MSG_RSCV_POKE + ABinding->PeerIP);

         TNotifyEvent cbWaitPokeClickEvent, edtIpChangeEvent; // Sorry -_-'
         cbWaitPokeClickEvent = this->cbWaitPoke->OnClick;
         edtIpChangeEvent     = this->edtIp->OnChange;
         this->cbWaitPoke->OnClick = NULL;
         this->edtIp->OnChange = NULL;

         this->edtIp->Text = rawData.SubString(Pos(":",rawData)+1, Pos(">",rawData)-Pos(":",rawData)-1);

         this->cbWaitPoke->Checked = false;
         this->cbWaitPoke->OnClick = cbWaitPokeClickEvent; // = cbWaitPokeClick;
         this->edtIp->OnChange     = edtIpChangeEvent; // = edtIpChange;

         this->SendFile();

         this->edtPort->Enabled = true;
         fClipboard->bSend->Enabled = true;
         this->fileSendCount = 0;
         this->prgBarSendTotal->Position = 0;
         this->prgBarSendTotal->Max = 100;
      } else {
         this->mServerOut->Lines->Add(MSG_RSCV_MSG + ABinding->PeerIP);
         rawData = rawData.SubString(Pos(":",rawData)+1, rawData.Length()-13);
         String data;

         if (cbEncrypt->Checked && rawData.Length() == static_cast<int>((FILE_NONCE_SIZE+MAXMESSAGESIZE+FILE_META_HMAC_SIZE)*2)) {
            byte msgNonce[FILE_NONCE_SIZE];
            byte msgKey[SHA256_HASH_SIZE];
            byte msgHMAC[FILE_META_HMAC_SIZE];
            byte buffer[FILE_NONCE_SIZE+MAXMESSAGESIZE+FILE_META_HMAC_SIZE];
            byte message[MAXMESSAGESIZE+1];

            try {
               int j = 0;
               for (int i=1; i<rawData.Length(); i+=2) {
                  String hexVal = "0x";
                  hexVal += rawData[i];
                  hexVal += rawData[i+1];
                  buffer[j] = static_cast<byte>(StrToInt(hexVal));
                  j++;
               }
            }
            catch (...) {
               this->mServerOut->Lines->Add(MSG_RSCV_MSG_HEX_ERR);
               return;
            }

            memcpy(msgNonce, buffer, FILE_NONCE_SIZE);
            GetHMAC_SHA256(this->rootPRK, SHA256_HASH_SIZE, msgNonce, FILE_NONCE_SIZE, msgKey);

            GetHMAC_Tiger(msgKey, SHA256_HASH_SIZE, buffer, FILE_NONCE_SIZE+MAXMESSAGESIZE, msgHMAC);
         
            if (memcmp(buffer+FILE_NONCE_SIZE+MAXMESSAGESIZE, msgHMAC, FILE_META_HMAC_SIZE) != 0) {
               this->mServerOut->Lines->Add(MSG_RSCV_MSG_HMAC_ERR);
               return;
            }

            RunCipherNonce(buffer+FILE_NONCE_SIZE, MAXMESSAGESIZE, msgNonce, msgKey);
            
            memcpy(message, buffer+FILE_NONCE_SIZE, sizeof(message)-1);
            message[MAXMESSAGESIZE] = 0x00;
            data = String((const char*)message);
         } else {
            data = rawData;
         }

         fClipboard->mText->Lines->Add(MSG_RSCV_MSG_DONE);
         fClipboard->mText->Lines->Add(data);
      }
   }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::bbMessageClick(TObject *Sender)
{
   (void)Sender;

   fClipboard->Show();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::mServerOutMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y)
{
   (void)Sender;
   (void)X;
   (void)Y;

   if (Button == mbLeft && Shift.Contains(ssCtrl))
      this->mServerOut->Lines->Clear();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::mClientOutMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y)
{
   (void)Sender;
   (void)X;
   (void)Y;

   if (Button == mbLeft && Shift.Contains(ssCtrl))
      this->mClientOut->Lines->Clear();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::cbWaitPokeClick(TObject *Sender)
{
   (void)Sender;

   if (this->cbWaitPoke->Checked)
      this->edtIp->Text = "0.0.0.0";
   else {
      if (this->edtIp->Text == "0.0.0.0")
         this->edtIp->Text = "";

      if (this->fileSendList != NULL) {
         tFileList *fileDel;
         while (this->fileSendList != NULL) {
            fileDel = this->fileSendList;
            this->fileSendList = this->fileSendList->nextFile;
            delete fileDel;
         }
         this->mClientOut->Lines->Add(MSG_RSCV_POKE_NO_FILE);
      }
   }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::edtIpChange(TObject *Sender)
{
   (void)Sender;

   if (this->edtIp->Text != "0.0.0.0") {
      this->cbWaitPoke->Checked = false;
   }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::bClearClick(TObject *Sender)
{
   (void)Sender;

   this->localNetList->Clear();
   this->localNetTTL->Clear();
   lbLocalNet->Clear();
   this->bPoke->Enabled = false;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::bSelectClick(TObject *Sender)
{
   (void)Sender;

   for (int i = 0; i<this->lbLocalNet->Count ; i++) {
      if (this->lbLocalNet->Selected[i]) {
         String data = this->lbLocalNet->Items->Strings[i];
         this->edtIp->Text = data.SubString(Pos( "(", data)+1, data.Length()-Pos("(",data)-1);
      }
   }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::lbLocalNetDblClick(TObject *Sender)
{
   (void)Sender;

   this->bSelect->Click();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::cbFileTimeClick(TObject *Sender)
{
   (void)Sender;

   this->disableFileTime = this->cbFileTime->Checked;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::IdTCPFileServerConnect(TIdContext *AContext)
{
   (void)AContext;

   this->edtPort->Enabled = false;
   //AContext->Binding()->PeerIP;
   this->mServerOut->Lines->Add(MSG_RSCV_START + AContext->Connection->Socket->Binding->PeerIP);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::IdTCPFileServerDisconnect(TIdContext *AContext)
{
   (void)AContext;

   this->edtPort->Enabled = true;
   //AContext->Binding()->PeerIP;
   this->mServerOut->Lines->Add(MSG_RSCV_STOP + AContext->Connection->Socket->Binding->PeerIP);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::IdTCPFileServerStatus(TObject *ASender, const TIdStatus AStatus,
          const UnicodeString AStatusText)
{
   (void)ASender;
   (void)AStatus;

   this->mServerOut->Lines->Add(MSG_RSCV_STATUS + AStatusText);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::IdTCPFileServerExecute(TIdContext *AContext)
{
   uint64_t fileSize;
   uint64_t fileTime;
   String fileName;
   byte fileIV[FILE_IV_SIZE];
   byte fileKey[AES_KEY_SIZE];

   if (!ReceiveMetaInfo(AContext, fileSize, fileTime, fileName, fileIV, fileKey)) {
      return;
   }
   
   this->mServerOut->Lines->Add(MSG_RSCV_FILE + fileName);

   String filePath = this->outputPath + "\\" + fileName;
   ForceDirectories(ExtractFilePath(filePath));

   TFileStream* fileID = new TFileStream( filePath, fmOpenWrite | fmCreate ); // fmOverwrite

   this->prgBarRecvPart->Max=fileSize/PAYLOADSIZE;
   this->prgBarRecvPart->Position=0;

   int32_t bufferSize = 0;
   int32_t dataSize = 0;
   uint32_t counterHMAC = 0;
   byte fileHMAC[FILE_DATA_HMAC_SIZE];
   memset(fileHMAC, 0x00, FILE_DATA_HMAC_SIZE);

   for (int32_t i=fileSize ; i>0 ; i-=dataSize) {
      dataSize = i>=PAYLOADSIZE?PAYLOADSIZE:i;
      bufferSize = dataSize;

      if (this->cbEncrypt->Checked) {
         bufferSize = PAYLOADSIZE + FILE_DATA_HMAC_SIZE;
      }

      TIdBytes buffer;
      buffer.Length = 0;

      AContext->Connection->Socket->ReadBytes(buffer, bufferSize, true);

      if (this->cbEncrypt->Checked) {
         byte* bufferPtr = &buffer[0];
         byte receiveFileHMAC[FILE_DATA_HMAC_SIZE];
         byte bufferHMAC[FILE_DATA_HMAC_SIZE+PAYLOADSIZE+sizeof(counterHMAC)];

         memcpy(receiveFileHMAC, bufferPtr+PAYLOADSIZE, FILE_DATA_HMAC_SIZE);

         memcpy(bufferHMAC, fileHMAC, FILE_DATA_HMAC_SIZE);
         memcpy(bufferHMAC+FILE_DATA_HMAC_SIZE, bufferPtr, PAYLOADSIZE);
         memcpy(bufferHMAC+FILE_DATA_HMAC_SIZE+PAYLOADSIZE, reinterpret_cast<void*>(&counterHMAC), sizeof(counterHMAC));

         GetHMAC_Tiger(fileKey, SHA256_HASH_SIZE, bufferHMAC, sizeof(bufferHMAC), fileHMAC);
         counterHMAC++;

         if (memcmp(receiveFileHMAC, fileHMAC, FILE_DATA_HMAC_SIZE) != 0) {
            this->mServerOut->Lines->Add(MSG_RSCV_FILE_ERR_HMAC + UIntToStr(counterHMAC));
            AContext->Connection->Socket->WriteLn(PROTOCOL_MSG_STOP);
            ShowMessage(MSG_RSCV_FILE_INVALID + outputPath + "\\" + fileName);
            break;
         }

         RunCipherIV(bufferPtr, PAYLOADSIZE, fileIV, fileKey);
      }

      fileID->WriteData(buffer, dataSize);
      this->prgBarRecvPart->Position+=1;

      if (i%32768 == 0) {
         AContext->Connection->Socket->WriteLn(PROTOCOL_MSG_OK);
         this->Invalidate();
         Application->ProcessMessages();
      }
   }
   fileID->Free();
   AContext->Connection->Socket->WriteLn(PROTOCOL_MSG_DONE);

   if (!disableFileTime) {
      FILETIME modTime;
      HANDLE fh = CreateFileW(filePath.c_str(), GENERIC_READ | FILE_WRITE_ATTRIBUTES, 0, NULL, OPEN_EXISTING, 0, NULL);

      memcpy(&modTime, &fileTime, sizeof(uint64_t));
      SetFileTime( fh, NULL, NULL, &modTime);
      CloseHandle(fh);
   }

   //AContext->Connection->Disconnect();
   this->prgBarRecvPart->Position=0;
   this->mServerOut->Lines->Add(MSG_RSCV_DONE + outputPath + "\\" + fileName);
}
//---------------------------------------------------------------------------

bool TMainForm::ReceiveMetaInfo(TIdContext *AContext, uint64_t &fileSize, uint64_t &fileTime, String &fileName, byte* fileIV, byte* fileKey)
{
   byte metaBuffer[ARGON_SALT_SIZE + SESSION_SALT_SIZE + FILE_NONCE_SIZE +
                   sizeof(uint64_t) + sizeof(uint64_t) + MAXFILEPATH +
                   FILE_IV_SIZE + FILE_META_HMAC_SIZE];
   TIdBytes buffer;

   buffer.Length = 0;
   AContext->Connection->Socket->ReadBytes(buffer, sizeof(metaBuffer), true);
   
   if (buffer.Length != static_cast<int>(sizeof(metaBuffer))) {
      AContext->Connection->Socket->WriteLn(PROTOCOL_MSG_META_SIZE_ERR);
      this->mServerOut->Lines->Add(MSG_RSCV_MSG_SIZE_ERR);
      AContext->Connection->Disconnect();
      return false;
   }

   memcpy(metaBuffer, &buffer[0], sizeof(metaBuffer));
   uint32_t metaSize = 0;

   if (this->cbEncrypt->Checked) {
      byte localArgonSalt[ARGON_SALT_SIZE];

      memcpy(localArgonSalt, metaBuffer, ARGON_SALT_SIZE);
      metaSize += ARGON_SALT_SIZE;

      if (memcmp(localArgonSalt, this->receiveArgonSalt, ARGON_SALT_SIZE) != 0) {
         memcpy(this->receiveArgonSalt, localArgonSalt, ARGON_SALT_SIZE);

         memcpy(this->receiveSessionSalt, metaBuffer+metaSize, SESSION_SALT_SIZE);
         metaSize += SESSION_SALT_SIZE;

         byte passwordHash[TIGER_HASH_SIZE+SESSION_SALT_SIZE];
         
         memcpy(passwordHash, this->rootPassword, TIGER_HASH_SIZE);
         memcpy(passwordHash+TIGER_HASH_SIZE, this->receiveSessionSalt, SESSION_SALT_SIZE);

         SetPasswordHash(passwordHash, sizeof(passwordHash), this->receiveArgonSalt, this->receiveRootKey);

         crypto_wipe(passwordHash, sizeof(passwordHash));

         GetHMAC_SHA256(this->receiveRootKey, ARGON_HASH_SIZE, this->receiveSessionSalt, SESSION_SALT_SIZE, this->receiveRootPRK);
      } else {
         metaSize += SESSION_SALT_SIZE;
      }

      byte fileNonce[FILE_NONCE_SIZE];
      memcpy(fileNonce, metaBuffer+metaSize, FILE_NONCE_SIZE);
      metaSize += FILE_NONCE_SIZE;

      byte metaKey[SHA256_HASH_SIZE];
      GetHMAC_SHA256(this->receiveRootPRK, SHA256_HASH_SIZE, fileNonce, FILE_NONCE_SIZE, metaKey);

      byte metaHMAC[FILE_META_HMAC_SIZE];
      metaSize += sizeof(uint64_t) + sizeof(uint64_t) + MAXFILEPATH + FILE_IV_SIZE;
      GetHMAC_Tiger(metaKey, SHA256_HASH_SIZE, metaBuffer, metaSize, metaHMAC);
      
      if (memcmp(metaBuffer+metaSize, metaHMAC, FILE_META_HMAC_SIZE) != 0) {
         AContext->Connection->Socket->WriteLn(PROTOCOL_MSG_META_HMAC_ERR);
         this->mServerOut->Lines->Add(MSG_SEND_CHECK_KEY);
         AContext->Connection->Disconnect();
         return false;
      }

      RunCipherNonce(metaBuffer+ARGON_SALT_SIZE+SESSION_SALT_SIZE+FILE_NONCE_SIZE,
                     metaSize, fileNonce, metaKey);
   }

   byte fileMeta[sizeof(uint64_t) + sizeof(uint64_t) + MAXFILEPATH];
   metaSize = ARGON_SALT_SIZE + SESSION_SALT_SIZE + FILE_NONCE_SIZE;
   memcpy(fileMeta, metaBuffer+metaSize, sizeof(uint64_t) + sizeof(uint64_t) + sizeof(fileMeta));

   fileSize = 0;
   fileTime = 0;

   memcpy(reinterpret_cast<void*>(&fileSize), metaBuffer+metaSize, sizeof(uint64_t));
   metaSize += sizeof(uint64_t);
   memcpy(reinterpret_cast<void*>(&fileTime), metaBuffer+metaSize, sizeof(uint64_t));
   metaSize += sizeof(uint64_t);

   byte filePath[MAXFILEPATH+1];

   memcpy(filePath, metaBuffer+metaSize, sizeof(filePath)-1);
   filePath[MAXFILEPATH] = 0x00;
   fileName = String((const char*)filePath);
   metaSize += MAXFILEPATH;

   memcpy(fileIV, metaBuffer+metaSize, FILE_IV_SIZE);
   metaSize += FILE_IV_SIZE;

   if (this->cbEncrypt->Checked) { 
      byte fileKeyInput[ARGON_HASH_SIZE+SESSION_SALT_SIZE+sizeof(fileMeta)];

      memcpy(fileKeyInput, this->receiveRootKey, ARGON_HASH_SIZE);
      memcpy(fileKeyInput+ARGON_HASH_SIZE, this->receiveSessionSalt, SESSION_SALT_SIZE);
      memcpy(fileKeyInput+ARGON_HASH_SIZE+SESSION_SALT_SIZE, fileMeta, sizeof(fileMeta));

      SetPasswordHash(fileKeyInput, sizeof(fileKeyInput), this->receiveArgonSalt, fileKey); 
   }
   
   AContext->Connection->Socket->WriteLn(PROTOCOL_MSG_OK);
   return true;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::lblMsgIpDblClick(TObject *Sender)
{
   (void)Sender;

   this->edtIp->Text = "127.0.0.1";
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::lblNameDblClick(TObject *Sender)
{
   (void)Sender;

   this->IdUDPNetServer->Active = false;
   this->edtComputerName->Enabled = false;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::cbEncryptClick(TObject *Sender)
{
   (void)Sender;
   TfSelPassKey *fSelPassKey = new TfSelPassKey( this );

   if (this->cbEncrypt->Checked) {
      try {
         if (fSelPassKey->ShowModal() == mrOk) {
            memcpy(this->rootPassword, fSelPassKey->rootPassword, TIGER_HASH_SIZE);
            memcpy(this->sessionSalt,  fSelPassKey->sessionSalt,  SESSION_SALT_SIZE);
            memcpy(this->argonSalt,    fSelPassKey->argonSalt,    ARGON_SALT_SIZE);
            memcpy(this->rootKey,      fSelPassKey->sessionKey,   ARGON_HASH_SIZE);
            
            crypto_wipe(fSelPassKey->rootPassword, sizeof(fSelPassKey->rootPassword));
            crypto_wipe(fSelPassKey->sessionKey,   sizeof(fSelPassKey->sessionKey));

            GetHMAC_SHA256(this->rootKey, ARGON_HASH_SIZE, this->sessionSalt, SESSION_SALT_SIZE, this->rootPRK);
         } else {
            this->cbEncrypt->Checked = false;

            crypto_wipe(this->rootPassword, TIGER_HASH_SIZE);

            crypto_wipe(this->rootKey, ARGON_HASH_SIZE);
            crypto_wipe(this->rootPRK, SHA256_HASH_SIZE);
            crypto_wipe(this->receiveRootKey, ARGON_HASH_SIZE);
            crypto_wipe(this->receiveRootPRK, SHA256_HASH_SIZE);
         }
      } catch(Exception& e) {
         this->cbEncrypt->Checked = false;
      }
   }

   delete fSelPassKey;
}
//---------------------------------------------------------------------------

