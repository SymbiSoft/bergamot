#include <eikenv.h>
#include <ImplementationProxy.h> 
#include "debug.h"

// User include
#include "SyncServerRecognizer.h"

const TInt KNumDataTypes = 1;
const TUid KExampleDllUid = {0xE0983803};
const TInt KImplementationUid = 0xE0983803;

// An example mime type
_LIT8(KExampleTextMimeType, "text/example");

/*
Constructor - sets the number of supported mime types, 
the recognizer's priority and its UID.
*/
CSyncServerRecognizer::CSyncServerRecognizer():CApaDataRecognizerType(KExampleDllUid, CApaDataRecognizerType::EHigh)
	{
    iCountDataTypes = KNumDataTypes;
	}

/*
Specifies this recognizer's preferred data buffer size passed to DoRecognizeL().
The actual value used will be the maximum of all recognizers.
*/
TUint CSyncServerRecognizer::PreferredBufSize()
	{
    return 24;
	}


/*
Returns the indexed data type that the recognizer can recognize. 
In this case, only 1 is supported. 
*/
TDataType CSyncServerRecognizer::SupportedDataTypeL(TInt /*aIndex*/) const
	{
	return TDataType(KExampleTextMimeType);
	}

/*
Attempts to recognize the data type, given the filename and data buffer.
*/
void CSyncServerRecognizer::DoRecognizeL(const TDesC& aName, const TDesC8& aBuffer)
	{
	_LIT8(KExampleData, "example");
	_LIT(KDotExample, ".Example");

	TParse parse;
	parse.Set(aName,NULL,NULL);
	TPtrC ext=parse.Ext(); // extract the extension from the filename

	if (ext.CompareF(KDotExample)==0 && aBuffer.FindF(KExampleData)!=KErrNotFound)
		{
		iConfidence=ECertain;
		iDataType=TDataType(KExampleTextMimeType); 
		}
    }

_LIT(KSysStartProcessMatch, "SysStart*");
 
void CSyncServerRecognizer::WaitForSysStartL()
{
	//CActiveScheduler *pA=new CActiveScheduler;
	//CActiveScheduler::Install(pA);

	iAppStarter.ConstructL();

	
	//delete pA;
}

CAppStarter::CAppStarter() : CActive(EPriorityNormal) { }

void CAppStarter::ConstructL()
	{
	TFindProcess findProc(KSysStartProcessMatch);
	TFullName procName;
	
	CActiveScheduler::Add(this);
	if (findProc.Next(procName) != KErrNotFound)
	{
		User::LeaveIfError(iSysStartProcess.Open(findProc));
		iSysStartProcess.Logon(iStatus);
		SetActive();
	}
	}

void CAppStarter::DoCancel() { }

void CAppStarter::RunL()
{
	DP("CAppStarter::RunL");
	User::LeaveIfError(iStatus.Int());

	RProcess process;
	int ret = process.Create(_L("SyncServer"), _L(""));
	
	process.Resume();
	DP1("Started process, ret=%d", ret);
	
}


/*
The ECom implementation creation function.
*/
CApaDataRecognizerType* CSyncServerRecognizer::CreateRecognizerL()
	{
	//return new (ELeave) CSyncServerRecognizer;

	CSyncServerRecognizer* rec = new CSyncServerRecognizer();
	CTrapCleanup* tc = CTrapCleanup::New();

	rec->WaitForSysStartL();
	delete tc;
	return rec; // NULL if new failed
	}

/*
Standard ECom framework code 
*/
const TImplementationProxy ImplementationTable[] = 
    {
	IMPLEMENTATION_PROXY_ENTRY(KImplementationUid,CSyncServerRecognizer::CreateRecognizerL)
	};

/*
Standard ECom framework code 
*/
EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
    {
    aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy);
    return ImplementationTable;
    }
