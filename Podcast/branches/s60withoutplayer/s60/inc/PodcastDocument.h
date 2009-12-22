#ifndef PODCASTDOCUMENT_H
#define PODCASTDOCUMENT_H

#include <akndoc.h>

class  CEikAppUi;
class CPodcastModel;
class CPodcastDocument : public CAknDocument
    {
    public:
        static CPodcastDocument* NewL( CEikApplication& aApp );
        virtual ~CPodcastDocument();

    private:
        CPodcastDocument( CEikApplication& aApp );
        void ConstructL();

    private:
        CEikAppUi* CreateAppUiL();
        CPodcastModel* iPodcastModel;
    };

#endif

