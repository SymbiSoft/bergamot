/*
* ==============================================================================
*  Name        : helloworldbasicappui.cpp
*  Part of     : Helloworldbasic
*  Interface   :
*  Description :
*  Version     :
*
*  Copyright (c) 2005-2006 Nokia Corporation.
*  This material, including documentation and any related
*  computer programs, is protected by copyright controlled by
*  Nokia Corporation.
* ==============================================================================
*/

// INCLUDE FILES
#include <avkon.hrh>
#include <aknnotewrappers.h>
#include <stringloader.h>
#include <HelloWorldBasic.rsg>
#include <f32file.h>
#include <s32file.h>
#include "debug.h"

#include "HelloWorldBasic.pan"
#include "HelloWorldBasicAppUi.h"
#include "HelloWorldBasicAppView.h"
#include "HelloWorldBasic.hrh"
#include "HelloWorldBasicQueryDialog.h"

_LIT( KHelloFileName, "Hello.txt" );

// ============================ MEMBER FUNCTIONS ===============================


// -----------------------------------------------------------------------------
// CHelloWorldBasicAppUi::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CHelloWorldBasicAppUi::ConstructL()
    {
    // Initialise app UI with standard value.
    BaseConstructL(CAknAppUi::EAknEnableSkin);
    
    iMonitor->ConstructL();
    // Create view object
    iAppView = CHelloWorldBasicAppView::NewL( ClientRect() );

    }
// -----------------------------------------------------------------------------
// CHelloWorldBasicAppUi::CHelloWorldBasicAppUi()
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CHelloWorldBasicAppUi::CHelloWorldBasicAppUi()
    {
    iMonitor = new CAccessoryMonitor(*this);
    // No implementation required
    }

// -----------------------------------------------------------------------------
// CHelloWorldBasicAppUi::~CHelloWorldBasicAppUi()
// Destructor.
// -----------------------------------------------------------------------------
//
CHelloWorldBasicAppUi::~CHelloWorldBasicAppUi()
    {
    if ( iAppView )
        {
        delete iAppView;
        iAppView = NULL;
        }
    }

// -----------------------------------------------------------------------------
// CHelloWorldBasicAppUi::HandleCommandL()
// Takes care of command handling.
// -----------------------------------------------------------------------------
//
void CHelloWorldBasicAppUi::HandleCommandL( TInt aCommand )
    {
  
    switch( aCommand )
        {
        case EEikCmdExit:
        case EAknSoftkeyExit:
            Exit();
            break;

        case EHelloWorldBasicCommand1:
            {
          case EHelloWorldBasicCommand2:
          case EHelloWorldBasicCommand3:
          default:
            break;
        }
    }
    }
// -----------------------------------------------------------------------------
//  Called by framework when layout is changed.
//  Passes the new client rectangle to the AppView
// -----------------------------------------------------------------------------
//
void CHelloWorldBasicAppUi::HandleResourceChangeL( TInt aType )
{
    // base-class call also
    CAknAppUi::HandleResourceChangeL(aType);
    if (aType == KEikDynamicLayoutVariantSwitch)
        {
        if (iAppView)
            iAppView->SetRect( ClientRect() );
        }
}

void CHelloWorldBasicAppUi::AccessoryConnected()
	{
	DP("CHelloWorldBasicAppUi::AccessoryConnected()");
	iAppView->AccessoryConnected();
	}

void CHelloWorldBasicAppUi::AccessoryDisconnected()
	{
	DP("CHelloWorldBasicAppUi::AccessoryDisconnected()");
	iAppView->AccessoryDisconnected();
	}


// End of File

