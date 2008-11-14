#include "PodcastDocument.h"
#include "PodcastAppui.h"
#include "PodcastModel.h"
CPodcastDocument::CPodcastDocument( CEikApplication& aApp )
    : CAknDocument( aApp )    
    {
    }

CPodcastDocument::~CPodcastDocument()
    {
    delete iPodcastModel;
    }

void CPodcastDocument::ConstructL()
    {
    iPodcastModel = CPodcastModel::NewL();
    }

CPodcastDocument* CPodcastDocument::NewL(CEikApplication& aApp )
    {
    CPodcastDocument* self = new (ELeave) CPodcastDocument( aApp );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }
    
CEikAppUi* CPodcastDocument::CreateAppUiL()
    {
    return new (ELeave) CPodcastAppUi;
    }
