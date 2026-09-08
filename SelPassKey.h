//---------------------------------------------------------------------------

#ifndef SelPassKeyH
#define SelPassKeyH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.Buttons.hpp>

#include "Crypto.h"
//---------------------------------------------------------------------------
class TfSelPassKey : public TForm
{
__published:	// IDE-managed Components
   TButton *bOk;
   TButton *bCancel;
   TEdit *eKey;
   TLabel *lKey;
   TBitBtn *bbViewKey;
   TButton *bTiger;
   void __fastcall bbViewKeyClick(TObject *Sender);
   void __fastcall bCancelClick(TObject *Sender);
   void __fastcall bOkClick(TObject *Sender);
   void __fastcall bTigerClick(TObject *Sender);
   void __fastcall lKeyDblClick(TObject *Sender);
    void __fastcall FormShow(TObject *Sender);
    void __fastcall eKeyKeyPress(TObject *Sender, System::WideChar &Key);
private:	// User declarations
   void TfSelPassKey::WipeString(String *input);
   void TfSelPassKey::WipeUTF8String(UTF8String *input);
   void TfSelPassKey::WipeEditBox(TEdit *input);
public:		// User declarations
   byte sessionSalt[SESSION_SALT_SIZE];
   byte argonSalt[ARGON_SALT_SIZE];
   byte sessionKey[ARGON_HASH_SIZE];
   byte rootPassword[TIGER_HASH_SIZE];

   __fastcall TfSelPassKey(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfSelPassKey *fSelPassKey;
//---------------------------------------------------------------------------
#endif
