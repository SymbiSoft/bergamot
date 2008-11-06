#include "PodcastAppui.h"
#include <Podcast.rsg>
#include "Podcast.hrh"
#include "PodcastMainView.h"
#include "PodcastFeedView.h"
#include "PodcastShowsView.h"
#include "PodcastPlayView.h"
#include "PodcastSettingsView.h"
#include <avkon.hrh>

void CPodcastAppUi::ConstructL()
    {
    BaseConstructL( CAknAppUi::EAknEnableSkin );
    iMainView = CPodcastMainView::NewL( ClientRect() );
	this->AddViewL(iMainView);

	iFeedView = CPodcastFeedView::NewL( ClientRect() );
	this->AddViewL(iFeedView);

	iShowsView = CPodcastShowsView::NewL( ClientRect() );
	this->AddViewL(iShowsView);

	iPlayView = CPodcastPlayView::NewL( ClientRect() );
	this->AddViewL(iPlayView);

	iSettingsView = CPodcastSettingsView::NewL( ClientRect() );
	this->AddViewL(iSettingsView);
    }

CPodcastAppUi::~CPodcastAppUi()
    {	
/*    if ( iAppContainer )
        {
        RemoveFromStack( iAppContainer );
        delete iAppContainer;
        iAppContainer = NULL;
        }
		*/
    }

// -----------------------------------------------------------------------------
// CPodcastAppUi::DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane)
//  This function is called by the EIKON framework just before it displays
//  a menu pane. Its default implementation is empty, and by overriding it,
//  the application can set the state of menu items dynamically according
//  to the state of application data.
// ------------------------------------------------------------------------------
//
void CPodcastAppUi::DynInitMenuPaneL(
    TInt /*aResourceId*/,CEikMenuPane* /*aMenuPane*/)
    {
    // no implementation required 
    }

// -----------------------------------------------------------------------------
// CPodcastAppUi::HandleKeyEventL(
//     const TKeyEvent& aKeyEvent,TEventCode /*aType*/)
// takes care of key event handling
// -----------------------------------------------------------------------------
//
TKeyResponse CPodcastAppUi::HandleKeyEventL(
    const TKeyEvent& /*aKeyEvent*/,TEventCode /*aType*/ )
    {
    return EKeyWasNotConsumed;
    }

// -----------------------------------------------------------------------------
// CPodcastAppUi::HandleCommandL(TInt aCommand)
// takes care of command handling
// -----------------------------------------------------------------------------
//
void CPodcastAppUi::HandleCommandL( TInt aCommand )
    {
    switch ( aCommand )
        {
        case EAknSoftkeyExit:
        case EEikCmdExit:
            {
            Exit();
            break;
            }
        default:
            break;      
        }
    }

// -----------------------------------------------------------------------------
// CPodcastAppUi::HandleResourceChangeL( TInt aType )
// Called by framework when layout is changed.
// -----------------------------------------------------------------------------
//
void CPodcastAppUi::HandleResourceChangeL( TInt aType )
    {
    CAknAppUi::HandleResourceChangeL( aType );

    if ( aType==KEikDynamicLayoutVariantSwitch )
        {
        //iAppContainer->SetRect( ClientRect() );
        }

//    iAppContainer->HandleResourceChange( aType );
    }
