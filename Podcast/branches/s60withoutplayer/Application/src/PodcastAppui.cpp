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
#include "ShowEngine.h"
#include "PodcastModel.h"
#include "debug.h"
#include "..\help\podcasting.hlp.hrh"
#include "PodcastApp.h"

#include <HLPLCH.H>
#include <avkon.hrh>

CPodcastAppUi::CPodcastAppUi(CPodcastModel* aPodcastModel):iPodcastModel(aPodcastModel)
	{
	
	}
void CPodcastAppUi::ConstructL()
    {
    DP("CPodcastAppUi::ConstructL() BEGIN");
    BaseConstructL(CAknAppUi::EAknEnableSkin); 
    
//    iMainView = CPodcastMainView::NewL(*iPodcastModel);
//	this->AddViewL(iMainView);

	iFeedView = CPodcastFeedView::NewL(*iPodcastModel);
	this->AddViewL(iFeedView);

	iShowsView = CPodcastShowsView::NewL(*iPodcastModel);
	this->AddViewL(iShowsView);

	iSettingsView = CPodcastSettingsView::NewL(*iPodcastModel);
	this->AddViewL(iSettingsView);
	
	iNaviPane =( CAknNavigationControlContainer * ) StatusPane()->ControlL( TUid::Uid( EEikStatusPaneUidNavi ) );
	NaviShowTabGroupL();
    DP("CPodcastAppUi::ConstructL() END");
    }

CPodcastAppUi::~CPodcastAppUi()
    {
	delete iNaviDecorator;
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
 
	TUid KUidHelpExampleApp; // this is the uid of your .hlp file.
 
    CArrayFixFlat<TCoeHelpContext>* array = 
                new(ELeave)CArrayFixFlat<TCoeHelpContext>(1);
    CleanupStack::PushL(array);
	array->AppendL(TCoeHelpContext(KUidPodcast,KContextApplication));
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
	//HBufC *label3 = iEikonEnv->AllocReadResourceLC(R_TABGROUP_DOWNLOADED);			
			
	iTabGroup->AddTabL(0,*label1);
	iTabGroup->AddTabL(1,*label2);
	//iTabGroup->AddTabL(2,*label3);
	
	//CleanupStack::PopAndDestroy(label3);
	CleanupStack::PopAndDestroy(label2);
	CleanupStack::PopAndDestroy(label1);
	
	iTabGroup->SetActiveTabByIndex(0);
	iTabGroup->SetObserver(this);

	iNaviPane->Pop();
	iNaviPane->PushL(*iNaviDecorator);

	}

void CPodcastAppUi::TabChangedL (TInt /*aIndex*/)
	{
	DP("CPodcastListView::TabChangedL ");
	TInt index = iTabGroup->ActiveTabIndex();
	
	TUid newview = TUid::Uid(0);
	TUid messageUid = TUid::Uid(0);
	
	if (index == 0) {
		newview = KUidPodcastFeedViewID;
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
		ActivateLocalViewL(newview,  messageUid, KNullDesC8());
		}
	}

void CPodcastAppUi::SetActiveTab(TInt aIndex) {
	iTabGroup->SetActiveTabByIndex(aIndex);
}
