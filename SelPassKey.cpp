//---------------------------------------------------------------------------

#include <vcl.h>
#include <System.hpp>
//#include <System.Hash.hpp>
#include <IdGlobal.hpp>
#pragma hdrstop

#include "Crypto.h"

#include "SelPassKey.h"
#include "MainExchange.h"
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
void __fastcall TfSelPassKey::FormShow(TObject *Sender)
{
   (void)Sender;

   eKey->Text.Unique();

   this->Caption = STR_SELKEY_CAP;
   this->lKey->Caption = STR_SELKEY_LBL; 
   this->bCancel->Caption = CAP_SELKEY_BTN;
   this->bOk->Caption = CAP_SELKEY_OK;

   this->eKey->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TfSelPassKey::eKeyKeyPress(TObject *Sender, System::WideChar &Key)
{
   (void)Sender;

   if (Key == VK_RETURN) { //0x0d
      this->bOk->Click();
      Key = 0;
   }
}
//---------------------------------------------------------------------------
void __fastcall TfSelPassKey::bbViewKeyClick(TObject *Sender)
{
   (void)Sender;
   //bbViewKey->Enabled = false;
   String KeyPass;

   KeyPass.Unique();
   KeyPass = eKey->Text;

   if ( eKey->PasswordChar == '*') {
      eKey->PasswordChar = 0x0;
   } else {
      eKey->PasswordChar = '*';
   }

   eKey->Text = KeyPass;
   this->WipeString(&KeyPass);
}
//---------------------------------------------------------------------------
void __fastcall TfSelPassKey::bCancelClick(TObject *Sender)
{
   (void)Sender;

   this->WipeEditBox(eKey);
   this->ModalResult = mrCancel;
}
//---------------------------------------------------------------------------
void __fastcall TfSelPassKey::bOkClick(TObject *Sender)
{
   (void)Sender;

   if (eKey->Text.Length() <= 0) {
      this->ModalResult = mrCancel;
      return;
   }

   UTF8String passwordConv;
   passwordConv.Unique();
   passwordConv = eKey->Text;
   this->WipeEditBox(eKey);
   
   if (!GetSecureRandomBytes(this->sessionSalt, sizeof(this->sessionSalt))) {
      ShowMessage(MSG_RNGEN_ERR);

      this->WipeUTF8String(&passwordConv);

      this->ModalResult = mrCancel;
      return;
   }

   uint32_t passwordLength;
   if (passwordConv.Length() < static_cast<int>(PASSWORD_MAX_SIZE)) {
      passwordLength = passwordConv.Length();
   } else {
      passwordLength = PASSWORD_MAX_SIZE;
   }

   byte password[PASSWORD_MAX_SIZE+sizeof(applicationSalt)];
   
   memset(password, 0x00, sizeof(password));
   memcpy(password, passwordConv.c_str(), passwordLength);
   memcpy(password+PASSWORD_MAX_SIZE, applicationSalt, sizeof(applicationSalt));

   GetTigerHash(password, sizeof(password), this->rootPassword);

   byte passwordHash[TIGER_HASH_SIZE+SESSION_SALT_SIZE];
   memcpy(passwordHash, this->rootPassword, TIGER_HASH_SIZE);
   memcpy(passwordHash+TIGER_HASH_SIZE, this->sessionSalt, SESSION_SALT_SIZE);
   
   passwordLength = 0;

   if (!GetPasswordHash(passwordHash, sizeof(passwordHash), this->argonSalt, this->sessionKey)) {
      ShowMessage(MSG_PWRD_GEN_ERR);

      this->WipeUTF8String(&passwordConv);
      crypto_wipe(password, sizeof(password));
      crypto_wipe(passwordHash, sizeof(passwordHash));
      crypto_wipe(this->rootPassword, sizeof(passwordHash));

      this->ModalResult = mrCancel;
      return;
   }

   this->WipeUTF8String(&passwordConv);
   crypto_wipe(password, sizeof(password));
   crypto_wipe(passwordHash, sizeof(passwordHash));
   
   this->ModalResult = mrOk;
}
//---------------------------------------------------------------------------
void __fastcall TfSelPassKey::bTigerClick(TObject *Sender)
{
   (void)Sender;

   ShowMessage("Debug");
}
//---------------------------------------------------------------------------

void __fastcall TfSelPassKey::lKeyDblClick(TObject *Sender)
{
   (void)Sender;

   this->bTiger->Visible = true;
}
//---------------------------------------------------------------------------

void TfSelPassKey::WipeString(String *input)
{
   int length = input->Length();
   WideChar* buffer = &(*input)[1];

   if (length > 0) {
      SecureZeroMemory(buffer, length * sizeof(WideChar));
   }

   *input = "";
}
//---------------------------------------------------------------------------

void TfSelPassKey::WipeUTF8String(UTF8String *input)
{
   int length = input->Length();
   char* buffer = &(*input)[1];

   if (length > 0) {
      SecureZeroMemory(buffer, length * sizeof(WideChar));
   }

   *input = "";
}

//---------------------------------------------------------------------------

void TfSelPassKey::WipeEditBox(TEdit *input)
{
   int length = input->Text.Length();

   if (length > 0) {
      input->Text = String("X", length);
   }

   input->Clear();
}
//---------------------------------------------------------------------------


