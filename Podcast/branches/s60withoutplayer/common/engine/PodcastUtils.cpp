#include "PodcastUtils.h"
			
void PodcastUtils::FixProtocolsL(TDes &aUrl)
	{
	// url is always present so access that								
	// Some pod links are written in format itpc://mylink.net/podcast.xml
	// Symbian HTTP stack does not like itpc:// 
	// Try to use a HTTP instead.
	TInt p = aUrl.Find(KItpcPrefix);
	if (p >= 0)
		{
		aUrl.Delete(p, KItpcPrefix().Length());
		}

	// Some pod links are written in format pcast://mylink.net/podcast.xml
	// Symbian HTTP stack does not like itpc:// 
	// Try to use a HTTP instead.
	p = aUrl.Find(KPcastPrefix);
	if (p >= 0)
		{
		aUrl.Delete(p, KPcastPrefix().Length());
		}

	// The URL must start with http://, otherwise the HTTP stack fails.
	TInt pos = aUrl.Find(KURLPrefix);
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
	}
