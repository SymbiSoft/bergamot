// HttpClient.cpp


#include <e32base.h>
#include <http/rhttpheaders.h>
#include <http.h>
#include <commdb.h>
#include <eikenv.h>
#include <es_sock.h>
#include <bautils.h>
#include <CommDbConnPref.h>
#include "debug.h"
#include "constants.h"
#include "HttpClient.h"
const TInt KTempBufferSize = 100;

CHttpClient::~CHttpClient()
  {
  if (iHandler)
  	{
  	iHandler->CloseSaveFile();
	delete iHandler;
  	}
  
  iSession.Close();
  }

CHttpClient* CHttpClient::NewL(CPodcastModel& aPodcastModel, MHttpClientObserver& aObserver)
  {
  CHttpClient* me = NewLC(aPodcastModel, aObserver);
  CleanupStack::Pop(me);
  return me;
  }

CHttpClient::CHttpClient(CPodcastModel& aPodcastModel, MHttpClientObserver& aObserver) : iPodcastModel(aPodcastModel), iObserver(aObserver)
  {
  iResumeEnabled = EFalse;
  }

CHttpClient* CHttpClient::NewLC(CPodcastModel& aPodcastModel, MHttpClientObserver& aObserver)
  {
  CHttpClient* me = new (ELeave) CHttpClient(aPodcastModel, aObserver);
  CleanupStack::PushL(me);
  me->ConstructL();
  return me;
  }

void CHttpClient::ConstructL()
  {

  }

void CHttpClient::SetHeaderL(RHTTPHeaders aHeaders, TInt aHdrField, const TDesC8& aHdrValue)
	{
	RStringF valStr = iSession.StringPool().OpenFStringL(aHdrValue);
	THTTPHdrVal val(valStr);
	aHeaders.SetFieldL(iSession.StringPool().StringF(aHdrField, RHTTPSession::GetTable()), val);
	valStr.Close();
	}

TBool CHttpClient::IsActive()
	{
	return iIsActive;
	}

void CHttpClient::SetResumeEnabled(TBool aEnabled)
	{
	iResumeEnabled = aEnabled;
	}

TBool CHttpClient::GetL(const TDesC& url, const TDesC& fileName,  TBool aSilent) {
	DP("CHttpClient::Get START");
	
	__ASSERT_DEBUG((iIsActive==EFalse), User::Panic(_L("Already active"), -2));
			
	HBufC8* url8 = HBufC8::NewLC(KDefaultURLBufferLength);
	url8->Des().Copy(url);
	
	if (iTransactionCount == 0) 
		{
		DP("CHttpClient::GetL\t*** Opening HTTP session ***");
		iSession.OpenL();
		if(!iPodcastModel.ConnectHttpSessionL(iSession)) // Returns false if not connected
			{
			CleanupStack::PopAndDestroy(url8);
			iSession.Close();
			return EFalse;
			}
		}
	
	TUriParser8 uri; 
	TInt urlError = uri.Parse(*url8);
	DP2("Getting '%S' to '%S'", &url, &fileName);
	if(urlError != KErrNone ||!uri.IsSchemeValid())
		{
		CleanupStack::PopAndDestroy(url8);
		iSession.Close();
		return EFalse;
		}

	// since nothing should be downloading now. Delete the handler
	if (iHandler)
		{
		delete iHandler;
		iHandler = NULL;
		}
		
	iHandler = CHttpEventHandler::NewL(this, iObserver, iPodcastModel.FsSession());
	iHandler->SetSilent(aSilent);

	TEntry entry;
	TBuf8<KTempBufferSize> rangeText;

	if (iResumeEnabled && iPodcastModel.FsSession().Entry(fileName, entry) == KErrNone) {
		DP1("Found file, with size=%d", entry.iSize);
		// file exists, so we should probably resume
		rangeText.Format(_L8("bytes=%d-"), entry.iSize-KByteOverlap);
		iHandler->SetSaveFileName(fileName, ETrue);
	} else {
		// otherwise just make sure the directory exists
		BaflUtils::EnsurePathExistsL(iPodcastModel.FsSession(),fileName);
		iHandler->SetSaveFileName(fileName);
	}
	
	RStringPool strP = iSession.StringPool();
	RStringF method;
	method = strP.StringF(HTTP::EGET, RHTTPSession::GetTable());

	iTrans = iSession.OpenTransactionL(uri, *iHandler, method);
	RHTTPHeaders hdr = iTrans.Request().GetHeaderCollection();
	// Add headers appropriate to all methods
	SetHeaderL(hdr, HTTP::EUserAgent, KUserAgent);
	SetHeaderL(hdr, HTTP::EAccept, KAccept);
	TBuf<KTempBufferSize> range16;
	range16.Copy(rangeText);
	DP1("range text: %S", &range16);
	if (rangeText.Length() > 0) {
		SetHeaderL(hdr, HTTP::ERange, rangeText);
	}
	iTransactionCount++;
	// submit the transaction
	iTrans.SubmitL();
	iIsActive = ETrue;	
	DP("CHttpClient::Get END");
	CleanupStack::PopAndDestroy(url8);
	return ETrue;
}

void CHttpClient::Stop()
	{
	iIsActive = EFalse;
	if(iHandler != NULL)
		{
		// cancel the ongoing transaction
		iTrans.Cancel();
		iTransactionCount = 0;
		
		// make sure that we save the file
		iHandler->CloseSaveFile();
		
		// we could now delete the handler since a new will be created
		delete iHandler;
		iHandler = NULL;
		
		// close the session
		DP("CHttpClient::Stop\t*** Closing HTTP session ***");
		iSession.Close();
		}

	TRAP_IGNORE(iObserver.CompleteL(this, KErrDisconnected));

	}

void CHttpClient::ClientRequestCompleteL(TInt aErrorCode) {
	iIsActive = EFalse;
	iObserver.CompleteL(this, aErrorCode);
	DP("CHttpClient::ClientRequestCompleteL");
	iTransactionCount--;
	
	if(iTransactionCount == 0) 
		{
		DP("CHttpClient::ClientRequestCompleteL\t*** Closing HTTP session ***");
		delete iHandler;
		iHandler = NULL;
		iSession.Close();
	}
}
