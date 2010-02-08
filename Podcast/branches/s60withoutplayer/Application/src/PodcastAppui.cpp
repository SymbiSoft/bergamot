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

#include "PodcastAppui.h"
#include <Podcast.rsg>
#include "Podcast.hrh"
#include "PodcastFeedView.h"
#include "PodcastShowsView.h"
#include "PodcastSettingsView.h"
#include "PodcastSearchView.h"
#include "ShowEngine.h"
#include "PodcastModel.h"
#include "debug.h"
#include "..\help\podcatcher.hlp.hrh"
#include "PodcastApp.h"

#include <HLPLCH.H>
#include <avkon.hrh>

const TUint KDelayLaunch = 1000;

CPodcastAppUi::CPodcastAppUi(CPodcastModel* aPodcastModel):iPodcastModel(aPodcastModel)
	{
	
	}

void CPodcastAppUi::ConstructL()
    {
    DP("CPodcastAppUi::ConstructL() BEGIN");
    BaseConstructL(CAknAppUi::EAknEnableSkin); 

	iFeedView = CPodcastFeedView::NewL(*iPodcastModel);
	this->AddViewL(iFeedView);

	iShowsView = CPodcastShowsView::NewL(*iPodcastModel);
	this->AddViewL(iShowsView);

	iSearchView = CPodcastSearchView::NewL(*iPodcastModel);
	this->AddViewL(iSearchView);
	
	iSettingsView = CPodcastSettingsView::NewL(*iPodcastModel);
	this->AddViewL(iSettingsView);
	
	iNaviPane =( CAknNavigationControlContainer * ) StatusPane()->ControlL( TUid::Uid( EEikStatusPaneUidNavi ) );
	NaviShowTabGroupL();
	
	// start a timer to let ConstructL finish before we display any dialogs or start downloading
	// this provides another entry point in HandleTimeout below
	iStartTimer = CTimeout::NewL(*this);
	iStartTimer->After(KDelayLaunch);
    DP("CPodcastAppUi::ConstructL() END");
    }

CPodcastAppUi::~CPodcastAppUi()
    {
    iNaviPane->Pop(iNaviDecorator);
	delete iNaviDecorator;
	delete iStartTimer;
    }

// -----------------------------------------------------------------------------
// CPodcastAppUi::DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane)
//  This function is called by the EIKON framework just before it displays
//  a menu pane. Its default implementation is empty, and by overriding it,
//  the application can set the state of menu items dynamically according
//  to the state of application data.
// ------------------------------------------------------------------------------
//
void CPodcastAppUi::DynInitMenuPaneL(
    TInt /*aResourceId*/,CEikMenuPane* /*aMenuPane*/)
    {
    // no implementation required 
    }

// -----------------------------------------------------------------------------
// CPodcastAppUi::HandleCommandL(TInt aCommand)
// takes care of command handling
// -----------------------------------------------------------------------------
//
void CPodcastAppUi::HandleCommandL( TInt aCommand )
    {
    switch ( aCommand )
        {
        case EAknSoftkeyExit:
            {
            Exit();
            break;
            }
        case EEikCmdExit:
        	{
			TApaTask task(CEikonEnv::Static()->WsSession());
			task.SetWgId(CEikonEnv::Static()->RootWin().Identifier());
			task.SendToBackground(); 
			break;
        	}
	case EPodcastHelp:
        	{
        	CArrayFix<TCoeHelpContext>* buf = CPodcastAppUi::AppHelpContextL();		
        	HlpLauncher::LaunchHelpApplicationL(iEikonEnv->WsSession(), buf);
        	}
        	break;      	
        default:
            break;      
        }
    }

CArrayFix<TCoeHelpContext>* CPodcastAppUi::HelpContextL() const
   { 
    CArrayFixFlat<TCoeHelpContext>* array = 
                new(ELeave)CArrayFixFlat<TCoeHelpContext>(1);
    CleanupStack::PushL(array);
    // todo: view detection doesn't seem to work
    if (ViewShown(KUidPodcastSearchViewID)) {
		array->AppendL(TCoeHelpContext(KUidPodcast,KContextSettings));
    } else {
		array->AppendL(TCoeHelpContext(KUidPodcast,KContextApplication));
    }
	
    CleanupStack::Pop(array);
    return array;
	}

void CPodcastAppUi::NaviShowTabGroupL()
	{
	iNaviDecorator = iNaviPane->CreateTabGroupL();
	
	iTabGroup = STATIC_CAST(CAknTabGroup*, iNaviDecorator->DecoratedControl());
	iTabGroup->SetTabFixedWidthL(EAknTabWidthWithTwoTabs);

	HBufC *label1 = iEikonEnv->AllocReadResourceLC(R_TABGROUP_FEEDS);
	HBufC *label2 = iEikonEnv->AllocReadResourceLC(R_TABGROUP_QUEUE);
			
	iTabGroup->AddTabL(0,*label1);
	iTabGroup->AddTabL(1,*label2);
	
	iTabGroup->SetTabFixedWidthL(EAknTabWidthWithThreeTabs);
	HBufC *label3 = iEikonEnv->AllocReadResourceLC(R_TABGROUP_SEARCH);			
	iTabGroup->AddTabL(2,*label3);
	CleanupStack::PopAndDestroy(label3);

	CleanupStack::PopAndDestroy(label2);
	CleanupStack::PopAndDestroy(label1);
	
	iTabGroup->SetActiveTabByIndex(0);
	iTabGroup->SetObserver(this);

	iNaviPane->Pop();
	iNaviPane->PushL(*iNaviDecorator);
	}

void CPodcastAppUi::TabChangedL (TInt aIndex)
	{
	DP("CPodcastListView::TabChangedL ");
	
	TUid newview = TUid::Uid(0);
	TUid messageUid = TUid::Uid(0);
	
	if (aIndex == 0) {
		newview = KUidPodcastFeedViewID;
	} else if (aIndex == 1) {
		newview = KUidPodcastShowsViewID;
		messageUid = TUid::Uid(EShowPendingShows);
	} else if (aIndex == 2) {
		newview = KUidPodcastSearchViewID;
		messageUid = TUid::Uid(0);	
	} else {
		User::Leave(KErrTooBig);
	}
	
	if(newview.iUid != 0)
		{			
		ActivateLocalViewL(newview,  messageUid, KNullDesC8());
		}
	}

void CPodcastAppUi::SetActiveTab(TInt aIndex) {
	iTabGroup->SetActiveTabByIndex(aIndex);
}

void CPodcastAppUi::HandleTimeout(const CTimeout& aId, TInt aError)
	{
	iFeedView->CheckResumeDownload();
	}

void CPodcastAppUi::UpdateQueueTab(TInt aQueueLength)
	{
	if (aQueueLength == 0)
		{
		HBufC *queue = iEikonEnv->AllocReadResourceLC(R_TABGROUP_QUEUE);
		iTabGroup->ReplaceTabL(1, *queue);
		CleanupStack::PopAndDestroy(queue);
		}
	else
		{
		HBufC *queueTemplate = iEikonEnv->AllocReadResourceLC(R_TABGROUP_QUEUE_COUNTER);
		HBufC *queueCounter = HBufC::NewLC(queueTemplate->Length()+2);
		queueCounter->Des().Format(*queueTemplate, aQueueLength);
		
		iTabGroup->ReplaceTabL(1, *queueCounter);
		CleanupStack::PopAndDestroy(queueCounter);
		CleanupStack::PopAndDestroy(queueTemplate);	
		}
	}

void CPodcastAppUi::TabLeft()
	{
	TUint ati = iTabGroup->ActiveTabIndex();
	if(ati > 0) {
		SetActiveTab(ati-1);
		TabChangedL(ati-1);
	}
	}

void CPodcastAppUi::TabRight()
	{
	TUint ati = iTabGroup->ActiveTabIndex();
	
	if(ati < iTabGroup->TabCount()-1) {
		SetActiveTab(ati+1);
		TabChangedL(ati+1);
	}
	}
