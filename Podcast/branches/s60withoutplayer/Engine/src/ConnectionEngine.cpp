/*
 * Copyright (c) 2007-2010 Sebastian Brannstrom, Lars Persson, EmbedDev AB
 *
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of the License "Eclipse Public License v1.0"
 * which accompanies this distribution, and is available
 * at the URL "http://www.eclipse.org/legal/epl-v10.html".
 *
 * Initial Contributors:
 * EmbedDev AB - initial contribution.
 *
 * Contributors:
 *
 * Description:
 *
 */

#include "connectionengine.h"
#include "settingsengine.h"
#include "podcastmodel.h"

CConnectionEngine* CConnectionEngine::NewL(CPodcastModel& aPodcastModel)
	{
	CConnectionEngine* self = new (ELeave) CConnectionEngine(aPodcastModel);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

CConnectionEngine::~CConnectionEngine()
	{
	delete iMobility;

	Cancel();
	
	iConnection.Close();

	iSocketServer.Close();
	
	iObserverArray.Close();
	}

CConnectionEngine::CConnectionEngine(CPodcastModel& aPodcastModel):
					CActive(CActive::EPriorityStandard),iPodcastModel(aPodcastModel)
	{
	CActiveScheduler::Add(this);	
	}

void CConnectionEngine::ConstructL()
	{
	User::LeaveIfError(iSocketServer.Connect());

	}

void CConnectionEngine::RunL()
	{
	switch(iConnectionType)
		{
		case EUserSelectConnection:
			{

			}break;
		case ESNAPConnection:			
		case EMobilityConnection:
			{
			if ( iStatus.Int() == KErrNone )
				{				
				iMobility = CActiveCommsMobilityApiExt::NewL( iConnection, *this );
				}
			
			}break;
		}
	iConnectionState = iStatus.Int() == KErrNone?CConnectionEngine::EConnected:CConnectionEngine::ENotConnected;
	ReportConnection( iStatus.Int() );
	}

void CConnectionEngine::DoCancel()
	{
	}

TInt CConnectionEngine::RunError( TInt /*aError*/ )
	{
	return KErrNone;
	}

void CConnectionEngine::PreferredCarrierAvailable( TAccessPointInfo aOldAPInfo,
		TAccessPointInfo aNewAPInfo,
		TBool aIsUpgrade,
		TBool aIsSeamless )
	{   
	if ( aIsUpgrade )
		{        
		}
	else
		{       
		}

	if ( aIsSeamless )
		{
		// in S60 3.2, this situation cannot occur.        
		}
	else
		{       
		// Sockets have to be closed at this point.        

		iMobility->MigrateToPreferredCarrier();
		}

	}

void CConnectionEngine::NewCarrierActive( TAccessPointInfo aNewAPInfo, TBool aIsSeamless )
	{    
	if ( aIsSeamless )
		{
		// in S60 3.2, this situation cannot occur.

		}
	else
		{        
		// Sockets have to be re-opened and check they can connect
		// to their server at this point.        

		iMobility->NewCarrierAccepted();
		}
	}

void CConnectionEngine::Error( TInt /*aError*/ )
	{

	}

TBool CConnectionEngine::ConnectionSettingL()
	{
	TBool selected( EFalse );

	CCmApplicationSettingsUi* settings = CCmApplicationSettingsUi::NewL();
	CleanupStack::PushL( settings );

	TUint listedItems = CMManager::EShowAlwaysAsk |
	CMManager::EShowDefaultConnection |
	CMManager::EShowDestinations |
	CMManager::EShowConnectionMethods;

	TBearerFilterArray filter;

	selected = settings->RunApplicationSettingsL( iUserSelection,
			listedItems,
			filter );

	CleanupStack::PopAndDestroy( settings );

	return selected;
	}


void CConnectionEngine::StartL(TConnectionType aConnectionType)
	{
	DP1("CConnectionEngine::StartL, aConnectionType=%d", aConnectionType);
	iConnection.Close();
	User::LeaveIfError( iConnection.Open( iSocketServer ) );
	// Connect using UI Setting
	if(aConnectionType == EUserSelectConnection)
		{				
		TBool selected = ConnectionSettingL();

		if ( selected )
			{
			switch ( iUserSelection.iResult )
				{
				case CMManager::EDestination:
					{					
					iSnapPreference.SetSnap( iUserSelection.iId );
					iConnection.Start( iSnapPreference, iStatus );
					aConnectionType = ESNAPConnection;
					break;
					}
				case CMManager::EConnectionMethod:
					{					
					iCommDBPreference.SetIapId( iUserSelection.iId );
					iCommDBPreference.SetDialogPreference( ECommDbDialogPrefDoNotPrompt );					
					iConnection.Start( iCommDBPreference, iStatus );
					break;
					}
				case CMManager::EDefaultConnection:
					{					
					iConnection.Start( iStatus );
					break;
					}
				default: // CMManager::EAlwaysAsk
					{					
					iCommDBPreference.SetDialogPreference( ECommDbDialogPrefPrompt );					
					iConnection.Start( iCommDBPreference, iStatus );
					}
				}
			SetActive();
			}
		}
	// Connect using SNAP
	else if (aConnectionType == ESNAPConnection)
		{	
		iSnapPreference.SetSnap(iPodcastModel.SettingsEngine().SpecificIAP());
		iConnection.Start( iSnapPreference, iStatus );
		SetActive();
		}
	// Connect using mobility UI
	else
		{
		iSnapPreference.SetSnap(iPodcastModel.SettingsEngine().SpecificIAP());
		iConnection.Start( iSnapPreference, iStatus );
		SetActive();	
		}		
	
	iConnectionType = aConnectionType;
	}

RConnection& CConnectionEngine::Connection()
	{
	return iConnection;	
	}

CConnectionEngine::TConnectionState CConnectionEngine::ConnectionState()
	{
	return iConnectionState;
	}

void CConnectionEngine::AddObserver(MConnectionObserver* aObserver)
	{
	iObserverArray.Append(aObserver);
	}

RSocketServ& CConnectionEngine::SockServ()
	{
	return iSocketServer;
	}


void CConnectionEngine::ReportConnection(TInt aError)
	{
	TInt noObservers = iObserverArray.Count();
	while(noObservers)
		{
		noObservers--;
		iObserverArray[noObservers]->ConnectCompleteL(aError);
		}
	}
