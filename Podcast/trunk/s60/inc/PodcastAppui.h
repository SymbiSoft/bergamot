#ifndef PODCASTAPPUI_H
#define PODCASTAPPUI_H

#include <aknviewappui.h>

#include "PodcastContainer.h"
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
const TUid KUidPodcastPlayViewID = {0x00000004};
const TUid KUidPodcastSettingsViewID = {0x00000005};

class CPodcastAppUi : public CAknViewAppUi
    {
    public: 
        void ConstructL();
       ~CPodcastAppUi();
        void DBTest();
    private:
        // From MEikMenuObserver
        void DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane);

    private:
        void HandleCommandL(TInt aCommand);
    private:
    	CPodcastMainView *iMainView;
		CPodcastFeedView* iFeedView;
		CPodcastShowsView* iShowsView;
		CPodcastPlayView*  iPlayView;
		CPodcastSettingsView* iSettingsView;
		CPodcastModel* iPodcastModel;
    };

#endif
