#include <eikedwin.h>
#include <e32hashtab.h>

#include "PodcastClient.hrh"
#include "PodcastClientAddFeedDlg.h"
#include "PodcastModel.h"
#include "FeedEngine.h"

CPodcastClientAddFeedDlg::CPodcastClientAddFeedDlg(CPodcastModel& aPodcastModel):iPodcastModel(aPodcastModel)
{
}

CPodcastClientAddFeedDlg::~CPodcastClientAddFeedDlg()
{
}

_LIT(KURLPrefix, "http://");
void CPodcastClientAddFeedDlg::PreLayoutDynInitL()
{
		CEikEdwin* edwin = static_cast<CEikEdwin*>(ControlOrNull(EPodcastAddEditFeedDlgUrl));
		edwin->SetTextL(&KURLPrefix());
}


TBool CPodcastClientAddFeedDlg::OkToExitL(TInt aCommandId)
{
	if(aCommandId == EEikBidYes)
	{
		return ETrue;
	}
	CEikEdwin* edwin = static_cast<CEikEdwin*>(ControlOrNull(EPodcastAddEditFeedDlgUrl));

	if(edwin != NULL)
	{
		TBuf<1024> buffer;
		edwin->GetText(buffer);
		CFeedInfo* newFeedInfo = new (ELeave) CFeedInfo;
		newFeedInfo->SetUrl(buffer);
		newFeedInfo->SetTitle(iFeedInfo.Url());
		iPodcastModel.FeedEngine().AddFeed(newFeedInfo);
		iPodcastModel.FeedEngine().UpdateFeed(newFeedInfo->Uid());
	}

	return ETrue;
}
