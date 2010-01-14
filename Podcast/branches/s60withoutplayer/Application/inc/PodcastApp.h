#ifndef PODCASTAPP_H
#define PODCASTAPP_H

#include <aknapp.h>

const TUid KUidPodcast = { 0xA0009D00 };

class CPodcastApp : public CAknApplication
    {
    private:

        CApaDocument* CreateDocumentL();
        TUid AppDllUid() const;
    };

#endif
