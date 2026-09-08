//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Clipboard.h"
#include "MainExchange.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfClipboard *fClipboard;
//---------------------------------------------------------------------------
__fastcall TfClipboard::TfClipboard(TComponent* Owner)
   : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TfClipboard::FormShow(TObject *Sender)
{
   (void)Sender;

   this->Caption = STR_CLIPBOARD_CAP;
   this->bSend->Caption = CAP_CLIPBOARD_BTN;
   this->mText->Hint = STR_MEMO_HINT;
}
//---------------------------------------------------------------------------
void __fastcall TfClipboard::bSendClick(TObject *Sender)
{
   (void)Sender;

   if (MainForm->edtIp->Text != "" || MainForm->lbLocalNet->ItemIndex >= 0) {
      if (this->eText->Text != "") {
         this->mText->Lines->Add(MSG_SEND_MSG_INIT);
         this->mText->Lines->Add(this->eText->Text);

         MainForm->SendMessage(this->eText->Text);
         this->eText->Clear();
      }
   } else
      ShowMessage(MSG_SEND_IP_ERR);
}
//---------------------------------------------------------------------------
void __fastcall TfClipboard::mTextMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y)
{
   (void)Sender;
   (void)X;
   (void)Y;

   if (Button == mbLeft && Shift.Contains(ssCtrl)) {
      this->mText->Lines->Clear();
      this->eText->Clear();
   }
}
//---------------------------------------------------------------------------

void __fastcall TfClipboard::eTextKeyPress(TObject *Sender, System::WideChar &Key)

{
   (void)Sender;

   if (Key == VK_RETURN)
      this->bSend->Click();
}
//---------------------------------------------------------------------------

