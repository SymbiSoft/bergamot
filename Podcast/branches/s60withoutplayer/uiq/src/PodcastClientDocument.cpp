#include "PodcastClientDocument.h"


/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
CPodcastClientDocument::CPodcastClientDocument (CQikApplication& aApp) : CQikDocument(aApp) {}


/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
CPodcastClientDocument* CPodcastClientDocument::NewL(CQikApplication& aApp)
{
  CPodcastClientDocument* self = new (ELeave) CPodcastClientDocument(aApp);
  CleanupStack::PushL(self);
  self->ConstructL();
  CleanupStack::Pop(self);
  return self;
}


/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
void CPodcastClientDocument::ConstructL() {}


/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
CPodcastClientDocument::~CPodcastClientDocument() {}


/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
CQikAppUi* CPodcastClientDocument::CreateAppUiL()
{
  return new(ELeave) CPodcastClientAppUi;
}
