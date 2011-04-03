// Application.cpp

#include "SyncClientView.h"
#include "SyncClientApplication.h"


/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
CApaDocument* CSyncClientApplication::CreateDocumentL()
{
  return CSyncClientDocument::NewL(*this);
}

/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
TUid CSyncClientApplication::AppDllUid() const
{
  return KUidSyncClientID;
}


/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
EXPORT_C CApaApplication* CSyncClientApplication::NewApplication()
{
  return new CSyncClientApplication;
}


/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
LOCAL_C CApaApplication* NewApplication()
{
  return new CSyncClientApplication;
}


/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
GLDEF_C TInt E32Main()
{
  return EikStart::RunApplication(NewApplication);
}


