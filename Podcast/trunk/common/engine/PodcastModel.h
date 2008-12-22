#ifndef PODCASTMODEL_H
#define PODCASTMODEL_H
#include <e32base.h>
#include <eikenv.h>
#include <commdb.h>
#include <CommDbConnPref.h>
#include <es_sock.h>
#include <http/rhttpsession.h>
#include "ShowEngine.h"
#include "FeedInfo.h"
#include "ShowInfo.h"
#include "TelephonyListener.h"
#include "RemoteControlListener.h"
#include "debug.h"
#include "sqlite3.h"


class CFeedEngine;
class CSoundEngine;
//class CShowEngine;
class CSettingsEngine;
class CCommsDatabase;

class TPodcastIAPItem
{
public:
	TUint32 iIapId;
	TBuf<KCommsDbSvrMaxFieldLength> iServiceType;
	TBuf<KCommsDbSvrMaxFieldLength> iBearerType;
};

/**
 * This class handles application storage needs and ownership of audioplayer, resource lists etc.
 */
class CPodcastModel : public CBase
{
public:
	static CPodcastModel* NewL();
	~CPodcastModel();
	
	// Infrastructure
	CFeedEngine& FeedEngine() { return *iFeedEngine; };
	CShowEngine& ShowEngine() { return *iShowEngine; };
	CSoundEngine& SoundEngine();
	CSettingsEngine& SettingsEngine();
	sqlite3* DB();
	CEikonEnv* EikonEnv();

	// Zoom settings
	TBool SetZoomState(TInt aZoomState);
	TInt ZoomState();

	// IAP and connection management
	void UpdateIAPListL();
	CDesCArrayFlat* IAPNames();
	RArray<TPodcastIAPItem>& IAPIds();
	void SetIap(TInt aIap);
	RConnection& Connection();
	TConnPref& ConnPref();
	TBool ConnectHttpSessionL(RHTTPSession& aSession);
	void SetProxyUsageIfNeeded(RHTTPSession& aSession);
	void GetProxyInformationForConnectionL(TBool& aIsUsed, HBufC*& aServerName, TUint32& aPort);
	TInt GetIapId();
	
	// Show access methods
	void GetAllShows();
	void GetNewShows();
	void GetShowsDownloaded();
	void GetShowsDownloading();
	TUint GetShowsDownloadingCount();
	void GetShowsByFeed(TUint aFeedUid);
	void MarkSelectionPlayed();
	RShowInfoArray& ActiveShowList();
	
	
	void ResumeDownloads();
	void StopDownloads();
	void AddShowEngineObserver(MShowEngineObserver *aObserver);
		
	// Feed access methods
	void GetFeedList();
	void GetBookList();
	RFeedInfoArray& ActiveFeedList();
	CFeedInfo* ActiveFeedInfo();

	void SetActiveFeedUid(TUint aFeedUid);
	TInt FindActiveShowByUid(TUint aUid);
	
	CShowInfo* PlayingPodcast();
	TUint PlayingShowUid();
	void SetPlayingShowUid(TUint aShowUid);
	void PlayPausePodcastL(TUint aShowUid, TBool aPlayOnInit = EFalse);

	void RemoveFeed(TUint aUid);
protected:
	CPodcastModel();
	void ConstructL();
	
private:	   
	
   // infrastructure
   CFeedEngine* iFeedEngine;
   CShowEngine* iShowEngine;
   CSoundEngine* iSoundEngine;
   CSettingsEngine *iSettingsEngine;
   sqlite3* iDB;
   CEikonEnv* iEnv;
   
   // active data sets
   RShowInfoArray iActiveShowList;
   RFeedInfoArray iActiveFeedList;

   // UIDs to active and playing shows
   TUint iActiveFeedUid;
   TUint iPlayingShowUid;

   TInt iZoomState;
   
   // connection
   RArray<TPodcastIAPItem> iIapIdArray;
   CDesCArrayFlat* iIapNameArray;
   CCommsDatabase* iCommDB;
   RSocketServ iSocketServ;
   RConnection iConnection;
   TCommDbConnPref iConnPref;
   
   CTelephonyListener *iTelephonyListener;
   CRemoteControlListener *iRemConListener;
};

#endif // PODCASTMODEL_H

