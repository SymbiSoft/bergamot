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
	static CPodcastModel* NewL();
	~CPodcastModel();
	CFeedEngine& FeedEngine();
	CShowEngine& ShowEngine();
	CSoundEngine& SoundEngine();
	CSettingsEngine& SettingsEngine();
	CShowInfo* PlayingPodcast();
	void SetPlayingPodcast(CShowInfo* aPodcast);
	void PlayPausePodcastL(CShowInfo * aPodcast, TBool aPlayOnInit = EFalse);
	CFeedInfo* ActiveFeedInfo();
	void SetActiveFeedInfo(CFeedInfo* aFeedInfo);
	CEikonEnv* EikonEnv();
	RShowInfoArray& ActiveShowList();
	void SetActiveShowList(RShowInfoArray& aShowArray);

	TBool SetZoomState(TInt aZoomState);
	TInt ZoomState();

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
	
	 sqlite3* DB();
	
	
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

