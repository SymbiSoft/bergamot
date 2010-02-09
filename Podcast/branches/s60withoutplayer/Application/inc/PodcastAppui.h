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
#include <akntabobserver.h>
#include <akntabgrp.h>
#include <aknnavide.h> 

class CPodcastMainView;
class CPodcastFeedView;
class CPodcastShowsView;
class CPodcastSettingsView;
class CPodcastSearchView;
class CPodcastModel;

const TUid KUidPodcastClientID          = {0xA00046AE};
const TUid KUidPodcastBaseViewID = {0x00000001};
const TUid KUidPodcastFeedViewID = {0x00000002};
const TUid KUidPodcastShowsViewID = {0x00000003};
const TUid KUidPodcastSettingsViewID = {0x00000005};
const TUid KUidPodcastSearchViewID = {0x00000006};

const TUint KTabIdFeeds = 0;
const TUint KTabIdShows = 1;
const TUint KTabIdQueue = 2;
const TUint KTabIdSearch = 3;

const TInt KTimeoutPrio = CActive::EPriorityStandard;

class CTimeout;

class MTimeoutObserver
{
public:
    virtual void HandleTimeout(const CTimeout& aId, TInt aError)=0;
protected:
    virtual ~MTimeoutObserver() {}
};

class CTimeout : public CTimer
{
public:
	
     static CTimeout* NewLC(MTimeoutObserver& aObserver, TInt aPrio=KTimeoutPrio)
    	 {
         CTimeout* self=new(ELeave) CTimeout(aObserver, aPrio);
         CleanupStack::PushL(self);
         self->ConstructL();
         return self;
    	 }
     
     static CTimeout* NewL(MTimeoutObserver& aObserver, TInt aPrio=KTimeoutPrio)
		{
		CTimeout* self=NewLC(aObserver, aPrio);
		CleanupStack::Pop(self);
		return self;
		}
     
     ~CTimeout()
    	 {
    	 Cancel();
    	 }
protected:
    CTimeout(MTimeoutObserver& aObserver, TInt aPrio) : CTimer(aPrio), iObserver(aObserver)
    {
        CActiveScheduler::Add(this);
    }
    void ConstructL()
    	{
    	    CTimer::ConstructL();
    	}
    
    void RunL()
    	{
    	    TInt r=iStatus.Int();
    	    iObserver.HandleTimeout(*this, r);
    	}

protected:
    MTimeoutObserver& iObserver;
};

class CPodcastAppUi : public CAknViewAppUi, public MAknTabObserver, MTimeoutObserver
    {
    public: 
    	CPodcastAppUi(CPodcastModel* aPodcastModel);
        void ConstructL();
       ~CPodcastAppUi();

       void SetActiveTab(TInt aIndex);
       void UpdateQueueTab(TInt aQueueLength);
       void TabLeft();
       void TabRight();
    private:
        // From MEikMenuObserver
        void DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane);
        void TabChangedL (TInt aIndex);
        CArrayFix<TCoeHelpContext>* HelpContextL() const;
    private:
        void HandleCommandL(TInt aCommand);
        void NaviShowTabGroupL();
    protected:
    	void HandleTimeout(const CTimeout& aId, TInt aError);

    private:
    	CPodcastMainView *iMainView;
		CPodcastFeedView* iFeedView;
		CPodcastShowsView* iShowsView;		
		CPodcastSearchView* iSearchView;
		CPodcastSettingsView* iSettingsView;
		CPodcastModel* iPodcastModel;

		CAknNavigationDecorator* iNaviDecorator;
		CAknTabGroup* iTabGroup;
		CAknNavigationControlContainer* iNaviPane;
		
		CTimeout* iStartTimer;
    };

#endif
