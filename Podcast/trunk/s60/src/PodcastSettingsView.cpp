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
#include "SettingsEngine.h"

class CPodcastSettingItemList:public CAknSettingItemList
	{
public:
	CPodcastSettingItemList(CPodcastModel& aPodcastModel) : iPodcastModel(aPodcastModel)
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
		CSettingsEngine &se = iPodcastModel.SettingsEngine();
		iShowDir.Copy(se.BaseDir());
		iIntervalUpdate = se.UpdateFeedInterval();
		iTimeUpdate = se.UpdateFeedTime();
		iAutoDownload = se.DownloadAutomatically();
		iConnection = se.SpecificIAP();
		
		switch(aSettingId)
			{
			case EPodcastSettingShowDir:
				return new (ELeave) CAknTextSettingItem(aSettingId, iShowDir);
				break;
			case EPodcastSettingUpdateInterval:
				return new (ELeave) CAknIntegerEdwinSettingItem(aSettingId, iIntervalUpdate);
				break;
			case EPodcastSettingUpdateTime:
				return new (ELeave) CAknTimeOrDateSettingItem(aSettingId, CAknTimeOrDateSettingItem::ETime, iTimeUpdate);
				break;
			case EPodcastSettingConnection:
				return new (ELeave) CAknEnumeratedTextPopupSettingItem (aSettingId, iConnection);
				break;
			case EPodcastSettingAutoDownload:
				return new (ELeave) CAknBinaryPopupSettingItem (aSettingId, iAutoDownload);
				break;
			default:
				return CAknSettingItemList::CreateSettingItemL(aSettingId);
				break;
			}
		return NULL;
		}
	TFileName iShowDir;
	TInt iIntervalUpdate;
	TTime iTimeUpdate;
	TInt iAutoDownload;
	TInt iConnection;
	
	CPodcastModel &iPodcastModel;
	};

class CPodcastSettingsContainer : public CCoeControl
    {
    public: 
		CPodcastSettingsContainer(CPodcastModel& aPodcastModel);
		~CPodcastSettingsContainer();
		void ConstructL( const TRect& aRect );
		TInt CountComponentControls() const;
		CCoeControl* ComponentControl( TInt aIndex ) const;
    protected:
    	TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
    	{
    		return iListbox->OfferKeyEventL(aKeyEvent, aType);
    	}
		void HandleResourceChange(TInt aType);
	protected:
		CPodcastSettingItemList  * iListbox;
		CAknNavigationDecorator* iNaviDecorator;
        CAknNavigationControlContainer* iNaviPane;
	private:
		CPodcastModel &iPodcastModel;
	};



void CPodcastSettingsContainer::HandleResourceChange(TInt aType)
{
		switch( aType )
    	{
	    case KEikDynamicLayoutVariantSwitch:
		    SetRect(iEikonEnv->EikAppUi()->ClientRect());
		    break;
	    }
}

CPodcastSettingsContainer::CPodcastSettingsContainer(CPodcastModel &aPodcastModel) : iPodcastModel(aPodcastModel)
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
	iListbox =new (ELeave) CPodcastSettingItemList(iPodcastModel);
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
	iSettingsContainer = new (ELeave) CPodcastSettingsContainer(iPodcastModel);
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

/*
CMyEnumSetting::CMyEnumSetting(TInt aResourceId, CPodcastSettingItemList *aSettingList) : CAknEnumeratedTextPopupSettingItem(aResourceId, 0)
	{
	
	}
CMyEnumSetting::~CMyEnumSetting()
	{
	
	}

void CMyEnumSetting::EditItemL(TBool aCalledFromMenu)
	{
	
	}

void CMyEnumSetting::HandleSettingPageEventL(CAknSettingPage* aSettingPage,TAknSettingPageEvent aEventType)
	{
	
	}
*/
