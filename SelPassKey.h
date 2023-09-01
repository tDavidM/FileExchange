//---------------------------------------------------------------------------

#ifndef SelPassKeyH
#define SelPassKeyH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.Buttons.hpp>
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
private:	// User declarations
public:		// User declarations
   String        KeyString;
   TByteDynArray KeyBytes;

   __fastcall TfSelPassKey(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfSelPassKey *fSelPassKey;
//---------------------------------------------------------------------------
#endif
