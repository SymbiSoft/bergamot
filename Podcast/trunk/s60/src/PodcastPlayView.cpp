/**
 * This file is a part of Escarpod Podcast project
 * (c) 2008 The Bergamot Project
 * (c) 2008 Teknolog (Sebastian Brännström)
 * (c) 2008 Anotherguest (Lars Persson)
 */

#include "PodcastPlayView.h"

#include <aknnavide.h> 
#include <podcast.rsg>

class CPodcastPlayContainer : public CCoeControl
    {
    public: 
		CPodcastPlayContainer();
		~CPodcastPlayContainer();
		void ConstructL( const TRect& aRect );
	protected:
		CAknNavigationDecorator* iNaviDecorator;
        CAknNavigationControlContainer* iNaviPane;
	};

CPodcastPlayContainer::CPodcastPlayContainer()
{
}

void CPodcastPlayContainer::ConstructL( const TRect& aRect )
{
	CreateWindowL();

	 // Set the windows size
    SetRect( aRect );    
    MakeVisible(EFalse);
    // Activate the window, which makes it ready to be drawn
    ActivateL();   
}

CPodcastPlayContainer::~CPodcastPlayContainer()
{
	delete iNaviDecorator;
}


CPodcastPlayView* CPodcastPlayView::NewL()
    {
    CPodcastPlayView* self = CPodcastPlayView::NewLC();
    CleanupStack::Pop( self );
    return self;
    }

CPodcastPlayView* CPodcastPlayView::NewLC()
    {
    CPodcastPlayView* self = new ( ELeave ) CPodcastPlayView();
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

CPodcastPlayView::CPodcastPlayView()
{
}

void CPodcastPlayView::ConstructL()
{
	BaseConstructL(R_PODCAST_PLAYVIEW);	
	iPlayContainer = new (ELeave) CPodcastPlayContainer;
	iPlayContainer->ConstructL(ClientRect());
}
    
CPodcastPlayView::~CPodcastPlayView()
    {
    delete iPlayContainer;    
    }

TUid CPodcastPlayView::Id() const
{
	return TUid::Uid(4);
}
		
void CPodcastPlayView::DoActivateL(const TVwsViewId& aPrevViewId,
	                                  TUid aCustomMessageId,
	                                  const TDesC8& aCustomMessage)
{
}

void CPodcastPlayView::DoDeactivate()
{
}