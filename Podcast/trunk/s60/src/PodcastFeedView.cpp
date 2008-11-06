/**
 * This file is a part of Escarpod Podcast project
 * (c) 2008 The Bergamot Project
 * (c) 2008 Teknolog (Sebastian Brännström)
 * (c) 2008 Anotherguest (Lars Persson)
 */


#include "PodcastFeedView.h"

#include <aknnavide.h> 

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


CPodcastFeedView* CPodcastFeedView::NewL( const TRect& aRect )
    {
    CPodcastFeedView* self = CPodcastFeedView::NewLC( aRect );
    CleanupStack::Pop( self );
    return self;
    }

CPodcastFeedView* CPodcastFeedView::NewLC( const TRect& aRect )
    {
    CPodcastFeedView* self = new ( ELeave ) CPodcastFeedView();
    CleanupStack::PushL( self );
    self->ConstructL( aRect );
    return self;
    }

CPodcastFeedView::CPodcastFeedView()
{
}

void CPodcastFeedView::ConstructL( const TRect& aRect )
{
	BaseConstructL();
	iFeedContainer = new (ELeave) CPodcastFeedContainer;
	iFeedContainer->ConstructL(aRect);
}
    
CPodcastFeedView::~CPodcastFeedView()
    {
    delete iFeedContainer;    
    }

TUid CPodcastFeedView::Id() const
{
	return TUid::Uid(2);
}
		
void CPodcastFeedView::DoActivateL(const TVwsViewId& aPrevViewId,
	                                  TUid aCustomMessageId,
	                                  const TDesC8& aCustomMessage)
{
}

void CPodcastFeedView::DoDeactivate()
{
}