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
class CPodcastSettingItemList:public CAknSettingItemList
	{
public:
	CPodcastSettingItemList()
		{

		}
	
	~CPodcastSettingItemList()
		{

		}

	/**
	 * Framework method to create a setting item based upon the user id aSettingId. The 
	 * client code decides what type to contruct. new (ELeave) must then be used and the resulting 
	 * pointer returned. Ownership is thereafter base class's responsiblity.
	 *
	 * @param aSettingId	ID to use to determine the type of the setting item
	 * @return a constructed (not 2nd-stage constructed) setting item.
	 */
	CAknSettingItem* CreateSettingItemL( TInt aSettingId )
		{
		switch(aSettingId)
			{
			case 1:
			default:
				return CAknSettingItemList::CreateSettingItemL(aSettingId);
				break;
			}
		return NULL;
		}
	};

class CPodcastSettingsContainer : public CCoeControl
    {
    public: 
		CPodcastSettingsContainer();
		~CPodcastSettingsContainer();
		void ConstructL( const TRect& aRect );
		TInt CountComponentControls() const;
		CCoeControl* ComponentControl( TInt aIndex ) const;
	protected:
		CPodcastSettingItemList  * iListbox;
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

CCoeControl* CPodcastSettingsContainer::ComponentControl( TInt /*aIndex*/ ) const
{
	return iListbox;
}

void CPodcastSettingsContainer::ConstructL( const TRect& aRect )
	{
	CreateWindowL();
	iListbox =new (ELeave) CPodcastSettingItemList;
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
	                                  TUid /*aCustomMessageId*/,
	                                  const TDesC8& /*aCustomMessage*/)
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