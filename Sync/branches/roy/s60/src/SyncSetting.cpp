#include "SyncSetting.h"
#include "debug.h"
#include <SyncClientS60_0x2002C265.rsg>
#include <aknquerydialog.h>


CSyncSetting::CSyncSetting(TInt aResourceId, TUint aProfileId, RSyncServerSession &aSession) :
	CAknEnumeratedTextPopupSettingItem(aResourceId, iValue), iValue(0), iProfileId(aProfileId), iSession(aSession)
	{
	}

CSyncSetting::~CSyncSetting()
	{
	}

void CSyncSetting::CompleteConstructionL()
	{
	// Complete construction
	CAknEnumeratedTextPopupSettingItem::CompleteConstructionL();
	LoadL();
	}

void CSyncSetting::SetValue(TUint value) 
	{
	iValue = value;
	LoadL();
	}

void CSyncSetting::EditItemL(TBool aCalledFromMenu)
	{
	DP("CSyncSetting::EditItemL BEGIN");
	CAknEnumeratedTextPopupSettingItem::EditItemL(aCalledFromMenu);
	StoreL();
	DP2("External value toggled (StoreL), internal=%d, external=%d",InternalValue(), ExternalValue());
	DP("CSyncSetting::EditItemL END");
	}

// When we change value for a setting/sync-profile
// It calls RSyncServerSession 
// iSession.SetTimer(iProfileId, (TSyncServerPeriod) QueryValue()->CurrentValueIndex());
void CSyncSetting::HandleSettingPageEventL(CAknSettingPage* aSettingPage, TAknSettingPageEvent aEventType)
	{
	CAknSettingItem::HandleSettingPageEventL(aSettingPage, aEventType);
	if (aEventType == EEventSettingOked) 
		{	
		// Need to load queries for hour, minute, day, interval
		// according to selected value
		
		TSyncServerPeriod period = (TSyncServerPeriod) QueryValue()->CurrentValueIndex();
		
		TTime Today;
		Today.HomeTime();
		TDateTime TodayDateTime = Today.DateTime();
		
		TDay day = Today.DayNoInWeek();
		TInt hour = TodayDateTime.Hour();
		TInt minute = TodayDateTime.Minute();
		
		if (period == EInterval)
			{		
				// create dialog instance; number is the descriptor that is used for // the editor
				CAknNumberQueryDialog* dlg = new (ELeave) CAknNumberQueryDialog (hour);
				// Prepares the dialog, constructing it from the specified resource
				dlg->PrepareLC(R_QUERY_HOUR_INTERVAL);

				// Launch the dialog
				if (dlg->RunLD())
				{
					CAknNumberQueryDialog* dlgMins = new (ELeave) CAknNumberQueryDialog (minute);
					// Prepares the dialog, constructing it from the specified resource
					dlgMins->PrepareLC(R_QUERY_MINUTES_INTERVAL);
					
					// if user clicked cancel
					if (!dlgMins->RunLD())
					{
						return;
					}
				}
				else
				{
					return;
				}
			
			}
		else if (period == EDaily)
			{
				TTime time(_L("20000111:200600.000000"));
				
				// The prompt text for the query
				TBuf<128> prompt(_L("Enter Time:"));
				// create dialog instance; time is a reference to TTime object that is // used for the editor
				CAknTimeQueryDialog* dlg =
				CAknTimeQueryDialog::NewL( Today, CAknQueryDialog::ENoTone);
				CleanupStack::PushL(dlg);
				// set prompt text for query. This will override text given in
				// resource
				dlg->SetPromptL(prompt);
				CleanupStack::Pop(); //dlg
				
				// launch the dialog with resource
				if (dlg->ExecuteLD(R_QUERY_DAILY_TIME))
				{
					// ok pressed, time is a Ttime object containing the entered time // in the editor.
					hour = Today.DateTime().Hour();
					minute = Today.DateTime().Minute();
				}	
				else
				{
					return;
				}
			}
		else if (period == EWeekly)
			{
				// Need to choose a day, and time
				TInt index( 0 );
				CAknListQueryDialog* dlg = new( ELeave ) CAknListQueryDialog( &index );
				// launch the dialog with resource R_DEMO_MULTILINE_DATE_AND_TIME_QUERY
				if ( dlg->ExecuteLD(R_WEEKLY_DAY_QUERY) )
				{
					day = TDay(index);
					
					// The prompt text for the query
					TBuf<128> prompt(_L("Enter Time:"));
					// create dialog instance; time is a reference to TTime object that is // used for the editor
					CAknTimeQueryDialog* dlg =
					CAknTimeQueryDialog::NewL( Today, CAknQueryDialog::ENoTone);
					CleanupStack::PushL(dlg);
					// set prompt text for query. This will override text given in
					// resource
					dlg->SetPromptL(prompt);
					CleanupStack::Pop(); //dlg
					
					// launch the dialog with resource
					if (dlg->ExecuteLD(R_QUERY_DAILY_TIME))
					{
						// ok pressed, time is a Ttime object containing the entered time // in the editor.
						hour = Today.DateTime().Hour();
						minute = Today.DateTime().Minute();
					}	
					else
					{
						return;
					}
				}
				else
				{
					return;
				}
			}
		
		DP2("InternalValue=%d, ExternalValue=%d", InternalValue(), ExternalValue());
		StoreL();
		
		iSession.SetTimer(iProfileId, period, day, hour, minute);
		}
	}

TInt CSyncSetting::HandleSetting()
	{
	return ETrue; 
	}
