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
#include "connectionengine.h"
#include "settingsengine.h"

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
  iPodcastModel.ConnectionEngine().AddObserver(this);
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


TBool CHttpClient::ConnectHttpSessionL()
{
	DP("ConnectHttpSessionL START");	
	if(iPodcastModel.ConnectionEngine().ConnectionState() == CConnectionEngine::EConnected)
		{
		DP("ConnectionState == CConnectionEngine::EConnected");
		// Session already connected, call connect complete directly
		ConnectCompleteL(KErrNone);
		return ETrue;
		}

	DP1("SpecificIAP() == %d",iPodcastModel.SettingsEngine().SpecificIAP());

	if(iPodcastModel.SettingsEngine().SpecificIAP() == -1)
		{
		iPodcastModel.ConnectionEngine().StartL(CConnectionEngine::EUserSelectConnection);	
		}
	else if ( iPodcastModel.SettingsEngine().SpecificIAP() == 0 )
		{
		iPodcastModel.ConnectionEngine().StartL(CConnectionEngine::EDefaultConnection);	
		}
	else
		{
		iPodcastModel.ConnectionEngine().StartL(CConnectionEngine::EMobilityConnection);
		}
		
	DP("ConnectHttpSessionL END");
	return EFalse;
}

void CHttpClient::ConnectCompleteL(TInt aErrorCode)
	{
	if(iWaitingForGet)
		{
		iWaitingForGet = EFalse;
		if( aErrorCode == KErrNone)
			{
			RHTTPConnectionInfo connInfo = iSession.ConnectionInfo();
			RStringPool pool = iSession.StringPool();
			// Attach to socket server
			connInfo.SetPropertyL(pool.StringF(HTTP::EHttpSocketServ, RHTTPSession::GetTable()), THTTPHdrVal(iPodcastModel.ConnectionEngine().SockServ().Handle()));
			// Attach to connection
			TInt connPtr = REINTERPRET_CAST(TInt, &iPodcastModel.ConnectionEngine().Connection());
			connInfo.SetPropertyL(pool.StringF(HTTP::EHttpSocketConnection, RHTTPSession::GetTable()), THTTPHdrVal(connPtr));


			iPodcastModel.SetProxyUsageIfNeededL(iSession);
			
			if(!DoGetAfterConnectL())
				{
				iObserver.CompleteL(this, KErrBadName);
				}
			}
		else
			{
			ClientRequestCompleteL(aErrorCode);
			iSession.Close();
			}
		}			
	}

void CHttpClient::Disconnected()
	{
	iIsActive = EFalse;
	iSession.Close();
	}

TBool CHttpClient::DoGetAfterConnectL()
	{
	TUriParser8 uri; 
	TInt urlError = uri.Parse(iCurrentURL);
	
	if(urlError != KErrNone ||!uri.IsSchemeValid())
		{		
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
	iHandler->SetSilent(iSilentGet);

	TEntry entry;
	TBuf8<KTempBufferSize> rangeText;

	if (iResumeEnabled && iPodcastModel.FsSession().Entry(iCurrentFileName, entry) == KErrNone) {
		DP1("Found file, with size=%d", entry.iSize);
		// file exists, so we should probably resume
		rangeText.Format(_L8("bytes=%d-"), entry.iSize-KByteOverlap);
		iHandler->SetSaveFileName(iCurrentFileName, ETrue);
	} else {
		// otherwise just make sure the directory exists
		BaflUtils::EnsurePathExistsL(iPodcastModel.FsSession(),iCurrentFileName);
		iHandler->SetSaveFileName(iCurrentFileName);
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
	return ETrue;
	}

TBool CHttpClient::GetL(const TDesC& aUrl, const TDesC& aFileName,  TBool aSilent) {
	DP("CHttpClient::Get START");
	DP2("Getting '%S' to '%S'", &aUrl, &aFileName);	
	__ASSERT_DEBUG((iIsActive==EFalse), User::Panic(_L("Already active"), -2));
	iCurrentURL.Copy(aUrl);				
	iSilentGet = aSilent;
	iCurrentFileName.Copy(aFileName);
	iWaitingForGet = ETrue;
	
	if (iTransactionCount == 0) 
		{
		DP("CHttpClient::GetL\t*** Opening HTTP session ***");
		iSession.Close();
		iSession.OpenL();
		ConnectHttpSessionL();	
		return ETrue;
		}
	else
		{
		return DoGetAfterConnectL();
		}		
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
	if(iTransactionCount>0)
		{
		iTransactionCount--;
	
		if(iTransactionCount == 0) 
			{
			DP("CHttpClient::ClientRequestCompleteL\t*** Closing HTTP session ***");
			delete iHandler;
			iHandler = NULL;
			iSession.Close();
			}
		}
}
