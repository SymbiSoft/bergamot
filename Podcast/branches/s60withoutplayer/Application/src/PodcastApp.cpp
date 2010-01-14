#include    "PodcastApp.h"
#include    "PodcastDocument.h"

TUid CPodcastApp::AppDllUid() const
    {
    return KUidPodcast;
    }

CApaDocument* CPodcastApp::CreateDocumentL()
    {
    return CPodcastDocument::NewL( *this );
    }

