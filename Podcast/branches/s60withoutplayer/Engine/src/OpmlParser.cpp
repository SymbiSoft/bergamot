#include "OpmlParser.h"
#include "debug.h"
#include <f32file.h>
#include <bautils.h>
#include <s32file.h>
#include <charconv.h>
#include <xml/stringdictionarycollection.h>
#include <utf.h>

using namespace Xml;
const TInt KMaxParseBuffer = 1024;
const TInt KMaxStringBuffer = 100;
COpmlParser::COpmlParser(CFeedEngine& aFeedEngine, RFs& aFs) : iFeedEngine(aFeedEngine),iFs(aFs)
{	
}

COpmlParser::~COpmlParser()
{	
}

void COpmlParser::ParseOpmlL(const TFileName &feedFileName)
	{
	DP1("ParseOpmlL BEGIN: %S", &feedFileName);
	
	_LIT8(KXmlMimeType, "text/xml");
	// Contruct the parser object
	CParser* parser = CParser::NewLC(KXmlMimeType, *this);
	iOpmlState = EStateOpmlRoot;
	iEncoding = EUtf8;

	ParseL(*parser, iFs, feedFileName);

	CleanupStack::PopAndDestroy(parser);	
	//DP("ParseFeedL END");
	}

// from MContentHandler
void COpmlParser::OnStartDocumentL(const RDocumentParameters& aDocParam, TInt /*aErrorCode*/)
	{
	DP("OnStartDocumentL()");
	HBufC* charset = HBufC::NewLC(KMaxParseBuffer);
	charset->Des().Copy(aDocParam.CharacterSetName().DesC());
	iEncoding = EUtf8;
	if (charset->CompareF(_L("utf-8")) == 0) {
		DP("setting UTF8");
		iEncoding = EUtf8;
	} else if (charset->CompareF(_L("ISO-8859-1")) == 0) {
		iEncoding = EUtf8; //Latin1;
	} else {
		DP1("unknown charset: %S", &charset);
	}
	CleanupStack::PopAndDestroy(charset);
	}

void COpmlParser::OnEndDocumentL(TInt /*aErrorCode*/)
	{
	//DP("OnEndDocumentL()");
	}

void COpmlParser::OnStartElementL(const RTagInfo& aElement, const RAttributeArray& aAttributes, TInt /*aErrorCode*/)
	{
	TBuf<KMaxStringBuffer> str;
	str.Copy(aElement.LocalName().DesC());
	DP2("OnStartElementL START state=%d, element=%S", iOpmlState, &str);
	iBuffer.Zero();
	switch (iOpmlState) {
	case EStateOpmlRoot:
		// <body>
		if (str.CompareF(KTagBody) == 0) {
			iOpmlState = EStateOpmlBody;
		}
		break;
	case EStateOpmlBody:
		// <body> <outline>
		if(str.CompareF(KTagOutline) == 0) {
			//DP("New item");
			iOpmlState=EStateOpmlOutline;
			CFeedInfo* newFeed = CFeedInfo::NewL();
			
			TBool hasTitle = EFalse;
			TBool hasUrl = EFalse;
			
			for (int i=0;i<aAttributes.Count();i++) {
				RAttribute attr = aAttributes[i];
				TBuf<KMaxStringBuffer> attr16;
				attr16.Copy(attr.Attribute().LocalName().DesC());
				HBufC* val16 = HBufC::NewLC(KMaxParseBuffer);
				
				val16->Des().Copy(attr.Value().DesC());
				// xmlUrl=...
				if (attr16.Compare(KTagXmlUrl) == 0 || attr16.Compare(KTagUrl) == 0) {
					newFeed->SetUrlL(*val16);
					hasUrl = ETrue;
				// text=...
				} else if (attr16.Compare(KTagText) == 0) {
					newFeed->SetTitleL(*val16);
					newFeed->SetCustomTitle();
					hasTitle = ETrue;
				}
				CleanupStack::PopAndDestroy(val16);
			}

			if (!hasUrl) {
				break;
			}
			
			if (!hasTitle) {
				newFeed->SetTitleL(newFeed->Url());
			}
			
			iFeedEngine.AddFeedL(newFeed);
		}
		break;
	default:
		//DP2("Ignoring tag %S when in state %d", &str, iFeedState);
		break;
	}
//	DP1("OnStartElementL END state=%d", iFeedState);
	}

void COpmlParser::OnEndElementL(const RTagInfo& aElement, TInt /*aErrorCode*/)
	{
	
	TDesC8 lName = aElement.LocalName().DesC();
	TBuf<KMaxStringBuffer> str;
	str.Copy(aElement.LocalName().DesC());

	//DP("OnEndElementL START state=%d, element=%S"), iFeedState, &str);

	switch (iOpmlState) {
		case EStateOpmlOutline:
			iOpmlState=EStateOpmlBody;
			break;
		case EStateOpmlBody:
			iOpmlState = EStateOpmlRoot;
			break;
		default:
			// fall back to body level when in doubt
			iOpmlState = EStateOpmlBody;
			//DP("Don't know how to handle end tag %S when in state %d"), &str, iFeedState);
			break;
	}

	//DP("OnEndElementL END state=%d"), iFeedState);	
	}

void COpmlParser::OnContentL(const TDesC8& /*aBytes*/, TInt /*aErrorCode*/)
	{
	//DP("OnContentL()");
	}

void COpmlParser::OnStartPrefixMappingL(const RString& /*aPrefix*/, const RString& /*aUri*/, TInt /*aErrorCode*/)
	{
	//DP("OnStartPrefixMappingL()");
	}

void COpmlParser::OnEndPrefixMappingL(const RString& /*aPrefix*/, TInt /*aErrorCode*/)
	{
	//DP("OnEndPrefixMappingL()");
	}

void COpmlParser::OnIgnorableWhiteSpaceL(const TDesC8& /*aBytes*/, TInt /*aErrorCode*/)
	{
	//DP("OnIgnorableWhiteSpaceL()");
	}

void COpmlParser::OnSkippedEntityL(const RString& /*aName*/, TInt /*aErrorCode*/)
	{
	//DP("OnSkippedEntityL()");
	}

void COpmlParser::OnProcessingInstructionL(const TDesC8& /*aTarget*/, const TDesC8& /*aData*/, TInt /*aErrorCode*/)
	{
	//DP("OnProcessingInstructionL()");
	}

void COpmlParser::OnError(TInt aErrorCode)
	{
	DP1("COpmlParser::OnError %d", aErrorCode);
	}

TAny* COpmlParser::GetExtendedInterface(const TInt32 /*aUid*/)
	{
	//DP("GetExtendedInterface()");
	return NULL;
	}
