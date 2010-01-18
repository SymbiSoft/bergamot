/*
* Copyright (c) 2007-2010 Sebastian Brannstrom, Lars Persson, EmbedDev AB
*
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of the License "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* EmbedDev AB - initial contribution.
*
* Contributors:
*
* Description:
*
*/

#include "PodcastListView.h"
#include "Podcast.hrh"
#include "PodcastAppUi.h"
#include <aknnavide.h> 
#include <podcast.rsg>
#include <aknlists.h> 
#include <aknviewappui.h>
#include <aknnotedialog.h>
#include <aknsbasicbackgroundcontrolcontext.h>
#include <akntabgrp.h>

const TInt KDefaultGran = 5;
CPodcastListContainer::CPodcastListContainer()
{
}

void CPodcastListContainer::SetKeyEventListener(MKeyEventListener *aKeyEventListener)
	{
	iKeyEventListener = aKeyEventListener;
	}

TKeyResponse CPodcastListContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
{
	if (iKeyEventListener)
		iKeyEventListener->OfferKeyEventL(aKeyEvent, aType);
	return iListbox->OfferKeyEventL(aKeyEvent, aType);
}

void CPodcastListContainer::ConstructL( const TRect& aRect, TInt aListboxFlags )
{
	CreateWindowL();

	iBgContext = 
		    CAknsBasicBackgroundControlContext::NewL( KAknsIIDQsnBgAreaMain, 
		                                              aRect, 
		                                              ETrue );
		
	 // Set the windows size
    SetRect( aRect );    
    iListbox =static_cast<CEikFormattedCellListBox*>( new (ELeave) CAknDoubleLargeStyleListBox);
    iListbox->ConstructL(this, aListboxFlags);
    iListbox->SetMopParent( this );
	iListbox->SetContainerWindowL(*this);
	iListbox->CreateScrollBarFrameL(ETrue);
	iListbox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EAuto, CEikScrollBarFrame::EAuto );
	
	iListbox->ItemDrawer()->FormattedCellData()->EnableMarqueeL( ETrue );

	iListbox->SetSize(aRect.Size());
	iListbox->MakeVisible(ETrue);
    MakeVisible(EFalse);
	// Activate the window, which makes it ready to be drawn
    ActivateL();   
}

TInt CPodcastListContainer::CountComponentControls() const
    {
    return 1; // return number of controls inside this container
    }

CCoeControl* CPodcastListContainer::ComponentControl(TInt aIndex) const
    {
    switch ( aIndex )
        {
        case 0:
            return iListbox;
        default:
            return NULL;
        }
    }

void CPodcastListContainer::HandleResourceChange(TInt aType)
{
	switch( aType )
    	{
	    case KEikDynamicLayoutVariantSwitch:
		    SetRect(iEikonEnv->EikAppUi()->ClientRect());
		    break;
	    }
}

void CPodcastListContainer::ScrollToVisible() {
	if (iListbox != NULL) {
		iListbox->ScrollToMakeItemVisible(iListbox->CurrentItemIndex());
	}
}
void CPodcastListContainer::SizeChanged()
{
	DP2("CPodcastListContainer::SizeChanged(), width=%d, height=%d",Size().iWidth, Size().iHeight);
	if(iListbox != NULL)
	{
		iListbox->SetSize(Size());
	}
}

CEikFormattedCellListBox* CPodcastListContainer::Listbox()
{
	return iListbox;
}


CPodcastListContainer::~CPodcastListContainer()
{
	delete iListbox;
	delete iBgContext;
}


void CPodcastListContainer::Draw(const TRect& aRect) const
	{
	CWindowGc& gc = SystemGc();
	gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
	MAknsControlContext* cc = AknsDrawUtils::ControlContext(this);
	MAknsSkinInstance* skin = AknsUtils::SkinInstance();
	AknsDrawUtils::Background(skin, cc, (CCoeControl*) this, gc, aRect);
	}


TTypeUid::Ptr CPodcastListContainer::MopSupplyObject( TTypeUid aId )
    {
    if (iBgContext )
    {
      return MAknsControlContext::SupplyMopObject( aId, iBgContext );
    }
    return CCoeControl::MopSupplyObject(aId);
    }

CPodcastListView::CPodcastListView()
{
}

void CPodcastListView::ConstructL()
{
	iListContainer = new (ELeave) CPodcastListContainer;
	iListContainer->ConstructL(ClientRect(), iListboxFlags);
	iListContainer->SetMopParent(this);
	iListContainer->ActivateL();
	iItemArray = new (ELeave)CDesCArrayFlat(KDefaultGran);
	iListContainer->Listbox()->Model()->SetItemTextArray(iItemArray);
	iListContainer->Listbox()->Model()->SetOwnershipType(ELbmDoesNotOwnItemArray);
	iNaviPane =( CAknNavigationControlContainer * ) StatusPane()->ControlL( TUid::Uid( EEikStatusPaneUidNavi ) );
}

void CPodcastListView::HandleViewRectChange()
{
    if ( iListContainer )
	{
        iListContainer->SetRect( ClientRect() );
	}
}

void CPodcastListView::HandleStatusPaneSizeChange()
{
	DP2("CPodcastListView::HandleStatusPaneSizeChange(), width=%d, height=%d", ClientRect().Width(), ClientRect().Height());

	if ( iListContainer )
	{
        iListContainer->SetRect( ClientRect() );
	}
	
}

    
CPodcastListView::~CPodcastListView()
    {
    delete iListContainer;  
    delete iNaviDecorator;
    delete iItemArray;
    iItemIdArray.Close();
    }

	
void CPodcastListView::DoActivateL(const TVwsViewId& /*aPrevViewId */,
	                                  TUid /*aCustomMessageId */,
									  const TDesC8& /* aCustomMessage */)
{	
	DP("CPodcastListView::DoActivateL()");
	
	if(iListContainer)
	{
		iListContainer->SetSize(ClientRect().Size());
		iListContainer->SetMopParent(this);
		
		AppUi()->AddToStackL(*this, iListContainer);
		iListContainer->MakeVisible(ETrue);
		UpdateListboxItemsL();		
		iListContainer->DrawNow();
	}
	
	NaviShowTabGroupL();
	/*if(iNaviDecorator && iNaviPane)
		{
		iNaviPane->PushL(*iNaviDecorator);
		}*/
}

void CPodcastListView::DoDeactivate()
{
	DP("CPodcastListView::DoDeactivate()");
	if ( iListContainer )
	{
		AppUi()->RemoveFromViewStack( *this, iListContainer);
		iListContainer->MakeVisible(EFalse);
		if(iNaviDecorator && iNaviPane)
		{
		iNaviPane->Pop(iNaviDecorator);
		}
	}
}

/** 
* Command handling function intended for overriding by sub classes. 
* Default implementation is empty.  
* @param aCommand ID of the command to respond to. 
*/
void CPodcastListView::HandleCommandL(TInt aCommand)
{
	DP1("CPodcastListView::HandleCommandL=%d", aCommand);
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
	case EPodcastSettings:
		AppUi()->ActivateLocalViewL(KUidPodcastSettingsViewID);
		break;
	case EPodcastZoomSetting:
		break;
	case EPodcastAbout:
		RunAboutDialogL();
		break;
	default:
		break;
	}
}


void CPodcastListView::RunAboutDialogL()
{
	CAknNoteDialog* dlg = new(ELeave) CAknNoteDialog();
	CleanupStack::PushL(dlg);
	
	/*TBuf<255> aboutMessage;
	iEikonEnv->ReadResourceL(aboutMessage, STRING_R_ABOUT_TEXT_S60);
	dlg->SetTextL(aboutMessage);*/
	
	CleanupStack::Pop(dlg);
	dlg->ExecuteLD(R_DLG_ABOUT);
}

void CPodcastListView::SetNaviTextL(TDesC &aText)
	{
	if (iNaviPane != NULL)
		{
		iNaviPane->Pop(iNaviDecorator);
		delete iNaviDecorator;
		iNaviDecorator = NULL;
		}

	iNaviDecorator	= iNaviPane->CreateNavigationLabelL(aText);
	iNaviPane->PushL(*iNaviDecorator);
	}

void CPodcastListView::NaviShowTabGroupL()
	{
	if (iNaviDecorator == NULL) {
		iNaviDecorator = iNaviPane->CreateTabGroupL();
		iTabGroup = STATIC_CAST(CAknTabGroup*, iNaviDecorator->DecoratedControl());
		iTabGroup->SetTabFixedWidthL(KTabWidthWithTwoTabs);

		HBufC *label1 = iEikonEnv->AllocReadResourceLC(R_TABGROUP_FEEDS);
		HBufC *label2 = iEikonEnv->AllocReadResourceLC(R_TABGROUP_QUEUE);
		HBufC *label3 = iEikonEnv->AllocReadResourceLC(R_TABGROUP_DOWNLOADED);
				
				
		iTabGroup->AddTabL(0,*label1);
		iTabGroup->AddTabL(1,*label2);
		iTabGroup->AddTabL(1,*label3);
		
		CleanupStack::PopAndDestroy(label3);
		CleanupStack::PopAndDestroy(label2);
		CleanupStack::PopAndDestroy(label1);
		
		iTabGroup->SetActiveTabByIndex(0);
		iTabGroup->SetObserver(this);
	}

	iNaviPane->Pop();
	iNaviPane->PushL(*iNaviDecorator);
	}

void CPodcastListView::TabChangedL (TInt aIndex)
	{
	DP("CPodcastListView::TabChangedL ");
	TInt index = iTabGroup->ActiveTabIndex();
	
	TUid newview = TUid::Uid(0);
	TUid messageUid = TUid::Uid(0);
	
	if (index == 0) {
		newview = KUidPodcastFeedViewID;
		messageUid = TUid::Uid(EFeedsNormalMode);
	} else if (index == 1) {
		newview = KUidPodcastShowsViewID;
		messageUid = TUid::Uid(EShowPendingShows);
	} else if (index == 2) {
		newview = KUidPodcastShowsViewID;
		messageUid = TUid::Uid(EShowDownloadedShows);	
	} else {
		User::Leave(KErrTooBig);
	}
	
	if(newview.iUid != 0)
		{			
			AppUi()->ActivateLocalViewL(newview,  messageUid, KNullDesC8());
		}
	}
