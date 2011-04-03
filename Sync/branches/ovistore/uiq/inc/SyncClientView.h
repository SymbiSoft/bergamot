#ifndef SYNCCLIENTVIEW_H
#define SYNCCLIENTVIEW_H

#include <QikViewBase.h>
#include <QikScrollableContainer.h>
#include "SyncClient.hrh"
#include "S32FILE.H"
#include "e32debug.h"
#include <e32cmn.h>
#include "RSyncServerSession.h"
#include "SyncClientGlobals.h"

class CSyncClientView : public CQikViewBase
	{
public:
	static CSyncClientView* NewLC(CQikAppUi& aAppUi);
	~CSyncClientView();
	TVwsViewId ViewId()const;
	void HandleCommandL(CQikCommand& aCommand);
	
protected: 
	void ViewConstructL();
	void ViewActivatedL(const TVwsViewId &aPrevViewId, TUid aCustomMessageId, const TDesC8 &aCustomMessage);
	
private:
	CSyncClientView(CQikAppUi& aAppUi);
	void ConstructL();
	void CreateChoiceListItem(CQikScrollableContainer* container, int id, TDesC16& caption, int state);
	void HandleControlEventL(CCoeControl *aControl, TCoeEvent aEventType);
	void CSyncClientView::CreateNoItemsLabel(CQikScrollableContainer* container);

public:
	void ShowSyncProfiles(CQikScrollableContainer* container);

private:
	RSyncServerSession serverSession;
	CQikScrollableContainer* iContainer;
	RArray<TSmlProfileId> lastViewProfiles;
	};
#endif
