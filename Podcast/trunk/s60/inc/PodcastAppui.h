#ifndef PODCASTAPPUI_H
#define PODCASTAPPUI_H

#include <aknviewappui.h>

#include "PodcastContainer.h"
class CPodcastMainView;
class CPodcastFeedView;
class CPodcastShowsView;
class CPodcastPlayView;
class CPodcastSettingsView;

class CPodcastAppUi : public CAknViewAppUi
    {
    public: 
        void ConstructL();
       ~CPodcastAppUi();
        
    private:
        // From MEikMenuObserver
        void DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane);

    private:
        void HandleCommandL(TInt aCommand);
        virtual TKeyResponse HandleKeyEventL(
            const TKeyEvent& aKeyEvent,TEventCode aType);
        virtual void HandleResourceChangeL( TInt aType );            

    private:
    	CPodcastMainView *iMainView;
		CPodcastFeedView* iFeedView;
		CPodcastShowsView* iShowsView;
		CPodcastPlayView*  iPlayView;
		CPodcastSettingsView* iSettingsView;
    };

#endif
