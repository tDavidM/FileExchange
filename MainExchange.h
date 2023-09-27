//---------------------------------------------------------------------------

#ifndef MainExchangeH
#define MainExchangeH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <StrUtils.hpp>
#include <Forms.hpp>
#include <Math.hpp>

#include "SelPassKey.h"
#include "Clipboard.h"

#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
#include <IdBaseComponent.hpp>
#include <IdComponent.hpp>
#include <IdIPWatch.hpp>
#include <IdUDPBase.hpp>
#include <IdUDPClient.hpp>
#include <IdUDPServer.hpp>
#include <IdGlobal.hpp>
#include <IdSocketHandle.hpp>
#include <IdCustomTCPServer.hpp>
#include <IdTCPClient.hpp>
#include <IdTCPConnection.hpp>
#include <IdTCPServer.hpp>
#include <IdContext.hpp>
#include <Vcl.Buttons.hpp>

class tFileList
{
    public:
        String Path;
        String FileRoot;
        __int64 Size;
        //int Time;
        unsigned long TimeLow, TimeHigh;
        tFileList *Next;
};
//---------------------------------------------------------------------------
class TMainForm : public TForm
{
__published: // Composants g�r�s par l'EDI
        TGroupBox *BoxServ;
        TButton *CmdServ;
   TMemo *ServerOut;
        TGroupBox *BoxSend;
        TLabel *MsgIp;
        TEdit *TxtIp;
        TButton *CmdSend;
        TOpenDialog *OpenAny;
   TMemo *ClientOut;
        TTimer *SpeedUpDate;
        TProgressBar *PGBarSendTotal;
        TProgressBar *PGBarSendPart;
        TProgressBar *PGBarRecvPart;
   TIdIPWatch *IdIPWatch;
   TIdUDPClient *IdUDPNetClient;
   TIdUDPServer *IdUDPNetServer;
   TTimer *UpdateList;
   TGroupBox *BoxDiscover;
   TEdit *eComputerName;
   TLabel *lName;
   TListBox *lbLocalNet;
   TButton *bClear;
   TButton *bSelect;
   TIdTCPClient *IdTCPFileClient;
   TIdTCPServer *IdTCPFileServer;
   TLabel *MsgPort;
   TEdit *TxtPort;
   TLabel *Label1;
   TLabel *RecvSpeed;
   TLabel *Label2;
   TLabel *SendSpeed;
   TLabel *lServRoot;
   TLabel *lLocalIP;
   TLabel *lLblLocalIP;
   TLabel *lDbg;
   TCheckBox *cbEncrypt;
   TButton *bPoke;
   TCheckBox *cbWaitPoke;
   TCheckBox *cbFileTime;
   TBitBtn *bbMessage;
        void __fastcall CmdServClick(TObject *Sender);
        void __fastcall CmdSendClick(TObject *Sender);
        void __fastcall SpeedUpDateTimer(TObject *Sender);
   void __fastcall UpdateListTimer(TObject *Sender);
   void __fastcall FormCreate(TObject *Sender);
   void __fastcall bClearClick(TObject *Sender);
   void __fastcall IdUDPNetServerUDPRead(TIdUDPListenerThread *AThread, const TIdBytes AData,
          TIdSocketHandle *ABinding);
   void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
   void __fastcall bSelectClick(TObject *Sender);
   void __fastcall lbLocalNetDblClick(TObject *Sender);
   void __fastcall IdTCPFileServerConnect(TIdContext *AContext);
   void __fastcall IdTCPFileServerDisconnect(TIdContext *AContext);
   void __fastcall IdTCPFileServerStatus(TObject *ASender, const TIdStatus AStatus,
          const UnicodeString AStatusText);
   void __fastcall IdTCPFileServerExecute(TIdContext *AContext);
   void __fastcall MsgIpDblClick(TObject *Sender);
   void __fastcall lNameDblClick(TObject *Sender);
   void __fastcall TxtPortExit(TObject *Sender);
   void __fastcall cbEncryptClick(TObject *Sender);
   void __fastcall ServerOutMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
   void __fastcall ClientOutMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
   void __fastcall bPokeClick(TObject *Sender);
   void __fastcall cbWaitPokeClick(TObject *Sender);
   void __fastcall TxtIpChange(TObject *Sender);
   void __fastcall cbFileTimeClick(TObject *Sender);
   void __fastcall bbMessageClick(TObject *Sender);
   void __fastcall lbLocalNetClick(TObject *Sender);
private: // D�clarations de l'utilisateur
        int SearchFiles(String FilePath, String FileRoot);
        void DropFiles(TMessage &Message);
        void SendFile();
        TByteDynArray SetRunKey(String Src);
        TByteDynArray OFBCipher(TIdBytes Data, int Size, String Key, TByteDynArray RunningKey);

        int LastPGSendPos;
        int LastPGRecvPos;
        tFileList *FileList;
        int CmpSend;
        String PathSortie;
        bool DisableFileTime;

        String        KeyString;
        TByteDynArray KeyBytes;

        TStringList *LocalNetList;
        TStringList *LocalNetTTL;
public: // D�clarations de l'utilisateur
        __fastcall TMainForm(TComponent* Owner);
        void SendMessage(String Message);

        #pragma option push -winl
        BEGIN_MESSAGE_MAP
        MESSAGE_HANDLER(WM_DROPFILES, TMessage, DropFiles);
        END_MESSAGE_MAP(TForm);
        #pragma option pop // -winl

};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif
