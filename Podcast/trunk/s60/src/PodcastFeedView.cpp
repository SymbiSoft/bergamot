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

CPodcastFeedView::CPodcastFeedView(CPodcastModel& aPodcastModel):iPodcastModel(aPodcastModel)
{
}

void CPodcastFeedView::ConstructL()
{
	BaseConstructL(R_PODCAST_FEEDVIEW);	
	CPodcastListView::ConstructL();
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
}

void CPodcastFeedView::DoDeactivate()
{
	CPodcastListView::DoDeactivate();
}