/**
 * This file is a part of Escarpod Podcast project
 * (c) 2008 The Bergamot Project
 * (c) 2008 Teknolog (Sebastian Brännström)
 * (c) 2008 Anotherguest (Lars Persson)
 */

#include "PodcastSettingsView.h"
#include "PodcastAppUi.h"
#include "Podcast.hrh"
#include <aknlists.h>
#include <aknsettingitemlist.h>
#include <aknnavide.h> 
#include <podcast.rsg>

class CPodcastSettingsContainer : public CCoeControl
    {
    public: 
		CPodcastSettingsContainer();
		~CPodcastSettingsContainer();
		void ConstructL( const TRect& aRect );
		TInt CountComponentControls() const;
		CCoeControl* ComponentControl( TInt aIndex ) const;
	protected:
		CAknSettingItemList  * iListbox;
		CAknNavigationDecorator* iNaviDecorator;
        CAknNavigationControlContainer* iNaviPane;
	};

CPodcastSettingsContainer::CPodcastSettingsContainer()
{
}


TInt CPodcastSettingsContainer::CountComponentControls() const
{
	return iListbox!=NULL?1:0;
}

CCoeControl* CPodcastSettingsContainer::ComponentControl( TInt aIndex ) const
{
	return iListbox;
}

void CPodcastSettingsContainer::ConstructL( const TRect& aRect )
	{
	CreateWindowL();
	iListbox =new (ELeave) CAknSettingItemList;
	iListbox->SetMopParent( this );
	iListbox->SetContainerWindowL(*this);
	iListbox->ConstructFromResourceL(R_PODCAST_SETTINGS);
	iListbox->SetSize(aRect.Size());

	iListbox->MakeVisible(ETrue);
	// Set the windows size
	SetRect( aRect );    
	MakeVisible(EFalse);
	// Activate the window, which makes it ready to be drawn
	ActivateL();   
	}

CPodcastSettingsContainer::~CPodcastSettingsContainer()
{
	delete iNaviDecorator;
	delete iListbox;
}


CPodcastSettingsView* CPodcastSettingsView::NewL(CPodcastModel& aPodcastModel)
    {
    CPodcastSettingsView* self = CPodcastSettingsView::NewLC(aPodcastModel);
    CleanupStack::Pop( self );
    return self;
    }

CPodcastSettingsView* CPodcastSettingsView::NewLC(CPodcastModel& aPodcastModel)
    {
    CPodcastSettingsView* self = new ( ELeave ) CPodcastSettingsView(aPodcastModel);
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

CPodcastSettingsView::CPodcastSettingsView(CPodcastModel& aPodcastModel):iPodcastModel(aPodcastModel)
{
}

void CPodcastSettingsView::ConstructL()
{
	BaseConstructL(R_PODCAST_SETTINGSVIEW);	
	iSettingsContainer = new (ELeave) CPodcastSettingsContainer;
	iSettingsContainer->ConstructL(ClientRect());
}
    
CPodcastSettingsView::~CPodcastSettingsView()
    {
    delete iSettingsContainer;    
    }

TUid CPodcastSettingsView::Id() const
{
	return KUidPodcastSettingsViewID;
}
		
void CPodcastSettingsView::DoActivateL(const TVwsViewId& aPrevViewId,
	                                  TUid aCustomMessageId,
	                                  const TDesC8& aCustomMessage)
{
	iPreviousView = aPrevViewId;
	
	if(iSettingsContainer)
		{
		iSettingsContainer->SetRect(ClientRect());
		AppUi()->AddToViewStackL( *this, iSettingsContainer );	
		iSettingsContainer->MakeVisible(ETrue);
		iSettingsContainer->DrawNow();
		}
}

void CPodcastSettingsView::DoDeactivate()
	{
	if ( iSettingsContainer )
		{
		AppUi()->RemoveFromViewStack( *this, iSettingsContainer );
		iSettingsContainer->MakeVisible(EFalse);
		}
	}

/** 
* Command handling function intended for overriding by sub classes. 
* Default implementation is empty.  
* @param aCommand ID of the command to respond to. 
*/
void CPodcastSettingsView::HandleCommandL(TInt aCommand)
{
	RDebug::Printf("CPodcastListView::HandleCommandL=%d", aCommand);
	switch(aCommand)
	{
	case EAknSoftkeyExit:
	case EEikCmdExit:
		{
            AppUi()->Exit();
            break;
		}
	case EAknSoftkeyBack:
		{
		AppUi()->ActivateViewL(iPreviousView);
		}break;	
	case EPodcastZoomSetting:
		break;
	case EPodcastAbout:
		break;
	default:
		break;
	}
}