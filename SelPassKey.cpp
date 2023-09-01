//---------------------------------------------------------------------------

#include <vcl.h>
#include <System.hpp>
//#include <System.Hash.hpp>
#include <IdGlobal.hpp>
#pragma hdrstop

#include "sboxes.c"
#include "tiger.c"

#include "SelPassKey.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfSelPassKey *fSelPassKey;
//---------------------------------------------------------------------------
__fastcall TfSelPassKey::TfSelPassKey(TComponent* Owner)
   : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TfSelPassKey::bbViewKeyClick(TObject *Sender)
{
   //bbViewKey->Enabled = false;
   String KeyPass = "TIGER01234567890123456789";
   KeyPass = eKey->Text;
   if ( eKey->PasswordChar == '*')
      eKey->PasswordChar = 0x0;
   else
      eKey->PasswordChar = '*';
   eKey->Text = KeyPass;
   KeyPass = "TIGER01234567890123456789";
}
//---------------------------------------------------------------------------
void __fastcall TfSelPassKey::bCancelClick(TObject *Sender)
{
   eKey->Text = "TIGER01234567890123456789";
   this->ModalResult = mrCancel;
}
//---------------------------------------------------------------------------
void __fastcall TfSelPassKey::bOkClick(TObject *Sender)
{
   byte str[25];
   word64 res[3];

   wcstombs(str, eKey->Text.c_str(), 25);
   //192-bit (24-bytes)
   tiger((word64*)str, eKey->Text.Length(), res);

   this->KeyBytes.set_length(24);
   this->KeyString = "";

   for(int i=0; i<24; i++) {
      this->KeyBytes[i] = ((byte*)res)[i];
      this->KeyString = this->KeyString + LowerCase(ByteToHex(this->KeyBytes[i]));
   }

   eKey->Text = "TIGER01234567890123456789";
   this->ModalResult = mrOk;
}
//---------------------------------------------------------------------------
void __fastcall TfSelPassKey::bTigerClick(TObject *Sender)
{
   byte str[25] ;
   word64 res[3];
   String out;

   wcstombs(str, eKey->Text.c_str(), 25);

   tiger((word64*)str, eKey->Text.Length(), res);

   for(int i=0; i<24; i++)
      out = out + LowerCase(ByteToHex(((byte*)res)[i]));

   ShowMessage(out);
}
//---------------------------------------------------------------------------

void __fastcall TfSelPassKey::lKeyDblClick(TObject *Sender)
{
   this->bTiger->Visible = true;
}
//---------------------------------------------------------------------------

