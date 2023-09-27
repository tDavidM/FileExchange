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
   bool DelaySend = false;
   int CmpSearch = 0;

   if (this->TxtIp->Text == "" )
      ShowMessage("Specify the IP address of the server");
   else if (this->OpenAny->Execute() == true) { //multiselect
      this->TxtPort->Enabled = false;
      fClipboard->bSend->Enabled = false;
      DelaySend = this->FileList != NULL || this->cbWaitPoke->Checked;

      for (int i=0; i<this->OpenAny->Files->Count; i++) {
         CmpSearch += this->SearchFiles(this->OpenAny->Files->Strings[i],
                                        ExtractFilePath(ExtractFilePath(this->OpenAny->Files->Strings[0])));
      }

      if (!DelaySend) {
         this->CmpSend   = 0;
         this->PGBarSendTotal->Max = CmpSearch;
         this->SendFile();
      } else {
         this->PGBarSendTotal->Max += CmpSearch;
         if (this->cbWaitPoke->Checked) {
            this->ClientOut->Lines->Add(IntToStr(CmpSearch) + " File(s) awaiting...");
            this->CmpSend = 0;
         }
      }
   }

   if (!DelaySend) {
      this->TxtPort->Enabled = true;
      fClipboard->bSend->Enabled = true;
      this->CmpSend = 0;
      this->PGBarSendTotal->Position = 0;
      this->PGBarSendTotal->Max = 100;
   }
}
//---------------------------------------------------------------------------
void TMainForm::DropFiles(TMessage &Message)
{
   int nFiles;
   //char buffer[65536];
   wchar_t buffer[65536];
   bool DelaySend = false;
   int CmpSearch = 0;

   if (this->TxtIp->Text == "" ) {
      ShowMessage("Specify the IP address of the server");
   } else {
      this->TxtPort->Enabled = false;
      fClipboard->bSend->Enabled = false;
      nFiles = DragQueryFile((HDROP)Message.WParam, 0xFFFFFFFF, NULL, 0);
      DelaySend = this->FileList != NULL || this->cbWaitPoke->Checked;

      for (int i=0; i<nFiles; i++) {
         DragQueryFileW((HDROP)Message.WParam, i, buffer, 65536);
         CmpSearch += this->SearchFiles(buffer, ExtractFilePath(buffer));
      }
      DragFinish((HDROP)Message.WParam);

      if (!DelaySend) {
         this->CmpSend   = 0;
         this->PGBarSendTotal->Max = CmpSearch;
         this->SendFile();
      } else {
         this->PGBarSendTotal->Max += CmpSearch;
         if (this->cbWaitPoke->Checked) {
            this->ClientOut->Lines->Add(IntToStr(CmpSearch) + " File(s) awaiting...");
            this->CmpSend = 0;
         }
      }
   }

   if (!DelaySend) {
      this->TxtPort->Enabled = true;
      fClipboard->bSend->Enabled = true;
      this->CmpSend = 0;
      this->PGBarSendTotal->Position = 0;
      this->PGBarSendTotal->Max = 100;
   }
}
//---------------------------------------------------------------------------
int TMainForm::SearchFiles(String FilePath, String FileRoot)
{
   tFileList *New, *Last;
   //tFileList *Curr;
   int FHandle, Cmp = 0;
   FILETIME modtime;

   if(DirectoryExists(FilePath)) {
      DIR *dir;
      struct dirent *ent;
      if ((dir = opendir(AnsiString(FilePath).c_str())) == NULL)
         exit(1);

      while ((ent = readdir(dir)) != NULL) {
         if (strcmp(ent->d_name,".") != 0 && strcmp(ent->d_name,"..") != 0)
            Cmp += SearchFiles(FilePath + "\\" + ent->d_name, FileRoot);
      }
      closedir(dir);
      return Cmp;
   } else {
      New = new tFileList;
      New->Path     = FilePath;
      New->FileRoot = FileRoot;

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

      New->Next = NULL;

      if (this->FileList != NULL) {
         Last = this->FileList;
         while(Last->Next != NULL)
            Last = Last->Next;
         Last->Next = New;
      } else
        this->FileList = New;

      Application->ProcessMessages();
      return 1;
   }
}

//---------------------------------------------------------------------------
void TMainForm::SendFile()
{
   String FilePath, Data;
   int ReadSize;
   //char CharBuffer[PAYLOADSIZE];
   TIdBytes Buffer;
   TFileStream*Fichier=0;
   TByteDynArray RunningKey;

   this->ClientOut->Lines->Add("Sending...");

   while(this->FileList != NULL)
   {
      try
      {
         IdTCPFileClient->Host = TxtIp->Text;
         IdTCPFileClient->Connect();
         this->ClientOut->Lines->Add("Working (" + IdTCPFileClient->Socket->Binding->PeerIP + "): " +
                                     ExtractFileName(this->FileList->Path));
         this->PGBarSendPart->Max = this->FileList->Size/PAYLOADSIZE;
         this->PGBarSendPart->Position = 0;

         Application->ProcessMessages();

         FilePath = AnsiReplaceText(this->FileList->Path, this->FileList->FileRoot, "");

         IdTCPFileClient->Socket->WriteLn(IntToStr((__int64)this->FileList->Size));
         IdTCPFileClient->Socket->WriteLn(FilePath, TIdTextEncoding_UTF8);
         IdTCPFileClient->Socket->WriteLn(IntToStr((int)this->FileList->TimeLow));
         IdTCPFileClient->Socket->WriteLn(IntToStr((int)this->FileList->TimeHigh));

         //192-bit (24-byte)
         RunningKey = SetRunKey(this->KeyString + FilePath + IntToStr((__int64)this->FileList->Size));

         Data = IdTCPFileClient->Socket->ReadLn();
         this->ClientOut->Lines->Add("Server: " + Data);

         Fichier=new TFileStream(this->FileList->Path, fmOpenRead);
         //FILE* Fichier = fopen(AnsiString(this->FileList->Path).c_str(), "rb") ;
         //for( __int64 i=0 ; i<this->FileList->Size ; i++ ) {
         for( __int64 i=this->FileList->Size ; i>0 ; i=i-PAYLOADSIZE ) {
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

         this->ClientOut->Lines->Add("Sent: " + this->FileList->Path);

         this->PGBarSendPart->Position = 0;
         this->CmpSend++;
         this->PGBarSendTotal->Position = this->CmpSend;
         Application->ProcessMessages();

         tFileList *FileDone = this->FileList;
         this->FileList = this->FileList->Next;
         delete FileDone;

         IdTCPFileClient->Disconnect();
      }
      catch(Exception& e)
      {
         Fichier->Free();
         IdTCPFileClient->Disconnect();

         this->ClientOut->Lines->Add("ERROR: " + this->FileList->Path);
         this->ClientOut->Lines->Add(e.Message);

         if (FileList != NULL) {
            tFileList *FileDone = this->FileList;
            this->FileList = this->FileList->Next;
            delete FileDone;
         }

         this->PGBarSendPart->Position = 0;
         this->CmpSend++;
         this->PGBarSendTotal->Position = this->CmpSend;
         Application->ProcessMessages();
      }
   }
}

//---------------------------------------------------------------------------
void TMainForm::SendMessage(String Message)
{
   String Data;
   TBytes MessageBytes;
   int ReadSize;
   TIdBytes Buffer;
   TByteDynArray RunningKey;

   this->ClientOut->Lines->Add("Sending message");
   try
   {
      IdTCPFileClient->Host = TxtIp->Text;
      IdTCPFileClient->Connect();
      this->ClientOut->Lines->Add("Working (" + IdTCPFileClient->Socket->Binding->PeerIP + "): ");
      Application->ProcessMessages();

      MessageBytes = WideBytesOf(Message);

      IdTCPFileClient->Socket->WriteLn(IntToStr(MessageBytes.Length));
      IdTCPFileClient->Socket->WriteLn("?Message*:", TIdTextEncoding_UTF8);
      IdTCPFileClient->Socket->WriteLn(IntToStr(0));
      IdTCPFileClient->Socket->WriteLn(IntToStr(0));

      //192-bit (24-byte)
      RunningKey = SetRunKey(this->KeyString + "?Message*:" + IntToStr(MessageBytes.Length));

      Data = IdTCPFileClient->Socket->ReadLn();
      this->ClientOut->Lines->Add("Server: " + Data);

      for( int i=MessageBytes.Length ; i>0 ; i=i-PAYLOADSIZE ) {
         ReadSize = i>=PAYLOADSIZE?PAYLOADSIZE:i;
         Buffer.set_length(ReadSize);

         for ( int j=0; j<ReadSize; j++)
            Buffer[j] = MessageBytes[j+(MessageBytes.Length-i)];

         if (cbEncrypt->Checked && ReadSize>0)
            RunningKey = this->OFBCipher(Buffer, ReadSize, this->KeyString, RunningKey);

         IdTCPFileClient->Socket->Write(Buffer, ReadSize);
         Application->ProcessMessages();
         if (i%32768 == 0) {
            Data = IdTCPFileClient->Socket->ReadLn();
            this->Invalidate();
            Application->ProcessMessages();
         }
      }
      IdTCPFileClient->Disconnect();
   }
   catch(Exception& e)
   {
      IdTCPFileClient->Disconnect();

      this->ClientOut->Lines->Add("ERROR: Message");
         this->ClientOut->Lines->Add(e.Message);
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
   this->IdUDPNetClient->Active = false;
   this->IdUDPNetServer->Active = false;
   this->IdUDPNetClient->Port   = StrToInt(TxtPort->Text)-33;

   this->IdUDPNetServer->DefaultPort = StrToInt(TxtPort->Text)-33;
   this->IdUDPNetServer->Active      = true;

   this->IdTCPFileClient->Port = StrToInt(TxtPort->Text);

   bool Actif = IdTCPFileServer->Active;
   this->IdTCPFileServer->Active      = false;
   this->IdTCPFileServer->DefaultPort = StrToInt(TxtPort->Text);
   this->IdTCPFileServer->Bindings->Clear();
   this->IdTCPFileServer->Bindings->Add()->SetBinding("0.0.0.0", StrToInt(TxtPort->Text));
   //IdTCPFileServer->Bindings->Add();
   //IdTCPFileServer->Bindings->Items[0]->IP="0.0.0.0";
   //IdTCPFileServer->Bindings->Items[0]->Port=StrToInt(TxtPort->Text);
   if (Actif) {
      this->IdTCPFileServer->Active = true;
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
            this->bPoke->Enabled = false;
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

void __fastcall TMainForm::lbLocalNetClick(TObject *Sender)
{
   for(int i = 0; i<this->lbLocalNet->Count ; i++) {
      if (this->lbLocalNet->Selected[i] && IdTCPFileServer->Active)
         this->bPoke->Enabled = true;
   }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::bPokeClick(TObject *Sender)
{
   String IPAddr;

   for(int i = 0; i<this->lbLocalNet->Count ; i++) {
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
      this->IdUDPNetClient->Send("TigerMessage:" + IdIPWatch->CurrentIP + ">Poke");
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
   } else if (RawData.Pos("TigerMessage:") == 1) {
      if (this->CmpSend == 0) {

         TNotifyEvent cbWaitPokeClickEvent, TxtIpChangeEvent; // Sorry -_-'
         cbWaitPokeClickEvent = this->cbWaitPoke->OnClick;
         TxtIpChangeEvent     = this->TxtIp->OnChange;
         this->cbWaitPoke->OnClick = NULL;
         this->TxtIp->OnChange = NULL;

         this->TxtIp->Text = RawData.SubString(Pos(":",RawData)+1, Pos(">",RawData)-Pos(":",RawData)-1);

         this->cbWaitPoke->Checked = false;
         this->cbWaitPoke->OnClick = cbWaitPokeClickEvent; // = cbWaitPokeClick;
         this->TxtIp->OnChange     = TxtIpChangeEvent; // = TxtIpChange;

         this->SendFile();

         this->TxtPort->Enabled = true;
         fClipboard->bSend->Enabled = true;
         this->CmpSend = 0;
         this->PGBarSendTotal->Position = 0;
         this->PGBarSendTotal->Max = 100;
      }
   }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::bbMessageClick(TObject *Sender)
{
   fClipboard->Show();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ServerOutMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y)
{
   if (Button == mbLeft && Shift.Contains(ssCtrl))
      this->ServerOut->Lines->Clear();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ClientOutMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y)
{
   if (Button == mbLeft && Shift.Contains(ssCtrl))
      this->ClientOut->Lines->Clear();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::cbWaitPokeClick(TObject *Sender)
{
   if (this->cbWaitPoke->Checked)
      this->TxtIp->Text = "0.0.0.0";
   else {
      if (this->TxtIp->Text == "0.0.0.0")
         this->TxtIp->Text = "";

      if (this->FileList != NULL) {
         tFileList *FileDel;
         while (this->FileList != NULL) {
            FileDel = this->FileList;
            this->FileList = this->FileList->Next;
            delete FileDel;
         }
         this->ClientOut->Lines->Add("No file awaiting...");
      }
   }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::TxtIpChange(TObject *Sender)
{
   if (this->TxtIp->Text != "0.0.0.0")
      this->cbWaitPoke->Checked = false;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::bClearClick(TObject *Sender)
{
   this->LocalNetList->Clear();
   this->LocalNetTTL->Clear();
   lbLocalNet->Clear();
   this->bPoke->Enabled = false;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::bSelectClick(TObject *Sender)
{
   for(int i = 0; i<this->lbLocalNet->Count ; i++) {
      if (this->lbLocalNet->Selected[i]) {
         String Data= this->lbLocalNet->Items->Strings[i];
         this->TxtIp->Text = Data.SubString(Pos( "(", Data)+1, Data.Length()-Pos("(",Data)-1);
      }
   }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::lbLocalNetDblClick(TObject *Sender)
{
   this->bSelect->Click();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::cbFileTimeClick(TObject *Sender)
{
   this->DisableFileTime = this->cbFileTime->Checked;
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
   TIdBytes Buffer, Message;
   //char CharBuffer[PAYLOADSIZE];
   TFileStream*Fichier=0;
   TByteDynArray RunningKey;

   //AContext->Connection->IOHandler->DefStringEncoding = TIdTextEncoding_UTF8;

   FileSize = AContext->Connection->Socket->ReadLn();
   FileName = AContext->Connection->Socket->ReadLn(TIdTextEncoding_UTF8);
   FileTimeLow = StrToInt(AContext->Connection->Socket->ReadLn());
   FileTimeHigh = StrToInt(AContext->Connection->Socket->ReadLn());

   if(FileName == "?Message*:" && FileTimeLow == 0 && FileTimeHigh == 0) {
      this->ServerOut->Lines->Add("Message");
      AContext->Connection->Socket->WriteLn("OK");
      Message.set_length(StrToInt(FileSize));

      RunningKey = SetRunKey(this->KeyString + FileName + FileSize);

      for(int i=StrToInt(FileSize) ; i>0 ; i=i-PAYLOADSIZE ) {
         ReadSize = i>=PAYLOADSIZE?PAYLOADSIZE:i;
         Buffer.set_length(0);
         AContext->Connection->Socket->ReadBytes(Buffer,ReadSize);

         if (cbEncrypt->Checked)
            RunningKey = this->OFBCipher(Buffer, ReadSize, this->KeyString, RunningKey);

         for ( int j=0; j<ReadSize; j++)
            Message[j+(StrToInt(FileSize)-i)] = Buffer[j];

         if (i%32768 == 0) {
            AContext->Connection->Socket->WriteLn("OK");
            this->Invalidate();
            Application->ProcessMessages();
         }
      }
      //fClipboard->mText->Lines->Add(BytesToString(Message));
      fClipboard->mText->Lines->Add("Received:");
      fClipboard->mText->Lines->Add(WideStringOf(Message));
   } else {
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


