/**
 * This file is a part of Escarpod Podcast project
 * (c) 2008 The Bergamot Project
 * (c) 2008 Teknolog (Sebastian Brännström)
 * (c) 2008 Anotherguest (Lars Persson)
 */

#include "PodcastSettingsView.h"

#include <aknnavide.h> 
#include <podcast.rsg>

class CPodcastSettingsContainer : public CCoeControl
    {
    public: 
		CPodcastSettingsContainer();
		~CPodcastSettingsContainer();
		void ConstructL( const TRect& aRect );
	protected:
		CAknNavigationDecorator* iNaviDecorator;
        CAknNavigationControlContainer* iNaviPane;
	};

CPodcastSettingsContainer::CPodcastSettingsContainer()
{
}

void CPodcastSettingsContainer::ConstructL( const TRect& aRect )
{
	CreateWindowL();

	 // Set the windows size
    SetRect( aRect );    
    MakeVisible(EFalse);
    // Activate the window, which makes it ready to be drawn
    ActivateL();   
}

CPodcastSettingsContainer::~CPodcastSettingsContainer()
{
	delete iNaviDecorator;
}


CPodcastSettingsView* CPodcastSettingsView::NewL()
    {
    CPodcastSettingsView* self = CPodcastSettingsView::NewLC();
    CleanupStack::Pop( self );
    return self;
    }

CPodcastSettingsView* CPodcastSettingsView::NewLC()
    {
    CPodcastSettingsView* self = new ( ELeave ) CPodcastSettingsView();
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

CPodcastSettingsView::CPodcastSettingsView()
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
	return TUid::Uid(5);
}
		
void CPodcastSettingsView::DoActivateL(const TVwsViewId& aPrevViewId,
	                                  TUid aCustomMessageId,
	                                  const TDesC8& aCustomMessage)
{
}

void CPodcastSettingsView::DoDeactivate()
{
}