#include "SyncClientAppUi.h"
#include "SyncClientView.h"
#include <QikCommand.h>
#include <EIKDIALG.H>
#include "SyncClient.hrh"
#include "SyncClientView.h"
#include <SyncClient.rsg>
#include <EIKLABEL.H>

class CAboutDlg:public CEikDialog
{
public:
  CAboutDlg()
  {
  }

  ~CAboutDlg()
  {
  }

  void PreLayoutDynInitL()
  {
	  CEikLabel* label = static_cast<CEikLabel*>(ControlOrNull(ESwimAboutText));
	  if(label != NULL)
	  {
		  HBufC* aboutText = iEikonEnv->AllocReadResourceLC(R_SWIM_ABOUT_TEXT);
		  label->SetTextL(*aboutText);
		  CleanupStack::PopAndDestroy(aboutText);
	  }
  }

};

/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
void CSyncClientAppUi::ConstructL()
{
  CQikAppUi::ConstructL();
  CSyncClientView* baseView = CSyncClientView::NewLC(*this);
  AddViewL(*baseView);
  iBaseView = baseView;
  CleanupStack::Pop(baseView);
}

/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
CSyncClientAppUi::~CSyncClientAppUi()
{
}

/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
void CSyncClientAppUi::HandleCommandL(CQikCommand &aCommand)
{
  TQikViewMode viewMode = iBaseView->ViewMode();

  switch(aCommand.Id())
  {
        case EMyAbout:
        {
			CEikDialog* dlg = new (ELeave) CAboutDlg;
			dlg->ExecuteLD(R_SWIM_ABOUT_DLG);
        }
        return;
  }
  CQikAppUi::HandleCommandL(aCommand);
}
