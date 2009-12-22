#include "PodcastClientView.h"
#include "PodcastClientApplication.h"

/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
CApaDocument* CPodcastClientApplication::CreateDocumentL()
{
  return CPodcastClientDocument::NewL(*this);
}

/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
TUid CPodcastClientApplication::AppDllUid() const
{
  return KUidPodcastClientID;
}


/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
EXPORT_C CApaApplication* CPodcastClientApplication::NewApplication()
{
  return new CPodcastClientApplication;
}


/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
LOCAL_C CApaApplication* NewApplication()
{
  return new CPodcastClientApplication;
}


/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
GLDEF_C TInt E32Main()
{
  return EikStart::RunApplication(NewApplication);
}


