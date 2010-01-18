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

#ifndef PODCASTAPPUI_H
#define PODCASTAPPUI_H

#include <aknviewappui.h>

class CPodcastMainView;
class CPodcastFeedView;
class CPodcastShowsView;
class CPodcastPlayView;
class CPodcastSettingsView;
class CPodcastModel;
const TUid KUidPodcastClientID          = {0xA00046AE};
const TUid KUidPodcastBaseViewID = {0x00000001};
const TUid KUidPodcastFeedViewID = {0x00000002};
const TUid KUidPodcastShowsViewID = {0x00000003};
const TUid KUidPodcastSettingsViewID = {0x00000005};

class CPodcastAppUi : public CAknViewAppUi
    {
    public: 
    	CPodcastAppUi(CPodcastModel* aPodcastModel);
        void ConstructL();
       ~CPodcastAppUi();

       void UpdateAppStatus();

    private:
        // From MEikMenuObserver
        void DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane);

    private:
        void HandleCommandL(TInt aCommand);

    private:
    	CPodcastMainView *iMainView;
		CPodcastFeedView* iFeedView;
		CPodcastShowsView* iShowsView;		
		CPodcastSettingsView* iSettingsView;
		CPodcastModel* iPodcastModel;
    };

#endif
