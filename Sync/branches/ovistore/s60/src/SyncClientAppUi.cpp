// INCLUDE FILES
#include <avkon.hrh>
#include "SyncClientAppUi.h"
#include "SyncClientAppView.h"
#include "SyncClientS60.hrh"
#include <aknnotedialog.h> 
#include "debug.h"
#include <SyncClientS60_0x200409D8.rsg>
#include <aknmessagequerydialog.h>
#include <hlplch.h>  // HlpLauncher
#include "..\help\help.hlp.hrh"

const TUid KUidHelpFile = {0x200409D8};  // From the help file

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

void CSyncClientAppUi::HandleStatusPaneSizeChange()
	{
	const TRect r = ClientRect();
	DP2("HandleStatusPaneSizeChange, clientrect w=%d, h=%d", r.Width(), r.Height());
	iAppView->SetSize(r.Size());
	iAppView->SizeChanged();
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
        case ECmdHelp:
        	ShowHelp();
        	break;
      default:
            break;
        }
    }

void CSyncClientAppUi::ShowHelp()
	{
	CArrayFix <TCoeHelpContext>* buf = CCoeAppUi::AppHelpContextL();
	    HlpLauncher::LaunchHelpApplicationL(iEikonEnv->WsSession(), buf);
	}

void CSyncClientAppUi::RunAboutDialogL()
{
	HBufC *aboutTextTitle = iEikonEnv->AllocReadResourceLC(R_ABOUT_TITLE);
	HBufC *aboutTextBody = iEikonEnv->AllocReadResourceLC(R_ABOUT_BODY);
	
	CAknMessageQueryDialog* note = new ( ELeave ) CAknMessageQueryDialog(aboutTextBody, aboutTextTitle );
						
	note->PrepareLC( R_SHOW_INFO_NOTE ); // Adds to CleanupStack
	note->RunLD();

	CleanupStack::Pop(aboutTextBody);
	CleanupStack::Pop(aboutTextTitle);
}

CArrayFix <TCoeHelpContext>* CSyncClientAppUi::HelpContextL() const
    {
    CArrayFixFlat <TCoeHelpContext>* array =
        new (ELeave) CArrayFixFlat <TCoeHelpContext>(1);
    CleanupStack::PushL(array);
    // KContextApplication below should refer to the context declared in
    // help.rtf
    array->AppendL(TCoeHelpContext(KUidHelpFile, KGettingStarted));
    CleanupStack::Pop(array);
    return array;
    }
