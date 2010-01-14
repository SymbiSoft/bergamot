#include "PodcastClientAppUi.h"
#include "PodcastClientBaseView.h"
#include "PodcastClientFeedView.h"
#include "PodcastClientShowsView.h"
#include "PodcastClientPlayView.h"
#include "PodcastModel.h"
#include <qikcommand.h>
#include "ShowEngine.h"
#include <PodcastClient.rsg>
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
void CPodcastClientAppUi::ConstructL()
{
  CQikAppUi::ConstructL();
  iPodcastModel = CPodcastModel::NewL();

  iBaseView= CPodcastClientBaseView::NewLC(*this, *iPodcastModel);
  AddViewL(*iBaseView);
  CleanupStack::Pop(iBaseView);
  
  iFeedView = CPodcastClientFeedView::NewLC(*this, *iPodcastModel);
  AddViewL(*iFeedView);
  CleanupStack::Pop(iFeedView);

  iShowsView = CPodcastClientShowsView::NewLC(*this, *iPodcastModel);
  AddViewL(*iShowsView);
  CleanupStack::Pop(iShowsView);

  iPlayView = CPodcastClientPlayView::NewLC(*this, *iPodcastModel);
  AddViewL(*iPlayView);
  CleanupStack::Pop(iPlayView);
}

/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
CPodcastClientAppUi::~CPodcastClientAppUi()
{	
	// we shall destruct in reverse order
	if(iPlayView)
		{
		RemoveView(*iPlayView);
		delete iPlayView;
		}
	
	if(iShowsView)
		{
		RemoveView(*iShowsView);
		delete iShowsView;
		}
	
	if(iFeedView)
		{
		RemoveView(*iFeedView);
		delete iFeedView;
		}
	
	if(iBaseView)
		{
		RemoveView(*iBaseView); 
		delete iBaseView;
		}
	
	delete iPodcastModel;
}

/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
void CPodcastClientAppUi::HandleCommandL(CQikCommand &aCommand)
{
  CQikAppUi::HandleCommandL(aCommand);
}

