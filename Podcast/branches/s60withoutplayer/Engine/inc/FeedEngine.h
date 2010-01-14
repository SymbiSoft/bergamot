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
_LIT(KOpmlHeader, "<?xml version=\"1.0\" encoding=\"UTF-16\"?>\n<opml version=\"1.1\"><head>\n  <title>Escarpod Feed List</title>\n</head>\n<body>");
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
	IMPORT_C void RemoveFeed(TUint aUid);
	IMPORT_C TBool UpdateFeedL(TUint aFeedUid);
	IMPORT_C void UpdateAllFeedsL();
	IMPORT_C void CancelUpdateAllFeedsL();
	IMPORT_C const RFeedInfoArray& GetSortedFeeds();
	IMPORT_C CFeedInfo* GetFeedInfoByUid(TUint aFeedUid);	
	IMPORT_C void GetStatsByFeed(TUint aFeedUid, TUint &aNumShows, TUint &aNumUnplayed, TBool aIsBookFeed );
	IMPORT_C void GetDownloadedStats(TUint &aNumShows, TUint &aNumUnplayed);

	IMPORT_C void AddBookL(const TDesC& aBookTitle, CDesCArrayFlat* aFileNameArray);
	IMPORT_C void AddBookChaptersL(CFeedInfo& aFeedInfo, CDesCArrayFlat* aFileNameArray);
	IMPORT_C void RemoveBookL(TUint aUid);
	IMPORT_C const RFeedInfoArray& GetSortedBooks();
	IMPORT_C void ImportBookL(const TDesC& aTitle, const TDesC& aFile);


	IMPORT_C void AddObserver(MFeedEngineObserver *observer);
	IMPORT_C void RemoveObserver(MFeedEngineObserver *observer);

	void RunFeedTimer();
	
	void FileNameFromUrl(const TDesC &aUrl, TFileName &aFileName);
	void EnsureProperPathName(TFileName &aPath);

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
	void CompleteL(CHttpClient* aClient, TBool aSuccessful);
	void FileError(TUint /*aError*/) { }
	// from FeedParser
	TBool NewShowL(CShowInfo *item);
	void ParsingCompleteL(CFeedInfo *item);

	void GetFeedImageL(CFeedInfo *aFeedInfo);
	void ReplaceString(TDes & aString, const TDesC& aStringToReplace,const TDesC& aReplacement);
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
	
	RFs iFs;
	
	// RSS parser
	CFeedParser* iParser;
	
	// the list of feeds
	RFeedInfoArray iSortedFeeds;

	// the list of Books
	RFeedInfoArray iSortedBooks;


	CFeedInfo *iActiveFeed;
	TFileName iUpdatingFeedFileName;

	RFeedInfoArray iFeedsUpdating;
	
	// observers that will receive callbacks
    RArray<MFeedEngineObserver*> iObservers;
    
    TBool iCatchupMode;
    TUint iCatchupCounter;
    
    sqlite3* iDB;
    
    TBuf<2048> iSqlBuffer;
};
#endif /*FEEDENGINE_H_*/

