/**
 * This file is a part of Escarpod Podcast project
 * (c) 2008 The Bergamot Project
 * (c) 2008 Teknolog (Sebastian Brännström)
 * (c) 2008 Anotherguest (Lars Persson)
 */

#include "PodcastMainView.h"

#include <aknnavide.h> 
#include <podcast.rsg>
#include <podcast.mbg>
#include <gulicon.h>
#include <eikenv.h>

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
	CPodcastListView::ConstructL();	// Create icon array with granularity of 1 icon
	
	CArrayPtr< CGulIcon >* icons = new(ELeave) CArrayPtrFlat< CGulIcon >(1);
	
	CleanupStack::PushL( icons );

	// Load the bitmap for play icon	
	CFbsBitmap* bitmap = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastPlay_40x40);
	CleanupStack::PushL( bitmap );		
	// Load the mask for play icon	
	CFbsBitmap* mask = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastPlay_40x40m );	
	CleanupStack::PushL( mask );
	// Append the play icon to icon array
	icons->AppendL( CGulIcon::NewL( bitmap, mask ) );
	CleanupStack::Pop(2); // bitmap, mask


	// Load the bitmap for shows icon	
	bitmap = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastDownloaded_view_40x40);
	CleanupStack::PushL( bitmap );		
	// Load the mask for shows icon	
	mask = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastDownloaded_view_40x40m );	
	CleanupStack::PushL( mask );
	// Append the shows icon to icon array
	icons->AppendL( CGulIcon::NewL( bitmap, mask ) );
	CleanupStack::Pop(2); // bitmap, mask


	// Load the bitmap for download icon	
	bitmap = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastPending_view_40x40);
	CleanupStack::PushL( bitmap );		
	// Load the mask for download icon	
	mask = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastPending_view_40x40m );	
	CleanupStack::PushL( mask );
	// Append the download icon to icon array
	icons->AppendL( CGulIcon::NewL( bitmap, mask ) );
	CleanupStack::Pop(2); // bitmap, mask


	// Load the bitmap for Feeds icon	
	bitmap = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastFeeds_40x40);
	CleanupStack::PushL( bitmap );		
	// Load the mask for Feeds icon	
	mask = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastFeeds_40x40m );	
	CleanupStack::PushL( mask );
	// Append the Feeds icon to icon array
	icons->AppendL( CGulIcon::NewL( bitmap, mask ) );
	CleanupStack::Pop(2); // bitmap, mask


	// Load the bitmap for audiobooks icon	
	bitmap = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastAudiobooks_40x40);
	CleanupStack::PushL( bitmap );		
	// Load the mask for audiobooks icon	
	mask = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastAudiobooks_40x40m );	
	CleanupStack::PushL( mask );
	// Append the audiobooks icon to icon array
	icons->AppendL( CGulIcon::NewL( bitmap, mask ) );
	CleanupStack::Pop(2); // bitmap, mask

	iListContainer->Listbox()->ItemDrawer()->FormattedCellData()->SetIconArrayL( icons );
	CleanupStack::Pop(icons); // icons

	CDesCArray* items = iEikonEnv->ReadDesCArrayResourceL(R_PODCAST_MAINMENU_ARRAY);
	iListContainer->Listbox()->Model()->SetItemTextArray(items);
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