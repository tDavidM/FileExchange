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
void __fastcall TfClipboard::bSendClick(TObject *Sender)
{
   if (MainForm->TxtIp->Text != "" || MainForm->lbLocalNet->ItemIndex >= 0) {
      if (this->eText->Text != "") {
         this->mText->Lines->Add("Sent:");
         this->mText->Lines->Add(this->eText->Text);

         MainForm->SendMessage(this->eText->Text);
         this->eText->Clear();
      }
   } else
      ShowMessage("Specify the IP address of the server");
}
//---------------------------------------------------------------------------
void __fastcall TfClipboard::mTextMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y)
{
   if (Button == mbLeft && Shift.Contains(ssCtrl)) {
      this->mText->Lines->Clear();
      this->eText->Clear();
   }
}
//---------------------------------------------------------------------------

void __fastcall TfClipboard::eTextKeyPress(TObject *Sender, System::WideChar &Key)

{
   if (Key == VK_RETURN)
      this->bSend->Click();
}
//---------------------------------------------------------------------------

