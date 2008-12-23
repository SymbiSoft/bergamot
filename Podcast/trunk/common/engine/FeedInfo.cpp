#include "FeedInfo.h"
#include <e32hashtab.h>


CFeedInfo* CFeedInfo::NewL()
	{
	CFeedInfo* self = new (ELeave) CFeedInfo();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

CFeedInfo* CFeedInfo::NewLC()
	{
	CFeedInfo* self = new (ELeave) CFeedInfo();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CFeedInfo::CFeedInfo()
	{
	iCustomTitle = EFalse;
	}

CFeedInfo::~CFeedInfo()
	{
	delete iUrl;
	delete iTitle;
	delete iDescription;
	delete iImageUrl;
	delete iImageFileName;
	delete iLink;
	}

void CFeedInfo::ConstructL()
	{
	}

const TDesC& CFeedInfo::Url() const
	{
	return iUrl ? *iUrl : KNullDesC();
	}

void CFeedInfo::SetUrlL(const TDesC &aUrl) 
	{
	if (iUrl)
		{
		delete iUrl;
		iUrl = NULL;
		}
		
	iUrl = aUrl.AllocL();		
	iUid = DefaultHash::Des16(Url());
	}

const TDesC& CFeedInfo::Title() const
	{
	return iTitle ? *iTitle : KNullDesC();
	}

void CFeedInfo::SetTitleL(const TDesC &aTitle)
	{
	if(iTitle)
		{
		delete iTitle;
		iTitle = NULL;
		}		
	iTitle = aTitle.AllocL();
	}

const TDesC& CFeedInfo::Description() const
	{
	return iDescription ? *iDescription : KNullDesC();
	}

void CFeedInfo::SetDescriptionL(const TDesC &aDescription)
	{
	if (iDescription)
		{
		delete iDescription;
		iDescription = NULL;
		}
	
	iDescription = aDescription.AllocL(); 
	}

const TDesC& CFeedInfo::ImageUrl() const
	{
	return iImageUrl ? *iImageUrl : KNullDesC();
	}

void CFeedInfo::SetImageUrlL(const TDesC &aImageUrl)
	{
	if (iImageUrl)
		{
		delete iImageUrl;
		iImageUrl = NULL;	
		}	
	iImageUrl = aImageUrl.AllocL();
	}

const TDesC& CFeedInfo::Link() const
	{
	return iLink ? *iLink : KNullDesC();
	}

void CFeedInfo::SetLinkL(const TDesC& aLink)
	{
	if (iLink)
		{
		delete iLink;
		iLink = NULL;
		}
	iLink = aLink.AllocL();
	}

TTime CFeedInfo::BuildDate()
	{
	return iBuildDate;
	}

void CFeedInfo::SetBuildDate(TTime aBuildDate)
	{
	iBuildDate = aBuildDate;
	}

TTime CFeedInfo::LastUpdated()
	{
	return iLastUpdated;
	}

void CFeedInfo::SetLastUpdated(TTime aUpdated)
	{
	iLastUpdated = aUpdated;
	}

TUint CFeedInfo::Uid()
	{
	return iUid;
	}

const TDesC& CFeedInfo::ImageFileName() const
	{
	return iImageFileName ? *iImageFileName : KNullDesC();
	}

void CFeedInfo::SetImageFileNameL(const TDesC& aFileName)
	{
	if (iImageFileName)
		{
		delete iImageFileName;
		iImageFileName = NULL;
		}
	iImageFileName = aFileName.AllocL();
	}


TBool CFeedInfo::IsBookFeed()
	{
	return iIsBookFeed;
	}

void CFeedInfo::SetIsBookFeed()
	{
	iIsBookFeed = ETrue;
	}

TBool CFeedInfo::CustomTitle()
	{
	return iCustomTitle;
	}

void CFeedInfo::SetCustomTitle()
	{
	iCustomTitle = ETrue;
	}
