/**
 * This file is a part of Escarpod Podcast project
 * (c) 2008 The Bergamot Project
 * (c) 2008 Teknolog (Sebastian Brännström)
 * (c) 2008 Anotherguest (Lars Persson)
 */

#include "PodcastShowsView.h"

#include <aknnavide.h> 
#include <podcast.rsg>

class CPodcastShowsContainer : public CCoeControl
    {
    public: 
		CPodcastShowsContainer();
		~CPodcastShowsContainer();
		void ConstructL( const TRect& aRect );
	protected:
		CAknNavigationDecorator* iNaviDecorator;
        CAknNavigationControlContainer* iNaviPane;
	};

CPodcastShowsContainer::CPodcastShowsContainer()
{
}

void CPodcastShowsContainer::ConstructL( const TRect& aRect )
{
	CreateWindowL();

	 // Set the windows size
    SetRect( aRect );    
    
    // Activate the window, which makes it ready to be drawn
    ActivateL();   
}

CPodcastShowsContainer::~CPodcastShowsContainer()
{
	delete iNaviDecorator;
}

CPodcastShowsView* CPodcastShowsView::NewL()
    {
    CPodcastShowsView* self = CPodcastShowsView::NewLC();
    CleanupStack::Pop( self );
    return self;
    }

CPodcastShowsView* CPodcastShowsView::NewLC()
    {
    CPodcastShowsView* self = new ( ELeave ) CPodcastShowsView();
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

CPodcastShowsView::CPodcastShowsView()
{
}

void CPodcastShowsView::ConstructL()
{
	BaseConstructL(R_PODCAST_SHOWSVIEW);	
	CPodcastListView::ConstructL();
}
    
CPodcastShowsView::~CPodcastShowsView()
    {
    }

TUid CPodcastShowsView::Id() const
{
	return TUid::Uid(3);
}
		
void CPodcastShowsView::DoActivateL(const TVwsViewId& aPrevViewId,
	                                  TUid aCustomMessageId,
	                                  const TDesC8& aCustomMessage)
{
	CPodcastListView::DoActivateL(aPrevViewId, aCustomMessageId, aCustomMessage);
}

void CPodcastShowsView::DoDeactivate()
{
	CPodcastListView::DoDeactivate();
}