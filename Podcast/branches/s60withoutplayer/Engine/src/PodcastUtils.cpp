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

#include <e32std.h>
#include <e32base.h>
#include "PodcastUtils.h"
			
EXPORT_C void PodcastUtils::FixProtocolsL(TDes &aUrl)
	{
	HBufC* urlCopy = aUrl.AllocLC();
	TPtr urlCopyPtr (urlCopy->Des());
	urlCopyPtr.LowerCase();
	
	// url is always present so access that								
	// Some pod links are written in format itpc://mylink.net/podcast.xml
	// Symbian HTTP stack does not like itpc:// 
	// Try to use a HTTP instead.
	TInt p = urlCopyPtr.Find(KItpcPrefix);
	if (p >= 0)
		{
		aUrl.Delete(p, KItpcPrefix().Length());
		}

	// Some pod links are written in format pcast://mylink.net/podcast.xml
	// Symbian HTTP stack does not like itpc:// 
	// Try to use a HTTP instead.
	p = urlCopyPtr.Find(KPcastPrefix);
	if (p >= 0)
		{
		aUrl.Delete(p, KPcastPrefix().Length());
		}

	// The URL must start with http://, otherwise the HTTP stack fails.
	TInt pos = urlCopyPtr.Find(KURLPrefix);
	if (pos == KErrNotFound)
		{
		HBufC* newUrl = HBufC::NewL(aUrl.Length() + KURLPrefix().Length());
		TPtr ptr = newUrl->Des();
		ptr.Append(KURLPrefix());
		ptr.Append(aUrl);

		// replace the url buffer
		aUrl.Copy(*newUrl);
		delete newUrl;					
		}
	
	CleanupStack::PopAndDestroy(urlCopy);
	}

