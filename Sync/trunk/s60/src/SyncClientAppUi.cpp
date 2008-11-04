// INCLUDE FILES
#include <avkon.hrh>
#include "SyncClientAppUi.h"
#include "SyncClientAppView.h"
#include "SyncClientS60.hrh"
#include <aknnotedialog.h> 
#include <SyncClientS60.rsg>

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
    }

// Constructor
CSyncClientAppUi::CSyncClientAppUi()                              
    {
	// no implementation required
    }

// Destructor
CSyncClientAppUi::~CSyncClientAppUi()
    {
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
      default:
            break;
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

// End of file