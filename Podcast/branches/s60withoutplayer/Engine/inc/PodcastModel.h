#ifndef PODCASTMODEL_H
#define PODCASTMODEL_H
#include <e32base.h>
#include <eikenv.h>
#include <commdb.h>
#include <CommDbConnPref.h>
#include <es_sock.h>
#include <http/rhttpsession.h>
#include "FeedInfo.h"
#include "ShowInfo.h"
#include "TelephonyListener.h"
#include "RemoteControlListener.h"
#include "debug.h"
#include "sqlite3.h"

// SQLite leaks memory when sorting, so to test our own memory leaks we need to disable this
//#define DONT_SORT_SQL

class CFeedEngine;
class CSoundEngine;
class CShowEngine;
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
	IMPORT_C static CPodcastModel* NewL();
	IMPORT_C ~CPodcastModel();
	IMPORT_C CFeedEngine& FeedEngine();
	IMPORT_C CShowEngine& ShowEngine();
	IMPORT_C CSoundEngine& SoundEngine();
	IMPORT_C CSettingsEngine& SettingsEngine();
	IMPORT_C CShowInfo* PlayingPodcast();
	IMPORT_C void SetPlayingPodcast(CShowInfo* aPodcast);
	IMPORT_C void PlayPausePodcastL(CShowInfo * aPodcast, TBool aPlayOnInit = EFalse);
	IMPORT_C CFeedInfo* ActiveFeedInfo();
	IMPORT_C void SetActiveFeedInfo(CFeedInfo* aFeedInfo);
	
	CEikonEnv* EikonEnv();
	
	IMPORT_C RShowInfoArray& ActiveShowList();
	
	void SetActiveShowList(RShowInfoArray& aShowArray);

	TBool SetZoomState(TInt aZoomState);
	TInt ZoomState();

	void UpdateIAPListL();
	IMPORT_C CDesCArrayFlat* IAPNames();
	IMPORT_C RArray<TPodcastIAPItem>& IAPIds();
	
	IMPORT_C void SetIap(TInt aIap);
	RConnection& Connection();
	TConnPref& ConnPref();
	TBool ConnectHttpSessionL(RHTTPSession& aSession);
	
	void SetProxyUsageIfNeeded(RHTTPSession& aSession);
	void GetProxyInformationForConnectionL(TBool& aIsUsed, HBufC*& aServerName, TUint32& aPort);
	TInt GetIapId();
	
	sqlite3* DB();
	
	IMPORT_C void GetAllShows();
	IMPORT_C void GetNewShows();
	IMPORT_C void GetShowsDownloaded();
	IMPORT_C void GetShowsDownloading();
	IMPORT_C void GetShowsByFeed(TUint aFeedUid);
	IMPORT_C void MarkSelectionPlayed();
	
	TInt FindActiveShowByUid(TUint aUid);
	
protected:
	CPodcastModel();
	void ConstructL();
private:	
   CShowInfo* iPlayingPodcast;
   
   CFeedEngine* iFeedEngine;
   CShowEngine* iShowEngine;
   CSoundEngine* iSoundEngine;
   CSettingsEngine *iSettingsEngine;
   
   RShowInfoArray iActiveShowList;
   CFeedInfo *iActiveFeed;
   TInt iZoomState;
   
   CEikonEnv* iEnv;
   RArray<TPodcastIAPItem> iIapIdArray;
   CDesCArrayFlat* iIapNameArray;
   CCommsDatabase* iCommDB;
   
   RSocketServ iSocketServ;

   RConnection iConnection;
   TCommDbConnPref iConnPref;
   
   CTelephonyListener *iTelephonyListener;
   CRemoteControlListener *iRemConListener;
   
   sqlite3* iDB;
};

#endif // PODCASTMODEL_H
