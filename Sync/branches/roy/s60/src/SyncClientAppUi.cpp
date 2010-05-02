// INCLUDE FILES
#include <avkon.hrh>
#include "SyncClientAppUi.h"
#include "SyncClientAppView.h"
#include "SyncClientS60.hrh"
#include <aknnotedialog.h> 
#include <SyncClientS60_0x2002C265.rsg>
#include "APGCLI.H"

_LIT(KMobileMoreURL,"http://store.ovi.com/publisher/SBSH/");

// ================= MEMBER FUNCTIONS =======================

// ----------------------------------------------------------
// CSyncClientAppUi::ConstructL()
// ConstructL is called by the application framework
// ----------------------------------------------------------
//
void CSyncClientAppUi::ConstructL()
    {
    BaseConstructL(EAknEnableSkin);

    iAppView = new (ELeave) CSyncClientAppView;
    iAppView->SetMopParent(this);
    iAppView->ConstructL(ClientRect());

    AddToStackL(iAppView);
    

    CAknNavigationControlContainer *iNaviContainer;

     
    CEikStatusPane* sp  = iEikonEnv->AppUiFactory()->StatusPane();
    iNaviContainer      = (CAknNavigationControlContainer *)sp->ControlL(TUid::Uid(EEikStatusPaneUidNavi));
    iNaviLabelDecorator = iNaviContainer->CreateNavigationLabelL();
     
	HBufC *navitext = iEikonEnv->AllocReadResourceLC(R_NAVI_TEXT);
    static_cast<CAknNaviLabel*>(iNaviLabelDecorator->DecoratedControl())->SetTextL(*navitext);
    	iNaviContainer->PushL(*iNaviLabelDecorator);
    

    CleanupStack::PopAndDestroy(navitext);    	
    }

// Constructor
CSyncClientAppUi::CSyncClientAppUi()                              
    {
	// no implementation required
    }

// Destructor
CSyncClientAppUi::~CSyncClientAppUi()
    {
    delete iNaviLabelDecorator;
    if (iAppView)
        {
        RemoveFromStack(iAppView);
        delete iAppView;
        iAppView = NULL;
        }
    }

// ----------------------------------------------------
// CSyncClientAppUi::HandleCommandL()
// takes care of command handling
// ----------------------------------------------------
//
void CSyncClientAppUi::HandleCommandL(TInt aCommand)
    {
    switch(aCommand)
        {
        case EEikCmdExit:
        case EAknSoftkeyExit:
            Exit();
            break;
        case ECmdAbout:
        	RunAboutDialogL();
        	break;
        case ETake3DeleteSync:
        	iAppView->DeleteSync();
        	break;
        case ETake3ViewDetails:
        	iAppView->ViewDetails();
        	break;
        case ETake3GetMore:
        	RunBuyWebBrowser(KMobileMoreURL);
        	break;
        case ENewProfileGoogleCalendar:
        case ENewProfileGoogleContacts:
        case ENewProfileOvi:
        case ENewProfileZyb:
        case ENewProfileMobical:
        	iAppView->CreateNewProfile(aCommand);
        	break;
      default:
            break;
        }
    }
     
void CSyncClientAppUi::RunBuyWebBrowser(const TDesC& aUrl)
{
	const TInt KWmlBrowserUid = 0x10008D39;
	TUid id( TUid::Uid( KWmlBrowserUid ) );
	TApaTaskList taskList( CEikonEnv::Static()->WsSession() );
	TApaTask task = taskList.FindApp( id );
	if ( task.Exists() )
	{
		HBufC8* param = HBufC8::NewLC( aUrl.Length() + 2);
                //"4 " is to Start/Continue the browser specifying a URL
		param->Des().Append(_L("4 "));
		param->Des().Append(aUrl);
		task.SendMessage( TUid::Uid( 0 ), *param ); // Uid is not used
		CleanupStack::PopAndDestroy(param);
	}
	else
	{
		HBufC16* param = HBufC16::NewLC( aUrl.Length() + 2);
                //"4 " is to Start/Continue the browser specifying a URL
		param->Des().Append(_L("4 "));
		param->Des().Append(aUrl);
		RApaLsSession appArcSession;
                // connect to AppArc server 
		User::LeaveIfError(appArcSession.Connect()); 
		TThreadId id;
		appArcSession.StartDocument( *param, TUid::Uid( KWmlBrowserUid)
                                                                       , id );
		appArcSession.Close(); 
		CleanupStack::PopAndDestroy(param);
	}
}

void CSyncClientAppUi::RunAboutDialogL()
{
    // Create dialog
	CAknNoteDialog* dlg = new(ELeave) CAknNoteDialog(
	    CAknNoteDialog::EConfirmationTone,
	    CAknNoteDialog::ELongTimeout);
    CleanupStack::PushL(dlg);

    // Show dialog
    CleanupStack::Pop(dlg);
    dlg->ExecuteLD(R_DLG_ABOUT);
}
