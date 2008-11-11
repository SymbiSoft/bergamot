/**
 * This file is a part of Escarpod Podcast project
 * (c) 2008 The Bergamot Project
 * (c) 2008 Teknolog (Sebastian Brännström)
 * (c) 2008 Anotherguest (Lars Persson)
 */

#include "PodcastFeedView.h"
#include "PodcastAppUi.h"
#include <aknnavide.h> 
#include <podcast.rsg>
#include "FeedEngine.h"
#include <podcast.mbg>
#include <gulicon.h>
#include <eikenv.h>

class CPodcastFeedContainer : public CCoeControl
    {
    public: 
		CPodcastFeedContainer();
		~CPodcastFeedContainer();
		void ConstructL( const TRect& aRect );
	protected:
		CAknNavigationDecorator* iNaviDecorator;
        CAknNavigationControlContainer* iNaviPane;
	};

CPodcastFeedContainer::CPodcastFeedContainer()
{
}

void CPodcastFeedContainer::ConstructL( const TRect& aRect )
{
	CreateWindowL();

	 // Set the windows size
    SetRect( aRect );    
    
    // Activate the window, which makes it ready to be drawn
    ActivateL();   
}

CPodcastFeedContainer::~CPodcastFeedContainer()
{
	delete iNaviDecorator;
}


CPodcastFeedView* CPodcastFeedView::NewL(CPodcastModel& aPodcastModel)
    {
    CPodcastFeedView* self = CPodcastFeedView::NewLC(aPodcastModel);
    CleanupStack::Pop( self );
    return self;
    }

CPodcastFeedView* CPodcastFeedView::NewLC(CPodcastModel& aPodcastModel)
    {
    CPodcastFeedView* self = new ( ELeave ) CPodcastFeedView(aPodcastModel);
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

CPodcastFeedView::CPodcastFeedView(CPodcastModel& aPodcastModel):iPodcastModel(aPodcastModel), iFeeds(1)
{
}

void CPodcastFeedView::ConstructL()
{
	BaseConstructL(R_PODCAST_FEEDVIEW);	
	CPodcastListView::ConstructL();
	
	CArrayPtr< CGulIcon >* icons = new(ELeave) CArrayPtrFlat< CGulIcon >(1);
	CleanupStack::PushL( icons );

	// Load the bitmap for feed icon	
	CFbsBitmap* bitmap = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastFeed_40x40);
	CleanupStack::PushL( bitmap );		
	// Load the mask for feed icon	
	CFbsBitmap* mask = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastFeed_40x40m );	
	CleanupStack::PushL( mask );
	// Append the feed icon to icon array
	icons->AppendL( CGulIcon::NewL( bitmap, mask ) );
	CleanupStack::Pop(2); // bitmap, mask

	iListContainer->Listbox()->ItemDrawer()->FormattedCellData()->SetIconArrayL( icons );
	CleanupStack::Pop(icons); // icons

	iListContainer->Listbox()->SetListBoxObserver(this);
}
    
CPodcastFeedView::~CPodcastFeedView()
    {
    }

TUid CPodcastFeedView::Id() const
{
	return KUidPodcastFeedViewID;
}
		
void CPodcastFeedView::DoActivateL(const TVwsViewId& aPrevViewId,
	                                  TUid aCustomMessageId,
	                                  const TDesC8& aCustomMessage)
{
	CPodcastListView::DoActivateL(aPrevViewId, aCustomMessageId, aCustomMessage);
	

	const RFeedInfoArray &feeds =  iPodcastModel.FeedEngine().GetSortedFeeds();


	iFeeds.Reset();
	for (int i=0;i<feeds.Count();i++) {
		iFeeds.AppendL(feeds[i]->Title());
	}
	
	iListContainer->Listbox()->Model()->SetItemTextArray(&iFeeds);

	
}

void CPodcastFeedView::DoDeactivate()
{
	CPodcastListView::DoDeactivate();
}


void CPodcastFeedView::HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType)
{
	
}
