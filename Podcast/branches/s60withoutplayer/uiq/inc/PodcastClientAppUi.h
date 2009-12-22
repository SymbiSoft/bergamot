#ifndef __PODCASTCLIENTAPPUI_H__
#define __PODCASTCLIENTAPPUI_H__

#include <qikappui.h>
#include "PodcastClientView.h"
class CPodcastClientPlayView;
class CPodcastModel;
class CPodcastClientBaseView;
class CPodcastClientFeedView;
class CPodcastClientShowsView;

class CPodcastClientAppUi : public CQikAppUi
{
public:
  void ConstructL();
  ~CPodcastClientAppUi();

private:
	void HandleCommandL(CQikCommand &aCommand);
private:
	CPodcastClientBaseView* iBaseView;
	CPodcastClientFeedView* iFeedView;
	CPodcastClientShowsView* iShowsView;
	CPodcastClientPlayView* iPlayView;
	
	CPodcastModel* iPodcastModel;
};


#endif


