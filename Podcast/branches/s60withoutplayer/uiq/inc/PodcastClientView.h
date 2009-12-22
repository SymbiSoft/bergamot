#ifndef PODCASTCLIENTVIEW_H
#define PODCASTCLIENTVIEW_H

#include <qikviewbase.h>
#include "s32file.h"
#include <e32cmn.h>
#include <coecobs.h>
#include <mqiklistboxobserver.h>
#include <e32debug.h>
#include <qiklistboxmodel.h>
#include <qiklistbox.h>
#include <qiklistboxdata.h>
#include <qikappui.h>

#include "e32debug.h"
#include "PodcastClientGlobals.h"
#include "HttpClient.h"
#include "FeedEngine.h"
#include "PodcastClient.hrh"

class CPodcastModel;

class CPodcastClientView : public CQikViewBase, public MQikListBoxObserver
	{
public:
	~CPodcastClientView();
	void HandleCommandL(CQikCommand& aCommand);
  
	void HandleControlEventL(CCoeControl *aControl, TCoeEvent aEventType);
protected: 
	void ViewConstructL();
	void ViewActivatedL(const TVwsViewId &aPrevViewId, TUid aCustomMessageId, const TDesC8 &aCustomMessage);
	void ViewDeactivated();
protected:
	CPodcastClientView(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel);
	virtual void ConstructL();
	virtual void UpdateListboxItemsL() = 0;
protected:
	CPodcastModel& iPodcastModel;
	CQikListBox* iListbox;
	TBuf<1024> activeFeedTitle;
	HBufC* iViewLabel;
	HBufC* iAudioBooksLabel;

	// Use this to change feeds
	CQikCategoryModel* iCategories;
	TInt iLastZoomLevel;
	};
#endif
