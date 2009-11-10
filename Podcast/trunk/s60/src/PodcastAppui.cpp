#include "PodcastAppui.h"
#include <Podcast.rsg>
#include "Podcast.hrh"
#include "PodcastMainView.h"
#include "PodcastFeedView.h"
#include "PodcastShowsView.h"
#include "PodcastPlayView.h"
#include "PodcastSettingsView.h"
#include "ShowEngine.h"
#include "PodcastModel.h"

#include <avkon.hrh>

CPodcastAppUi::CPodcastAppUi(CPodcastModel* aPodcastModel):iPodcastModel(aPodcastModel)
	{
	
	}
void CPodcastAppUi::ConstructL()
    {

    BaseConstructL(CAknAppUi::EAknEnableSkin); 
    
    iMainView = CPodcastMainView::NewL(*iPodcastModel);
	this->AddViewL(iMainView);

	iFeedView = CPodcastFeedView::NewL(*iPodcastModel);
	this->AddViewL(iFeedView);

	iShowsView = CPodcastShowsView::NewL(*iPodcastModel);
	this->AddViewL(iShowsView);

	iPlayView = CPodcastPlayView::NewL(*iPodcastModel);
	this->AddViewL(iPlayView);

	iSettingsView = CPodcastSettingsView::NewL(*iPodcastModel);
	this->AddViewL(iSettingsView);
    }

CPodcastAppUi::~CPodcastAppUi()
    {	
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
