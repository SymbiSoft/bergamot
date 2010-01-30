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

#include "FeedInfo.h"
#include <e32hashtab.h>


EXPORT_C CFeedInfo* CFeedInfo::NewL()
	{
	CFeedInfo* self = new (ELeave) CFeedInfo();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

EXPORT_C CFeedInfo* CFeedInfo::NewLC()
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

EXPORT_C CFeedInfo::~CFeedInfo()
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

EXPORT_C const TDesC& CFeedInfo::Url() const
	{
	return iUrl ? *iUrl : KNullDesC();
	}

EXPORT_C void CFeedInfo::SetUrlL(const TDesC &aUrl) 
	{
	if (iUrl)
		{
		delete iUrl;
		iUrl = NULL;
		}
		
	iUrl = aUrl.AllocL();		
	iUid = DefaultHash::Des16(Url());
	}

EXPORT_C const TDesC& CFeedInfo::Title() const
	{
	return iTitle ? *iTitle : KNullDesC();
	}

EXPORT_C void CFeedInfo::SetTitleL(const TDesC &aTitle)
	{
	if(iTitle)
		{
		delete iTitle;
		iTitle = NULL;
		}		
	iTitle = aTitle.AllocL();
	}

EXPORT_C const TDesC& CFeedInfo::Description() const
	{
	return iDescription ? *iDescription : KNullDesC();
	}

EXPORT_C void CFeedInfo::SetDescriptionL(const TDesC &aDescription)
	{
	if (iDescription)
		{
		delete iDescription;
		iDescription = NULL;
		}
	
	iDescription = aDescription.AllocL(); 
	}

EXPORT_C const TDesC& CFeedInfo::ImageUrl() const
	{
	return iImageUrl ? *iImageUrl : KNullDesC();
	}

EXPORT_C void CFeedInfo::SetImageUrlL(const TDesC &aImageUrl)
	{
	if (iImageUrl)
		{
		delete iImageUrl;
		iImageUrl = NULL;	
		}	
	iImageUrl = aImageUrl.AllocL();
	}

EXPORT_C const TDesC& CFeedInfo::Link() const
	{
	return iLink ? *iLink : KNullDesC();
	}

EXPORT_C void CFeedInfo::SetLinkL(const TDesC& aLink)
	{
	if (iLink)
		{
		delete iLink;
		iLink = NULL;
		}
	iLink = aLink.AllocL();
	}

EXPORT_C TTime CFeedInfo::BuildDate()
	{
	return iBuildDate;
	}

EXPORT_C void CFeedInfo::SetBuildDate(TTime aBuildDate)
	{
	iBuildDate = aBuildDate;
	}

EXPORT_C TTime CFeedInfo::LastUpdated()
	{
	return iLastUpdated;
	}

EXPORT_C void CFeedInfo::SetLastUpdated(TTime aUpdated)
	{
	iLastUpdated = aUpdated;
	}

EXPORT_C TUint CFeedInfo::Uid()
	{
	return iUid;
	}

EXPORT_C const TDesC& CFeedInfo::ImageFileName() const
	{
	return iImageFileName ? *iImageFileName : KNullDesC();
	}

EXPORT_C void CFeedInfo::SetImageFileNameL(const TDesC& aFileName)
	{
	if (iImageFileName)
		{
		delete iImageFileName;
		iImageFileName = NULL;
		}
	iImageFileName = aFileName.AllocL();
	}

EXPORT_C TBool CFeedInfo::CustomTitle()
	{
	return iCustomTitle;
	}

EXPORT_C void CFeedInfo::SetCustomTitle()
	{
	iCustomTitle = ETrue;
	}
