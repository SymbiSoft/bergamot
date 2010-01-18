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
#include "PodcastMainView.h"
#include "PodcastFeedView.h"
#include "PodcastShowsView.h"
#include "PodcastSettingsView.h"
#include "ShowEngine.h"
#include "PodcastModel.h"
#include "debug.h"
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
	
	UpdateAppStatus();
    DP("CPodcastAppUi::ConstructL() END");
    }

CPodcastAppUi::~CPodcastAppUi()
    {	
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
        default:
            break;      
        }
    }


void CPodcastAppUi::UpdateAppStatus()
{
	TBuf<40> buf;
	if(iPodcastModel->ShowEngine().GetNumDownloadingShowsL() > 0) {
		buf.Copy(_L("Downloading"));
	} else if (iPodcastModel->FeedEngine().ClientState() != ENotUpdating) {
		buf.Copy(_L("Updating"));
	} else {
		buf.Copy(_L("Idle"));
	}
	
	iFeedView->SetNaviTextL(buf);
}
