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

#ifndef FEEDENGINE_H_
#define FEEDENGINE_H_

#include "HttpClientObserver.h"
#include "HttpClient.h"
#include "FeedParser.h"
#include "FeedInfo.h"
#include "ShowInfo.h"
#include <e32cmn.h>
#include "Constants.h"
#include "FeedEngineObserver.h"
#include "FeedTimer.h"
#include "sqlite3.h"


class CPodcastModel;

_LIT(KOpmlFeed, "  <outline text=\"%S\" xmlUrl=\"%S\"/>");
_LIT(KOpmlHeader, "<?xml version=\"1.0\" encoding=\"UTF-16\"?>\n<opml version=\"1.1\"><head>\n  <title>Podcasting Feed List</title>\n</head>\n<body>");
_LIT(KOpmlFooter, "</body>\n</opml>");

enum TClientState {
	ENotUpdating,
	EUpdatingFeed,
	EUpdatingImage
};

class CFeedEngine : public CBase, public MHttpClientObserver, public MFeedParserObserver
{
public:
	static CFeedEngine* NewL(CPodcastModel& aPodcastModel);
	virtual ~CFeedEngine();
	
public:
	IMPORT_C TBool AddFeedL(CFeedInfo *item);
	IMPORT_C void ImportFeedsL(const TDesC& aFile);
	IMPORT_C TBool ExportFeedsL(TFileName& aFile);
	IMPORT_C void RemoveFeedL(TUint aUid);
	IMPORT_C TBool UpdateFeedL(TUint aFeedUid);
	IMPORT_C void UpdateAllFeedsL();
	IMPORT_C void CancelUpdateAllFeeds();
	IMPORT_C const RFeedInfoArray& GetSortedFeeds();
	IMPORT_C CFeedInfo* GetFeedInfoByUid(TUint aFeedUid);	
	IMPORT_C void GetStatsByFeed(TUint aFeedUid, TUint &aNumShows, TUint &aNumUnplayed);
	IMPORT_C void GetDownloadedStats(TUint &aNumShows, TUint &aNumUnplayed);

	IMPORT_C void AddObserver(MFeedEngineObserver *observer);
	IMPORT_C void RemoveObserver(MFeedEngineObserver *observer);

	void RunFeedTimer();
	
	static void FileNameFromUrl(const TDesC &aUrl, TFileName &aFileName);
	static void EnsureProperPathName(TFileName &aPath);

	IMPORT_C void UpdateFeed(CFeedInfo *aItem);
	/**
	 * Returns the current internal state of the feed engine4
	 */
	IMPORT_C TClientState ClientState();

	/**
	 * Returns the current updating client UID if clientstate is != ENotUpdateing
	 * @return TUint
	 */
	IMPORT_C TUint ActiveClientUid();
protected:
	
	static TInt CompareFeedsByTitle(const CFeedInfo &a, const CFeedInfo &b);

private:
	void ConstructL();
	CFeedEngine(CPodcastModel& aPodcastModel);

	// from HttpClientObserver
	void Connected(CHttpClient* aClient);
	void Disconnected(CHttpClient* aClient);
	void Progress(CHttpClient* aHttpClient, TInt aBytes, TInt aTotalBytes);
	void DownloadInfo(CHttpClient* aClient, TInt aSize);
	void CompleteL(CHttpClient* aClient, TInt aError);
	void FileError(TUint /*aError*/) { }
	// from FeedParser
	TBool NewShowL(CShowInfo *item);
	void ParsingCompleteL(CFeedInfo *item);

	void GetFeedImageL(CFeedInfo *aFeedInfo);
	static void ReplaceChar(TDes & aString, TUint aCharToReplace, TUint aReplacement);
	static void ReplaceString(TDes & aString, const TDesC& aStringToReplace,const TDesC& aReplacement);
	void CleanHtmlL(TDes &str);
	
	void UpdateNextFeedL();
	void NotifyFeedUpdateComplete();

private:
	void DBLoadFeedsL();
	TBool DBRemoveFeed(TUint aUid);
	TBool DBAddFeedL(CFeedInfo *item);
	CFeedInfo* DBGetFeedInfoByUidL(TUint aFeedUid);	
	TUint DBGetFeedCount();
	TBool DBUpdateFeed(CFeedInfo *aItem);
	void DBGetStatsByFeed(TUint aFeedUid, TUint &aNumShows, TUint &aNumUnplayed);

		
private:
	CHttpClient* iFeedClient;
	TClientState iClientState;
	CFeedTimer iFeedTimer;

	CPodcastModel& iPodcastModel;	
	
	// RSS parser
	CFeedParser* iParser;
	
	// the list of feeds
	RFeedInfoArray iSortedFeeds;

	CFeedInfo *iActiveFeed;
	TFileName iUpdatingFeedFileName;

	RFeedInfoArray iFeedsUpdating;
	
	// observers that will receive callbacks, not owning
    RArray<MFeedEngineObserver*> iObservers;
    
    TBool iCatchupMode;
    TUint iCatchupCounter;
    
    sqlite3& iDB;
    
    TBuf<KDefaultSQLDataBufferLength> iSqlBuffer;
};
#endif /*FEEDENGINE_H_*/

