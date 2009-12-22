// SyncClientDocument.cpp
#include "SyncClientDocument.h"


/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
CSyncClientDocument::CSyncClientDocument (CQikApplication& aApp) : CQikDocument(aApp) {}


/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
CSyncClientDocument* CSyncClientDocument::NewL(CQikApplication& aApp)
{
  CSyncClientDocument* self = new (ELeave) CSyncClientDocument(aApp);
  CleanupStack::PushL(self);
  self->ConstructL();
  CleanupStack::Pop(self);
  return self;
}


/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
void CSyncClientDocument::ConstructL() {}


/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
CSyncClientDocument::~CSyncClientDocument() {}


/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
CQikAppUi* CSyncClientDocument::CreateAppUiL()
{
  return new(ELeave) CSyncClientAppUi;
}
