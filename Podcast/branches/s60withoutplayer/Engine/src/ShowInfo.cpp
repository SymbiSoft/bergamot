#include "ShowInfo.h"
#include <e32hashtab.h>

EXPORT_C CShowInfo* CShowInfo::NewL(TUint /* aVersion */)
	{
	CShowInfo* self = new (ELeave) CShowInfo();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

void CShowInfo::ConstructL()
	{
	iDownloadState = ENotDownloaded;
	iDelete = EFalse;
	iShowType = EAudioPodcast;
	}

CShowInfo::CShowInfo()
	{
	iTrackNo = KMaxTUint; 
	}

EXPORT_C CShowInfo::~CShowInfo()
	{
	delete iTitle;
	delete iUrl;
	delete iDescription;
	delete iFileName;
	}

EXPORT_C const TDesC& CShowInfo::Title() const
	{
	return iTitle ? *iTitle : KNullDesC();
	}

EXPORT_C void CShowInfo::SetTitleL(const TDesC &aTitle)
	{
	if (iTitle)
		{
		delete iTitle;
		iTitle = NULL;
		}
	iTitle = aTitle.AllocL();
	}

EXPORT_C const TDesC& CShowInfo::Url() const
	{
	return iUrl ? *iUrl : KNullDesC();
	}

EXPORT_C void CShowInfo::SetUrlL(const TDesC &aUrl)
	{
	if (iUrl)
		{
		delete iUrl;
		iUrl = NULL;
		}
	iUrl = aUrl.AllocL();
	iUid = DefaultHash::Des16(Url());
	}

EXPORT_C const TDesC& CShowInfo::Description() const
	{
	return iDescription ? *iDescription : KNullDesC();
	}

EXPORT_C void CShowInfo::SetDescriptionL(const TDesC &aDescription)
	{
	if (iDescription)
		{
		delete iDescription;
		iDescription = NULL;
		}

	iDescription = aDescription.AllocL();
	}

EXPORT_C TTimeIntervalMicroSeconds CShowInfo::Position() const
	{
	return iPosition;
	}

EXPORT_C void CShowInfo::SetPosition(TTimeIntervalMicroSeconds aPosition)
	{
	iPosition = aPosition;
	}

EXPORT_C TUint CShowInfo::PlayTime() const
	{
	return iPlayTime;
	}

EXPORT_C void CShowInfo::SetPlayTime(TUint aPlayTime)
	{
	iPlayTime = aPlayTime;
	}

EXPORT_C TPlayState CShowInfo::PlayState()
	{
	return iPlayState;
	}

EXPORT_C void CShowInfo::SetPlayState(TPlayState aPlayState)
	{
	iPlayState = aPlayState;
	}

EXPORT_C TDownloadState CShowInfo::DownloadState()
	{
	return iDownloadState;
	}

EXPORT_C void CShowInfo::SetDownloadState(TDownloadState aDownloadState)
	{
	iDownloadState = aDownloadState;
	}

EXPORT_C TUint CShowInfo::FeedUid()
	{
	return iFeedUid;
	}

EXPORT_C void CShowInfo::SetFeedUid(TUint aFeedUid)
	{
	iFeedUid = aFeedUid;
	}

EXPORT_C void CShowInfo::SetUid(TUint aUid)
	{
	iUid = aUid;
	}
		
EXPORT_C TUint CShowInfo::Uid() const
	{
	return iUid;
	}

EXPORT_C TUint CShowInfo::ShowSize()
	{
	return iShowSize;
	}

EXPORT_C void CShowInfo::SetShowSize(TUint aShowSize)
	{
	iShowSize = aShowSize;
	}

EXPORT_C const TTime CShowInfo::PubDate() const
	{
	return iPubDate;
	}

EXPORT_C void CShowInfo::SetPubDate(TTime aPubDate)
	{
	iPubDate = aPubDate;
	}

EXPORT_C const TDesC& CShowInfo::FileName() const
	{
	return iFileName ? *iFileName : KNullDesC();
	}

EXPORT_C void CShowInfo::SetFileNameL(const TDesC &aFileName)
	{
	if (iFileName)
		{
		delete iFileName;
		iFileName = NULL;
		}
	iFileName = aFileName.AllocL();
	}

EXPORT_C void CShowInfo::SetShowType(TShowType aShowType)
	{
	iShowType = aShowType;
	}

EXPORT_C TShowType CShowInfo::ShowType()
	{
	return iShowType;
	}

EXPORT_C void CShowInfo::SetTrackNo(TUint aTrackId)
	{
	iTrackNo = aTrackId;
	}

EXPORT_C TUint CShowInfo::TrackNo() const
	{
	return iTrackNo;
	}

EXPORT_C CShowInfo::CShowInfo(CShowInfo *aInfo)
	{
	iTitle = aInfo->Title().Alloc();
	iUrl = aInfo->Url().Alloc();
	iDescription = aInfo->Description().Alloc();
	iFileName = aInfo->FileName().Alloc();
	iPosition = aInfo->Position();
	iPlayTime = aInfo->PlayTime();
	iPlayState = aInfo->PlayState();
	iDownloadState = aInfo->DownloadState();
	iFeedUid = aInfo->FeedUid();
	iUid = aInfo->Uid();
	iShowSize = aInfo->ShowSize();
	iTrackNo = aInfo->TrackNo();
	iPubDate = aInfo->PubDate();
	iShowType = aInfo->ShowType();
	}

