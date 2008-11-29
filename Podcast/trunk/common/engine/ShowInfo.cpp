#include "ShowInfo.h"
#include <e32hashtab.h>

CShowInfo* CShowInfo::NewL(TUint aVersion)
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

CShowInfo::~CShowInfo()
	{
	delete iTitle;
	delete iUrl;
	delete iDescription;
	delete iFileName;
	}

const TDesC& CShowInfo::Title() const
	{
	return iTitle ? *iTitle : KNullDesC();
	}

void CShowInfo::SetTitleL(const TDesC &aTitle)
	{
	if (iTitle)
		{
		delete iTitle;
		iTitle = NULL;
		}
	iTitle = aTitle.AllocL();
	}

const TDesC& CShowInfo::Url() const
	{
	return iUrl ? *iUrl : KNullDesC();
	}

void CShowInfo::SetUrlL(const TDesC &aUrl)
	{
	if (iUrl)
		{
		delete iUrl;
		iUrl = NULL;
		}
	iUrl = aUrl.AllocL();
	iUid = DefaultHash::Des16(Url());
	}

const TDesC& CShowInfo::Description() const
	{
	return iDescription ? *iDescription : KNullDesC();
	}

void CShowInfo::SetDescriptionL(const TDesC &aDescription)
	{
	if (iDescription)
		{
		delete iDescription;
		iDescription = NULL;
		}

	iDescription = aDescription.AllocL();
	}

TTimeIntervalMicroSeconds CShowInfo::Position() const
	{
	return iPosition;
	}

void CShowInfo::SetPosition(TTimeIntervalMicroSeconds aPosition)
	{
	iPosition = aPosition;
	}

TUint CShowInfo::PlayTime() const
	{
	return iPlayTime;
	}

void CShowInfo::SetPlayTime(TUint aPlayTime)
	{
	iPlayTime = aPlayTime;
	}

TPlayState CShowInfo::PlayState()
	{
	return iPlayState;
	}

void CShowInfo::SetPlayState(TPlayState aPlayState)
	{
	iPlayState = aPlayState;
	}

TDownloadState CShowInfo::DownloadState()
	{
	return iDownloadState;
	}

void CShowInfo::SetDownloadState(TDownloadState aDownloadState)
	{
	iDownloadState = aDownloadState;
	}

TUint CShowInfo::FeedUid()
	{
	return iFeedUid;
	}

void CShowInfo::SetFeedUid(TUint aFeedUid)
	{
	iFeedUid = aFeedUid;
	}

void CShowInfo::SetUid(TUint aUid)
	{
	iUid = aUid;
	}
		
TUint CShowInfo::Uid() const
	{
	return iUid;
	}

TUint CShowInfo::ShowSize()
	{
	return iShowSize;
	}

void CShowInfo::SetShowSize(TUint aShowSize)
	{
	iShowSize = aShowSize;
	}

const TTime CShowInfo::PubDate() const
	{
	return iPubDate;
	}

void CShowInfo::SetPubDate(TTime aPubDate)
	{
	iPubDate = aPubDate;
	}

const TDesC& CShowInfo::FileName() const
	{
	return iFileName ? *iFileName : KNullDesC();
	}

void CShowInfo::SetFileNameL(const TDesC &aFileName)
	{
	if (iFileName)
		{
		delete iFileName;
		iFileName = NULL;
		}
	iFileName = aFileName.AllocL();
	}

void CShowInfo::SetShowType(TShowType aShowType)
	{
	iShowType = aShowType;
	}

TShowType CShowInfo::ShowType()
	{
	return iShowType;
	}

void CShowInfo::SetTrackNo(TUint aTrackId)
	{
	iTrackNo = aTrackId;
	}

TUint CShowInfo::TrackNo() const
	{
	return iTrackNo;
	}

CShowInfo::CShowInfo(CShowInfo *aInfo)
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

