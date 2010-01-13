#ifndef SHOWINFO_H_
#define SHOWINFO_H_
#include <e32base.h>
#include <e32cmn.h>
#include <e32std.h>
#include <f32file.h>
#include <s32strm.h>

const int KShowInfoVersion = 8;

enum TPlayState {
	ENeverPlayed,
	EPlayed,
	EPlaying
};

enum TDownloadState {
	ENotDownloaded,
	EQueued,
	EDownloading,
	EDownloaded
};

enum TShowType {
	EAudioPodcast = 0,
	EAudioBook = 1, // this was previously a boolean IsAudioBook
	EVideoPodcast
};

class CShowInfo: public CBase {
public:
	static CShowInfo* NewL(TUint aVersion=KShowInfoVersion);
	~CShowInfo();
	CShowInfo(CShowInfo *aInfo); // copy constructor

public:
	const TDesC& Title() const;
	void SetTitleL(const TDesC& aTitle);

	const TDesC& Url() const;
	void SetUrlL(const TDesC &aUrl);
	
	const TDesC& Description() const;
	void SetDescriptionL(const TDesC &aDescription);
		
	TTimeIntervalMicroSeconds Position() const;
	void SetPosition(TTimeIntervalMicroSeconds aPosition);

	TUint PlayTime() const;
	void SetPlayTime(TUint aPlayTime);

	TPlayState PlayState();
	void SetPlayState(TPlayState aPlayState);

	TDownloadState DownloadState();
	void SetDownloadState(TDownloadState aDownloadState);

	TUint FeedUid();
	void SetFeedUid(TUint aFeedUid);

	void SetUid(TUint aUid);
	TUint Uid() const;
	
	TUint ShowSize();
	void SetShowSize(TUint aShowSize);
	
	const TTime PubDate() const;
	void SetPubDate(TTime aPubDate);
	
	const TDesC &FileName() const;
	void SetFileNameL(const TDesC &aFileName);
	
	void SetShowType(TShowType aShowType);
	TShowType ShowType();

	void SetTrackNo(TUint aTrackNo);
	TUint TrackNo() const;
private:
	CShowInfo();
	void ConstructL();

private:
	HBufC* iTitle;
	HBufC* iUrl;
	HBufC* iDescription;
	HBufC* iFileName;
	TTimeIntervalMicroSeconds iPosition;
	TUint   iPlayTime;
	TPlayState iPlayState;
	TDownloadState iDownloadState;
	TUint iFeedUid;
	TUint iUid;
	TUint iShowSize;
	TUint iTrackNo;
	TTime iPubDate;
	TBool iDelete;
	TShowType iShowType;
};

typedef RPointerArray<CShowInfo> RShowInfoArray;
#endif
