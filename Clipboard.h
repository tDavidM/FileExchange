//---------------------------------------------------------------------------

#ifndef ClipboardH
#define ClipboardH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
//---------------------------------------------------------------------------
class TfClipboard : public TForm
{
__published:	// IDE-managed Components
   TButton *bSend;
   TMemo *mText;
   TEdit *eText;
   void __fastcall bSendClick(TObject *Sender);
   void __fastcall mTextMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
private:	// User declarations
public:		// User declarations
   __fastcall TfClipboard(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfClipboard *fClipboard;
//---------------------------------------------------------------------------
#endif
