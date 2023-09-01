//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include <dirent.h>
#include "MainExchange.h"
#include <shellapi.h>
#include <FileCtrl.hpp>
//#include <System.Hash.hpp>
#include <sys/stat.h>
#include <utime.h>

#include "sboxes.c"
#include "tiger.c"

#define PAYLOADSIZE 4096
#define DIGESTSIZE 24

typedef unsigned long long int word64;
typedef unsigned char byte;

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMainForm *MainForm;
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
   : TForm(Owner)
{
   DragAcceptFiles(Handle, true);
   LastPGSendPos=0;
   LastPGRecvPos=0;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::CmdServClick(TObject *Sender)
{
   if (this->IdTCPFileServer->Active) {
      this->IdTCPFileServer->Active = false;
      this->ServerOut->Lines->Add("Deactivate");
      this->CmdServ->Caption = "Activate";
      this->lServRoot->Caption = "Target Directory:";
   } else {
      //ExtractFilePath(this->TxtFich->Text)
      if(SelectDirectory("Select the Directory where the files will be placed","", PathSortie)) {
         //PntServ = new ThServ(false,sortie+"\\");
         IdTCPFileServer->Active = true;
         this->ServerOut->Lines->Add("Activate, port: " + TxtPort->Text);
         this->ServerOut->Lines->Add("   Root: " + PathSortie);
         if (DisableFileTime)
            this->lServRoot->Caption = "* " + PathSortie;
         else
            this->lServRoot->Caption = PathSortie;
         this->CmdServ->Caption = "Deactivate";
      }
      else
         this->ServerOut->Lines->Add("Error:No output specified !");
   }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::CmdSendClick(TObject *Sender)
{
   if (this->TxtIp->Text == "" )
      ShowMessage("Specify the IP address of the server");
   else if (this->OpenAny->Execute() == true) {
      this->TxtPort->Enabled = false;
      //multiselect
      this->FilePile  = NULL;
      this->CmpSearch = 0;
      this->CmpSend   = 0;
      for (int i=0; i<this->OpenAny->Files->Count; i++) {
         this->SearchFiles(this->OpenAny->Files->Strings[i]);
      }
      this->PGBarSendTotal->Max = this->CmpSearch;
      this->SendFile(ExtractFilePath(ExtractFilePath(this->OpenAny->Files->Strings[0])));
   }
   this->TxtPort->Enabled = true;

   this->CmpSearch = 0;
   this->CmpSend = 0;
   this->PGBarSendTotal->Position = 0;
   this->PGBarSendTotal->Max = 100;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::DropFiles(TMessage &Message)
{
   int nFiles;
   //char buffer[65536];
   wchar_t buffer[65536];
   this->FilePile = NULL;
   if (this->TxtIp->Text != "" ) {
      this->TxtPort->Enabled = false;
      nFiles = DragQueryFile((HDROP)Message.WParam, 0xFFFFFFFF, NULL, 0);
      this->CmpSearch = 0;
      this->CmpSend   = 0;
      for (int i=0; i<nFiles; i++) {
         DragQueryFileW((HDROP)Message.WParam, i, buffer, 65536);
         this->SearchFiles(buffer);
      }
      DragFinish((HDROP)Message.WParam);
      this->PGBarSendTotal->Max = this->CmpSearch;
      this->SendFile(ExtractFilePath(buffer));
   } else {
      ShowMessage("Specify the IP address of the server");
   }
   this->TxtPort->Enabled = true;

   this->CmpSearch = 0;
   this->CmpSend = 0;
   this->PGBarSendTotal->Position = 0;
   this->PGBarSendTotal->Max = 100;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::SearchFiles(String FilePath)
{
   FileStack *New;
   //FileStack *Curr;
   int FHandle;
   FILETIME modtime;

   if(DirectoryExists(FilePath)) {
      DIR *dir;
      struct dirent *ent;
      if ((dir = opendir(AnsiString(FilePath).c_str())) == NULL)
         exit(1);

      while ((ent = readdir(dir)) != NULL) {
         if (strcmp(ent->d_name,".") != 0 && strcmp(ent->d_name,"..") != 0)
            SearchFiles(FilePath + "\\" + ent->d_name);
      }
      closedir(dir);
   } else {
      New = new FileStack;
      New->Path = FilePath;

      String FilePathBuff = FilePath;
      HANDLE fh = CreateFileW(FilePathBuff.c_str(), GENERIC_READ | FILE_WRITE_ATTRIBUTES, 0, NULL, OPEN_EXISTING, 0, NULL);
      GetFileTime(fh, NULL, NULL, &modtime);
      New->TimeLow  = modtime.dwLowDateTime;
      New->TimeHigh = modtime.dwHighDateTime;
      CloseHandle(fh);

      try {
         FHandle = FileOpen(FilePath, fmOpenRead);
         if ( FHandle >= 0)
            New->Size = FileSeek(FHandle, (__int64)0, 2);
         else
            New->Size = 0;
         FileClose( FHandle );
      }
      catch(Exception& e)
      {
         New->Size = 0;
         FileClose( FHandle );
      }

      New->Next = this->FilePile;
      this->FilePile = New;
      this->CmpSearch++;
   }
}

//---------------------------------------------------------------------------
void __fastcall TMainForm::SendFile(String FileRoot)
{
   String FilePath, Data;
   int ReadSize;
   //char CharBuffer[PAYLOADSIZE];
   TIdBytes Buffer;
   TFileStream*Fichier=0;
   TByteDynArray RunningKey;

   this->ClientOut->Lines->Add("Sending...");

   while(this->FilePile != NULL)
   {
      try
      {
         IdTCPFileClient->Host = TxtIp->Text;
         IdTCPFileClient->Connect();
         this->ClientOut->Lines->Add("Working (" + IdTCPFileClient->Socket->Binding->PeerIP + "): " +
                                     ExtractFileName(this->FilePile->Path));
         this->PGBarSendPart->Max = this->FilePile->Size/PAYLOADSIZE;
         this->PGBarSendPart->Position = 0;

         Application->ProcessMessages();

         FilePath = AnsiReplaceText(this->FilePile->Path, FileRoot, "");

         IdTCPFileClient->Socket->WriteLn(IntToStr((__int64)this->FilePile->Size));
         IdTCPFileClient->Socket->WriteLn(FilePath, TIdTextEncoding_UTF8);
         IdTCPFileClient->Socket->WriteLn(IntToStr((int)this->FilePile->TimeLow));
         IdTCPFileClient->Socket->WriteLn(IntToStr((int)this->FilePile->TimeHigh));

         //192-bit (24-byte)
         RunningKey = SetRunKey(this->KeyString + FilePath + IntToStr((__int64)this->FilePile->Size));

         Data = IdTCPFileClient->Socket->ReadLn();
         this->ClientOut->Lines->Add("Server: " + Data);

         Fichier=new TFileStream(this->FilePile->Path, fmOpenRead);
         //FILE* Fichier = fopen(AnsiString(this->FilePile->Path).c_str(), "rb") ;
         //for( __int64 i=0 ; i<this->FilePile->Size ; i++ ) {
         for( __int64 i=this->FilePile->Size ; i>0 ; i=i-PAYLOADSIZE ) {
            ReadSize = i>=PAYLOADSIZE?PAYLOADSIZE:i;

            Buffer.set_length(ReadSize);
            //fread(CharBuffer, 1, ReadSize, Fichier);
            Fichier->Read(Buffer, ReadSize);

            /*for (int j=0; j<=ReadSize; j++) {
                Buffer[j]= CharBuffer[j]; //BytesToChar(Buffer, j);
            }*/

            if (cbEncrypt->Checked && ReadSize>0)
               RunningKey = this->OFBCipher(Buffer, ReadSize, this->KeyString, RunningKey);

            IdTCPFileClient->Socket->Write(Buffer, ReadSize);

            this->PGBarSendPart->Position += 1;
            Application->ProcessMessages();

            if (i%32768 == 0) {
               Data = IdTCPFileClient->Socket->ReadLn();
               this->Invalidate();
               Application->ProcessMessages();
            }
         }
         //fclose(Fichier);
         Fichier->Free();

         this->ClientOut->Lines->Add("Sent: " + this->FilePile->Path);

         this->PGBarSendPart->Position = 0;
         this->CmpSend++;
         this->PGBarSendTotal->Position = this->CmpSend;

         FileStack *FileDone = this->FilePile;
         this->FilePile = this->FilePile->Next;
         delete FileDone;

         IdTCPFileClient->Disconnect();
      }
      catch(Exception& e)
      {
         Fichier->Free();
         IdTCPFileClient->Disconnect();

         this->ClientOut->Lines->Add("ERROR: " + this->FilePile->Path);
         this->ClientOut->Lines->Add(e.Message);

         if (FilePile != NULL) {
            FileStack *FileDone = this->FilePile;
            this->FilePile = this->FilePile->Next;
            delete FileDone;
         }

         this->PGBarSendPart->Position = 0;
         this->CmpSend++;
         this->PGBarSendTotal->Position = this->CmpSend;
      }
   }
}

//---------------------------------------------------------------------------
void __fastcall TMainForm::SpeedUpDateTimer(TObject *Sender)
{
   //__int64 temps;

   float debit;
   double interval = 0.75;//(this->SpeedUpDate->Interval / 1000);

   debit = ((this->PGBarSendPart->Position - this->LastPGSendPos)*PAYLOADSIZE) / interval;
   debit = abs((long)debit);
   this->LastPGSendPos = this->PGBarSendPart->Position;
   if(debit<1024 && debit!=0) {
      SendSpeed->Caption=FloatToStr(RoundTo(debit,-2))+" o/s";
   } else {
      debit/=1024;
      if(debit<1024) {
         SendSpeed->Caption=FloatToStr(RoundTo(debit,-2))+" Ko/s";
      } else {
         debit/=1024;
         if(debit<1024)
            SendSpeed->Caption=FloatToStr(RoundTo(debit,-2))+" Mo/s";
      }
   }

   debit = ((this->PGBarRecvPart->Position - this->LastPGRecvPos)*PAYLOADSIZE) / interval;
   debit = abs((long)debit);
   this->LastPGRecvPos = this->PGBarRecvPart->Position;
   if(debit<1024 && debit!=0) {
      RecvSpeed->Caption=FloatToStr(RoundTo(debit,-2))+" o/s";
   } else {
      debit/=1024;
      if(debit<1024) {
         RecvSpeed->Caption=FloatToStr(RoundTo(debit,-2))+" Ko/s";
      } else {
         debit/=1024;
         if(debit<1024)
            RecvSpeed->Caption=FloatToStr(RoundTo(debit,-2))+" Mo/s";
      }
   }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormCreate(TObject *Sender)
{
   TCHAR CompName[2048];
   DWORD CompNameCharCount = 2048;
   GetComputerName( CompName, &CompNameCharCount );
   eComputerName->Text = CompName;

   this->LocalNetList = new TStringList;
   this->LocalNetList->NameValueSeparator = '=';

   this->LocalNetTTL = new TStringList;
   this->LocalNetTTL->NameValueSeparator = '=';

   this->DisableFileTime = false;

   try {
      IdUDPNetServer->Active = true;
   } catch (Exception& e) {
      IdUDPNetServer->Active = false;
   }
}
//---------------------------------------------------------------------------

 void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
   IdTCPFileServer->Active = false;
   IdUDPNetServer->Active = false;
   delete this->LocalNetList;
   delete this->LocalNetTTL;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::TxtPortExit(TObject *Sender)
{
   IdUDPNetClient->Active = false;
   IdUDPNetServer->Active = false;
   IdUDPNetClient->Port   = StrToInt(TxtPort->Text)-33;

   IdUDPNetServer->DefaultPort = StrToInt(TxtPort->Text)-33;
   IdUDPNetServer->Active      = true;

   IdTCPFileClient->Port = StrToInt(TxtPort->Text);

   bool Actif = IdTCPFileServer->Active;
   IdTCPFileServer->Active      = false;
   IdTCPFileServer->DefaultPort = StrToInt(TxtPort->Text);
   IdTCPFileServer->Bindings->Clear();
   IdTCPFileServer->Bindings->Add()->SetBinding("0.0.0.0", StrToInt(TxtPort->Text));
   //IdTCPFileServer->Bindings->Add();
   //IdTCPFileServer->Bindings->Items[0]->IP="0.0.0.0";
   //IdTCPFileServer->Bindings->Items[0]->Port=StrToInt(TxtPort->Text);
   if (Actif) {
      IdTCPFileServer->Active = true;
      this->ServerOut->Lines->Add("Reactivate, port: " + TxtPort->Text);
   }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::UpdateListTimer(TObject *Sender)
{
   int TTLCmp;

   try {
      this->IdUDPNetClient->Active = true;
      this->IdUDPNetClient->Broadcast("TigerExchange:" + IdIPWatch->CurrentIP + ">" + eComputerName->Text, StrToInt(TxtPort->Text)-33);
      this->IdUDPNetClient->Active = false;

      for(int i = 0; i<this->LocalNetTTL->Count ; i++) {
         TTLCmp = StrToInt(this->LocalNetTTL->Values[this->LocalNetTTL->Names[i]]);
         if (TTLCmp <= 0) {
            this->LocalNetList->Delete(this->LocalNetList->IndexOfName(this->LocalNetTTL->Names[i]));
            this->LocalNetTTL->Delete(i);
            this->lbLocalNet->Clear();
            for(int j = 0; j<this->LocalNetList->Count ; j++)
               this->lbLocalNet->Items->Add(this->LocalNetList->Values[this->LocalNetList->Names[j]] + " (" + this->LocalNetList->Names[j] + ")");
         }
         else
            this->LocalNetTTL->Values[this->LocalNetTTL->Names[i]] = IntToStr(TTLCmp-1);
      }

      this->lLocalIP->Caption = IdIPWatch->CurrentIP;
   } catch (Exception& e) {
      this->lLocalIP->Caption = IdIPWatch->CurrentIP;
   }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::IdUDPNetServerUDPRead(TIdUDPListenerThread *AThread, const TIdBytes AData,
          TIdSocketHandle *ABinding)
{
   String RawData = BytesToString(AData);
   String Data, Name, IPAddr;
   int ListPos;

   if (RawData.Pos("TigerExchange:") == 1) {
      IPAddr = RawData.SubString(Pos(":",RawData)+1, Pos(">",RawData)-Pos(":",RawData)-1);
      Name   = RawData.SubString(Pos(">",RawData,2)+1,50);

      if (IPAddr != IdIPWatch->CurrentIP) {
         ListPos = this->LocalNetList->IndexOfName(IPAddr);
         if (ListPos < 0) {
            //Name=Value
            this->LocalNetList->Add(IPAddr + "=" + Name);
            //this->LocalNetList->Values[IPAddr] = Name;
            this->LocalNetTTL->Add(IPAddr + "=3");
            this->lbLocalNet->Items->Add(Name + " (" + IPAddr + ")");
         } else {
            this->LocalNetTTL->Values[IPAddr] = "3";
            if (this->LocalNetList->Values[IPAddr] != Name) {
               this->LocalNetList->Values[IPAddr] = Name;
               //this->LocalNetList->Delete(ListPos);
               //this->LocalNetList->Add(IPAddr + "=" + Name);
               this->lbLocalNet->Clear();
               for(int i = 0; i<this->LocalNetList->Count ; i++)
                  this->lbLocalNet->Items->Add(this->LocalNetList->Values[this->LocalNetList->Names[i]] + " (" + this->LocalNetList->Names[i] + ")");
            }
         }
      }
   }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::bClearClick(TObject *Sender)
{
   this->LocalNetList->Clear();
   lbLocalNet->Clear();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::bSelectClick(TObject *Sender)
{
   for(int i = 0; i<this->lbLocalNet->Count ; i++) {
      if (this->lbLocalNet->Selected[i]) {
         String Data= this->lbLocalNet->Items->Strings[i];
         TxtIp->Text = Data.SubString(Pos( "(", Data)+1, Data.Length()-Pos("(",Data)-1);
      }
   }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::lbLocalNetDblClick(TObject *Sender)
{
   this->bSelect->Click();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::lServRootDblClick(TObject *Sender)
{
   this->DisableFileTime = !this->DisableFileTime;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::IdTCPFileServerConnect(TIdContext *AContext)
{
   this->TxtPort->Enabled = false;
   //AContext->Binding()->PeerIP;
   this->ServerOut->Lines->Add("Connection: " + AContext->Connection->Socket->Binding->PeerIP);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::IdTCPFileServerDisconnect(TIdContext *AContext)
{
   this->TxtPort->Enabled = true;
   //AContext->Binding()->PeerIP;
   this->ServerOut->Lines->Add("Deconnection: " + AContext->Connection->Socket->Binding->PeerIP);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::IdTCPFileServerStatus(TObject *ASender, const TIdStatus AStatus,
          const UnicodeString AStatusText)
{
   this->ServerOut->Lines->Add("Status: " + AStatusText);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::IdTCPFileServerExecute(TIdContext *AContext)
{
   String FileSize, FileName, FilePath;
   int ReadSize, FileTimeLow, FileTimeHigh;
   TIdBytes Buffer;
   //char CharBuffer[PAYLOADSIZE];
   TFileStream*Fichier=0;
   TByteDynArray RunningKey;

   //AContext->Connection->IOHandler->DefStringEncoding = TIdTextEncoding_UTF8;

   FileSize = AContext->Connection->Socket->ReadLn();
   FileName = AContext->Connection->Socket->ReadLn(TIdTextEncoding_UTF8);
   FileTimeLow = StrToInt(AContext->Connection->Socket->ReadLn());
   FileTimeHigh = StrToInt(AContext->Connection->Socket->ReadLn());

   this->ServerOut->Lines->Add("File: " + FileName);
   //this->ServerOut->Lines->Add("Size: " + FileSize);

   FilePath = PathSortie + "\\" + FileName;
   ForceDirectories(ExtractFilePath(FilePath));
   //FILE*Fichier = fopen(AnsiString(FileName).c_str(),"wb");
   Fichier=new TFileStream( FilePath, fmOpenWrite | fmCreate ); // fmOverwrite

   this->PGBarRecvPart->Max=StrToInt64(FileSize)/PAYLOADSIZE;
   this->PGBarRecvPart->Position=0;

   AContext->Connection->Socket->WriteLn("OK");

   //192-bit (24-byte)
   RunningKey = SetRunKey(this->KeyString + FileName + FileSize);

   for( __int64 i=StrToInt64(FileSize) ; i>0 ; i=i-PAYLOADSIZE ) {
      ReadSize = i>=PAYLOADSIZE?PAYLOADSIZE:i;
      Buffer.set_length(0);
      AContext->Connection->Socket->ReadBytes(Buffer,ReadSize);

      /*for (int j=0; j <= ReadSize; j++) {
         CharBuffer[j] = BytesToChar(Buffer, j);
      }*/

      if (cbEncrypt->Checked)
         RunningKey = this->OFBCipher(Buffer, ReadSize, this->KeyString, RunningKey);

      //fwrite(CharBuffer,1,ReadSize,Fichier);
      //Fichier->Write(Buffer, ReadSize);
      Fichier->WriteData(Buffer, ReadSize);
      //Fichier->WriteBuffer(Buffer, ReadSize);
      this->PGBarRecvPart->Position+=1;

      if (i%32768 == 0) {
         AContext->Connection->Socket->WriteLn("OK");
         this->Invalidate();
         Application->ProcessMessages();
      }
   }

   //fclose(Fichier);
   delete Fichier;

   if (!DisableFileTime) {
      FILETIME modtime;
      HANDLE fh = CreateFileW(FilePath.c_str(), GENERIC_READ | FILE_WRITE_ATTRIBUTES, 0, NULL, OPEN_EXISTING, 0, NULL);
      modtime.dwLowDateTime  = FileTimeLow;
      modtime.dwHighDateTime = FileTimeHigh;
      SetFileTime( fh, NULL, NULL, &modtime);
      CloseHandle(fh);
   }

   this->PGBarRecvPart->Position=0;
   this->ServerOut->Lines->Add("Received: " + PathSortie + "\\" + FileName);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MsgIpDblClick(TObject *Sender)
{
   this->TxtIp->Text = "127.0.0.1";
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::lNameDblClick(TObject *Sender)
{
   this->IdUDPNetServer->Active = false;
   this->eComputerName->Enabled = false;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::cbEncryptClick(TObject *Sender)
{
   TfSelPassKey *fSelPassKey = new TfSelPassKey( this );

   if (cbEncrypt->Checked) {
      try {
         if (fSelPassKey->ShowModal() == mrOk) {
            this->KeyString = fSelPassKey->KeyString;
            this->KeyBytes  = fSelPassKey->KeyBytes;
         }
         else
            cbEncrypt->Checked = false;
      } catch(Exception& e) {
         cbEncrypt->Checked = false;
      }
   }

   delete fSelPassKey;
}
//---------------------------------------------------------------------------

TByteDynArray TMainForm::SetRunKey(String Src)
{
   TByteDynArray RunningKey;
   byte str[4096];
   word64 res[3];

   wcstombs(str, Src.c_str(), 4096);
   //192-bit (24-bytes)
   tiger((word64*)str, Src.Length(), res);

   RunningKey.set_length(DIGESTSIZE);

   for(int i=0; i<DIGESTSIZE; i++)
      RunningKey[i] = ((byte*)res)[i];

   return RunningKey;
}
//---------------------------------------------------------------------------

TByteDynArray TMainForm::OFBCipher(TIdBytes Data, int Size, String Key, TByteDynArray RunningKey)
{
   String CharKey = Key;
   int i=0;
   //TIdBytes Buffer;
   byte str[DIGESTSIZE*4];
   word64 res[3];

   //Buffer.set_length(DIGESTSIZE);

   do {
      //Data[i] = ToBytes(BytesToChar(Data[i]) ^ RunningKey[i%DIGESTSIZE]);
      Data[i] = Data[i] ^ RunningKey[i%DIGESTSIZE];
      CharKey = CharKey + LowerCase(ByteToHex(RunningKey[i%DIGESTSIZE]));

      i++;
      if (i%DIGESTSIZE == 0) {
         wcstombs(str, CharKey.c_str(), DIGESTSIZE*4);
         //192-bit (24-bytes)
         tiger((word64*)str, DIGESTSIZE*4, res);

         for(int j=0; j<DIGESTSIZE; j++)
            RunningKey[j] = ((byte*)res)[j];

         CharKey = Key;
      }
   } while (i<Size);

   CharKey = Key;
   for( int j=0 ; j<DIGESTSIZE ; j++ )
      CharKey = CharKey + LowerCase(ByteToHex(RunningKey[j]));

   wcstombs(str, CharKey.c_str(), DIGESTSIZE*4);
   //192-bit (24-bytes)
   tiger((word64*)str, DIGESTSIZE*4, res);

   for(int j=0; j<DIGESTSIZE; j++)
      RunningKey[j] = ((byte*)res)[j];

   return RunningKey;
}
//---------------------------------------------------------------------------


