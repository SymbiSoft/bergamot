#ifndef OPMLPARSER_H_
#define OPMLPARSER_H_

#include <xml/parser.h> // for CParser
#include <xml/contenthandler.h>	// for Xml::MContentHandler
#include <xml/documentparameters.h>
#include "FeedEngine.h"

_LIT(KTagOpml, "opml");
_LIT(KTagBody, "body");
_LIT(KTagOutline, "outline");
_LIT(KTagXmlUrl, "xmlUrl");
_LIT(KTagText, "text");

enum TOpmlState {
	EStateOpmlRoot,
	EStateOpmlBody,
	EStateOpmlOutline
	};
/*
enum TEncoding {
	EUtf8,
	ELatin1
};

const int KBufferLength = 1024;
*/
class COpmlParser : public CBase, public Xml::MContentHandler 
{
public:
	COpmlParser(CFeedEngine& aFeedEngine);
	virtual ~COpmlParser();
	
public:
	void ParseOpmlL(const TFileName &feedFileName);
	
public: // from MContentHandler
	void OnStartDocumentL(const Xml::RDocumentParameters& aDocParam, TInt aErrorCode);
	void OnEndDocumentL(TInt aErrorCode);
	void OnStartElementL(const Xml::RTagInfo& aElement, const Xml::RAttributeArray& aAttributes, TInt aErrorCode);
	void OnEndElementL(const Xml::RTagInfo& aElement, TInt aErrorCode);
	void OnContentL(const TDesC8& aBytes, TInt aErrorCode);
	void OnStartPrefixMappingL(const RString& aPrefix, const RString& aUri, TInt aErrorCode);
	void OnEndPrefixMappingL(const RString& aPrefix, TInt aErrorCode);
	void OnIgnorableWhiteSpaceL(const TDesC8& aBytes, TInt aErrorCode);
	void OnSkippedEntityL(const RString& aName, TInt aErrorCode);
	void OnProcessingInstructionL(const TDesC8& aTarget, const TDesC8& aData, TInt aErrorCode);
	void OnError(TInt aErrorCode);
	TAny* GetExtendedInterface(const TInt32 aUid);
private:
	CFeedEngine& iFeedEngine;
	TOpmlState iOpmlState;

	TBuf<KBufferLength> iBuffer;
	TEncoding iEncoding;

};

#endif
