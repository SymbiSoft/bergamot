/**
 * This file is a part of Escarpod Podcast project
 * (c) 2008 The Bergamot Project
 * (c) 2008 Teknolog (Sebastian Brännström)
 * (c) 2008 Anotherguest (Lars Persson)
 */

#include "PodcastMainView.h"

#include <aknnavide.h> 
#include <podcast.rsg>

class CPodcastMainContainer : public CCoeControl
    {
    public: 
		CPodcastMainContainer();
		~CPodcastMainContainer();
		void ConstructL( const TRect& aRect );
	protected:
		CAknNavigationDecorator* iNaviDecorator;
        CAknNavigationControlContainer* iNaviPane;
	};

CPodcastMainContainer::CPodcastMainContainer()
{
}

void CPodcastMainContainer::ConstructL( const TRect& aRect )
{
	CreateWindowL();

	 // Set the windows size
    SetRect( aRect );    
    
    // Activate the window, which makes it ready to be drawn
    ActivateL();   
}

CPodcastMainContainer::~CPodcastMainContainer()
{
	delete iNaviDecorator;
}

CPodcastMainView* CPodcastMainView::NewL( const TRect& aRect )
    {
    CPodcastMainView* self = CPodcastMainView::NewLC( aRect );
    CleanupStack::Pop( self );
    return self;
    }

CPodcastMainView* CPodcastMainView::NewLC( const TRect& aRect )
    {
    CPodcastMainView* self = new ( ELeave ) CPodcastMainView();
    CleanupStack::PushL( self );
    self->ConstructL( aRect );
    return self;
    }

CPodcastMainView::CPodcastMainView()
{
}

void CPodcastMainView::ConstructL( const TRect& aRect )
{
	BaseConstructL(R_PODCAST_MAINVIEW);
	iMainContainer = new (ELeave) CPodcastMainContainer;
	iMainContainer->ConstructL(aRect);
}
    
CPodcastMainView::~CPodcastMainView()
    {
    delete iMainContainer;    
    }

TUid CPodcastMainView::Id() const
{
	return TUid::Uid(1);
}
		
void CPodcastMainView::DoActivateL(const TVwsViewId& aPrevViewId,
	                                  TUid aCustomMessageId,
	                                  const TDesC8& aCustomMessage)
{
}

void CPodcastMainView::DoDeactivate()
{
}