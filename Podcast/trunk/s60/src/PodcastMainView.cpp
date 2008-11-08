/**
 * This file is a part of Escarpod Podcast project
 * (c) 2008 The Bergamot Project
 * (c) 2008 Teknolog (Sebastian Brännström)
 * (c) 2008 Anotherguest (Lars Persson)
 */

#include "PodcastMainView.h"

#include <aknnavide.h> 
#include <podcast.rsg>

CPodcastMainView* CPodcastMainView::NewL()
    {
    CPodcastMainView* self = CPodcastMainView::NewLC();
    CleanupStack::Pop( self );
    return self;
    }

CPodcastMainView* CPodcastMainView::NewLC()
    {
    CPodcastMainView* self = new ( ELeave ) CPodcastMainView();
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

CPodcastMainView::CPodcastMainView()
{
}

void CPodcastMainView::ConstructL()
{
	BaseConstructL(R_PODCAST_MAINVIEW);
	CPodcastListView::ConstructL();
}
    
CPodcastMainView::~CPodcastMainView()
    {
    }

TUid CPodcastMainView::Id() const
{
	return TUid::Uid(1);
}
		
void CPodcastMainView::DoActivateL(const TVwsViewId& aPrevViewId,
	                                  TUid aCustomMessageId,
	                                  const TDesC8& aCustomMessage)
{
	CPodcastListView::DoActivateL(aPrevViewId, aCustomMessageId, aCustomMessage);
}

void CPodcastMainView::DoDeactivate()
{
	CPodcastListView::DoDeactivate();
}