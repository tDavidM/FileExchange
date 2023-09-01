//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------
USEFORM("SelPassKey.cpp", fSelPassKey);
USEFORM("MainExchange.cpp", MainForm);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
   try
   {
      Application->Initialize();
      Application->MainFormOnTaskBar = true;
		Application->Title = "Exchange";
      Application->CreateForm(__classid(TMainForm), &MainForm);
       Application->CreateForm(__classid(TfSelPassKey), &fSelPassKey);
       Application->Icon = Application->MainForm->Icon;
      Application->Run();
   }
   catch (Exception &exception)
   {
      Application->ShowException(&exception);
   }
   catch (...)
   {
      try
      {
         throw Exception("");
      }
      catch (Exception &exception)
      {
         Application->ShowException(&exception);
      }
   }
   return 0;
}
//---------------------------------------------------------------------------
