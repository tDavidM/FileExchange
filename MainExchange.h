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
#include "Crypto.h"

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

#define VERSION_NUMBER_STR "2.3.2"

#define MAXFILEPATH 768
#define PAYLOADSIZE 4096
#define MAXMESSAGESIZE 512

#define PROTOCOL_MSG_OK            "OK"
#define PROTOCOL_MSG_STOP          "STOP"
#define PROTOCOL_MSG_DONE          "DONE"
#define PROTOCOL_MSG_META_SIZE_ERR "ERR-SIZE"
#define PROTOCOL_MSG_META_HMAC_ERR "ERR-HMAC"
#define PROTOCOL_MSG_KEYWORD       "TigerMessage:"
#define PROTOCOL_IDENT_KEYWORD     "TigerExchange:"
#define PROTOCOL_POKE_KEYWORD      ">Poke"

#define MSG_SRV_STOP            "Deactivate"                               // "Désactiver"
#define MSG_SRV_START_PORT      "Activate, port: "                         // "Activé, port: "
#define MSG_SRV_START_DIR       "   Root: "                                // "   Racine: "
#define MSG_SRV_START_ERR       "Error:No output specified !"              // "Erreur:Pas de sortie spécifié !"
#define MSG_SEND_IP_ERR         "Specify the IP address of the server."    // "Spécifier l'adresse IP du serveur."
#define MSG_SEND_PENDING_FILE   " File(s) awaiting..."                     // " Fichier(s) en attente..."
#define MSG_SEND_START          "Sending..."                               // "Envois..."
#define MSG_SEND_MSG_INIT       "Sending:"                                 // "Envois:"
#define MSG_SEND_ERR            "ERROR: "                                  // "ERREUR: "
#define MSG_SEND_PROCESSING     "Working ("                                // "Traitement ("
#define MSG_SEND_SRV_INFO       "Server: "                                 // "Serveur: "
#define MSG_SEND_CHECK_KEY      "Validate your encryption key."            // "Validez votre clé d'encryption."
#define MSG_SEND_META_ERR       "Error, metadata exchange."                // "Erreur d'envois des Métainformations."
#define MSG_SEND_DATA_ERR       "Error, data exchange."                    // "Erreur d'envois des informations."
#define MSG_SEND_DONE           "Sent: "                                   // "Envoyé: "
#define MSG_RNGEN_ERR           "Failed to generate secure random bytes."  // "Impossible de générer des octets aléatoires sécurisés."
#define MSG_SEND_PATHLENGTH_ERR "File path to long."                       // "Chemin de fichier trop long."
#define MSG_SEND_MSG            "Sending message "                         // "Envois de message"
#define MSG_SEND_MSG_ERR        "ERROR: Message"                           // "ERREUR: Message"
#define MSG_SRV_PORT_CHANGE     "Reactivate, port: "                       // "Réactivé, port: "
#define MSG_RSCV_POKE           "Poke: "                                   // "Poke: "
#define MSG_RSCV_MSG            "Message: "                                // "Message: "
#define MSG_RSCV_MSG_SIZE_ERR   "Entry stream error."                      // "Erreur de flux d'entré"
#define MSG_RSCV_MSG_HEX_ERR    "HEX Error"                                // "Erreur HEX"
#define MSG_RSCV_MSG_HMAC_ERR   "HMAC Error"                               // "Erreur HMAC"
#define MSG_RSCV_MSG_DONE       "Received:"                                // "Reçu:"
#define MSG_RSCV_POKE_NO_FILE   "No file awaiting..."                      // "Aucun fichier en attente..."
#define MSG_RSCV_START          "Connection: "                             // "Connexion: "
#define MSG_RSCV_STOP           "Disconnetion: "                           // "Déconnexion: "
#define MSG_RSCV_STATUS         "Status: "                                 // "Statut: "
#define MSG_RSCV_FILE           "File: "                                   // "Fichier: "
#define MSG_RSCV_FILE_ERR_HMAC  "!!!!! ERROR: HMAC BLOCK:"                 // "!!!!! ERREUR HMAC BLOC: "
#define MSG_RSCV_FILE_INVALID   "Invalid file: "                           // "Ficher invalide: "
#define MSG_RSCV_DONE           "Received: "                               // "Reçu: "
#define MSG_PWRD_GEN_ERR        "Failed to generate secure password hash." // "Impossible de générer un hash de mot de passe sécurisés." // 

#define STR_ROOTDIR_DIAG "Select the Directory where the files will be placed" // "Selectionner le Répertoire où seront placé les fichiers"
#define STR_SELKEY_CAP    "Enter a key"          // "Selection de la clé"
#define STR_SELKEY_LBL    "Encryption key"       // "Clé d'encryption"
#define STR_MEMO_HINT     "Ctrl+Clic to erase"  // "Shift+Clic pour effacer"
#define STR_CLIPBOARD_CAP "Clipboard"            // "Presse-Papier"
#define STR_SPEED_INIT    "0 KB/s"               // "0 Ko/s"

#define CAP_OUTPUT_ROOT         "Target directory"              // "Répertoire destination"
#define CAP_SRV_DIR_IND         "* "
#define CAP_MAINFORM_STOP_SVR   "Deactivate"                    // "Désactiver"
#define CAP_SPEED_B             " B/s"                          // " o/s"
#define CAP_SPEED_KB            " KB/s"                         // " Ko/s"
#define CAP_SPEED_MB            " MB/s"                         // " Mo/s"
#define CAP_CLIPBOARD_BTN       "Send"                          // "Envoyer"
#define CAP_SELKEY_BTN          "Cancel"                        // "Annuler"
#define CAP_SELKEY_OK           "Ok"
#define CAP_MAINFORM            "Exchange V" VERSION_NUMBER_STR // "Exchange V" 
#define CAP_MAINFORM_GBSERV     "Server"                        // "Serveur"
#define CAP_MAINFORM_LOCIP      "Local IP: "                    // "IP Locale: "
#define CAP_MAINFORM_START_SVR  "Activate"                      // "Activer"
#define CAP_MAINFORM_SRV_ROOT   "Target Directory:"             // "Répertoire destination:"
#define CAP_MAINFORM_RSCV_SPEED "Inbound: "                     // "Reception:"
#define CAP_MAINFORM_FILE_TIME  "Ignore received Date/Time"     // "Ignorer Date/Heure reçue"
#define CAP_MAINFORM_GBSEND     "Client"                        // "Client"
#define CAP_MAINFORM_DESTIP     "Server IP Address:"            // "Adresse Ip du serveur:"
#define CAP_MAINFORM_SEND       "Send"                          // "Envoyer"
#define CAP_MAINFORM_SEND_SPEED "Sent:"                         // "Envoi:"
#define CAP_MAINFORM_WAIT_POKE  "       Wait to receive a Poke" // "Attendre de recevoir un Poke"
#define CAP_MAINFORM_NETWORK    "Network"                       // "Réseau"
#define CAP_MAINFORM_PORT       "Port (Client/Server):"         // "Port (Client/Serveur):"
#define CAP_MAINFORM_NAME       "Computer Name:"                // "Nom Machine:"
#define CAP_MAINFORM_CRYPTO     "Use Encryption"                // "Activer l'encryption"
#define CAP_MAINFORM_REFRESH    "Refresh"                       // "Rafraichir"
#define CAP_MAINFORM_POKE       "Poke"
#define CAP_MAINFORM_SELECT     "Select"                        // "Selectionner"

class tFileList
{
    public:
        String filePath;
        String fileRoot;
        uint64_t fileSize;
        uint64_t fileTime;
        tFileList *nextFile;
};
//---------------------------------------------------------------------------
class TMainForm : public TForm
{
__published: // Composants gérés par l'EDI
    TGroupBox *gbServ;
    TButton *bStartServ;
    TMemo *mServerOut;
    TGroupBox *gbSend;
    TLabel *lblMsgIp;
    TEdit *edtIp;
    TButton *bCmdSend;
        TOpenDialog *OpenAny;
    TMemo *mClientOut;
    TTimer *SpeedUpdate;
    TProgressBar *prgBarSendTotal;
    TProgressBar *prgBarSendPart;
    TProgressBar *prgBarRecvPart;
   TIdIPWatch *IdIPWatch;
   TIdUDPClient *IdUDPNetClient;
   TIdUDPServer *IdUDPNetServer;
   TTimer *UpdateList;
    TGroupBox *gbDiscover;
    TEdit *edtComputerName;
    TLabel *lblName;
   TListBox *lbLocalNet;
   TButton *bClear;
   TButton *bSelect;
   TIdTCPClient *IdTCPFileClient;
   TIdTCPServer *IdTCPFileServer;
    TLabel *lblMsgPort;
    TEdit *edtPort;
    TLabel *lblRecv;
    TLabel *lblRecvSpeed;
    TLabel *lblSend;
    TLabel *lblSendSpeed;
    TLabel *lblServRoot;
    TLabel *lblLocalIPVal;
    TLabel *lblLocalIP;
   TLabel *lDbg;
   TCheckBox *cbEncrypt;
   TButton *bPoke;
   TCheckBox *cbWaitPoke;
   TCheckBox *cbFileTime;
   TBitBtn *bbMessage;
        void __fastcall bStartServClick(TObject *Sender);
        void __fastcall bCmdSendClick(TObject *Sender);
        void __fastcall SpeedUpdateTimer(TObject *Sender);
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
   void __fastcall lblMsgIpDblClick(TObject *Sender);
   void __fastcall lblNameDblClick(TObject *Sender);
   void __fastcall edtPortExit(TObject *Sender);
   void __fastcall cbEncryptClick(TObject *Sender);
   void __fastcall mServerOutMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
   void __fastcall mClientOutMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
   void __fastcall bPokeClick(TObject *Sender);
   void __fastcall cbWaitPokeClick(TObject *Sender);
   void __fastcall edtIpChange(TObject *Sender);
   void __fastcall cbFileTimeClick(TObject *Sender);
   void __fastcall bbMessageClick(TObject *Sender);
   void __fastcall lbLocalNetClick(TObject *Sender);
private: // Déclarations de l'utilisateur
        int SearchFiles(String filePath, String fileRoot);
        void DropFiles(TMessage &message);
        void SendFile();
        void SendMetaInfo(byte* fileIV, byte* fileKey);
        bool ReceiveMetaInfo(TIdContext *AContext, uint64_t &fileSize, uint64_t &fileTime, String &fileName, byte* fileIV, byte* fileKey);

        int lastPGSendPos;
        int lastPGRecvPos;
        tFileList *fileSendList;
        int fileSendCount;
        String outputPath;
        bool disableFileTime;

        byte rootPassword[TIGER_HASH_SIZE];

        byte sessionSalt[SESSION_SALT_SIZE];
        byte argonSalt[ARGON_SALT_SIZE];
        byte rootKey[ARGON_HASH_SIZE];
        byte rootPRK[SHA256_HASH_SIZE];

        byte receiveSessionSalt[SESSION_SALT_SIZE];
        byte receiveArgonSalt[ARGON_SALT_SIZE];
        byte receiveRootKey[ARGON_HASH_SIZE];
        byte receiveRootPRK[SHA256_HASH_SIZE];

        TStringList *localNetList;
        TStringList *localNetTTL;
public: // Déclarations de l'utilisateur
        __fastcall TMainForm(TComponent* Owner);
        void SendMessage(String message);

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
