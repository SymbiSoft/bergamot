#ifndef PODCASTPLAYVIEW_H
#define PODCASTPLAYVIEW_H

#include <qikviewbase.h>
#include <qikscrollablecontainer.h>
#include <coecobs.h>
#include <mqiklistboxobserver.h>
#include <s32file.h>
#include <e32debug.h>
#include <e32cmn.h>
#include <QikImageConverter.h>

#include "PodcastClient.hrh"
#include "PodcastClientGlobals.h"
#include "HttpClient.h"
#include "FeedEngine.h"
#include "SoundEngine.h"
#include "ShowEngineObserver.h"

class CEikLabel;
class CEikEdwin;
class CPodcastModel;
class CQikSlider;
class CEikImage;

class CPodcastClientPlayView : public CQikViewBase, public MSoundEngineObserver, public MFeedEngineObserver, public MQikImageConverterObserver, public MShowEngineObserver
	{
public:
	static CPodcastClientPlayView* NewLC(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel);
	~CPodcastClientPlayView();
	TVwsViewId ViewId()const;
	void HandleCommandL(CQikCommand& aCommand);
 protected: 
	void ViewConstructL();
	void ViewActivatedL(const TVwsViewId &aPrevViewId, TUid aCustomMessageId, const TDesC8 &aCustomMessage);
	void ViewDeactivated();

	void UpdateViewL();
	void PlaybackInitializedL();
	void PlaybackStartedL();
	void PlaybackStoppedL(); 
	static TInt PlayingUpdateStaticCallbackL(TAny* aPlayView);
	void UpdatePlayStatusL();

	void ShowListUpdated(){};
    void FeedInfoUpdated(CFeedInfo* /*aFeedInfo*/){}
	void FeedDownloadUpdatedL(TInt /*aPercentOfCurrentDownload*/){};
	void ShowDownloadUpdatedL(TInt aPercentOfCurrentDownload, TInt aBytesOfCurrentDownload, TInt aBytesTotal);
	void DownloadQueueUpdated(TInt aDownloadingShows, TInt aQueuedShows) {}
	void ImageConverterEventL(TQikImageConverterEvent aMessage, TInt aErrCode);

	TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);
private:
	void HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType);
	CPodcastClientPlayView(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel);
	void ConstructL();
private:
	CPodcastModel& iPodcastModel;
	CQikSlider* iProgress;
	CEikLabel* iTimeLabel;
	CEikEdwin* iInformationEdwin;
	CEikEdwin* iTitleEdwin;
	CQikImageConverter* iBitmapConverter;
	CEikImage* iCoverImageCtrl;
	CFbsBitmap* iCurrentCoverImage;
	CQikSlider* iVolumeSlider;
	CPeriodic* iPlaybackTicker;
	// Use this to change feeds
	CQikCategoryModel* iCategories;
	TBool iProgressAdded;
	TFileName iLastImageFileName;
	CShowInfo* iLastShowInfo;
	TInt iLastZoomLevel;
	};
#endif // PODCASTPLAYVIEW_H
