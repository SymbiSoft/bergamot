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

CConnectionEngine* CConnectionEngine::NewL()
	{
	CConnectionEngine* self = new (ELeave) CConnectionEngine();
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
	}

CConnectionEngine::CConnectionEngine():CActive(CActive::EPriorityStandard)
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
			{

			}break;
		case EMobilityConnection:
			{
			if ( iStatus.Int() == KErrNone )
				{				
				iMobility = CActiveCommsMobilityApiExt::NewL( iConnection, *this );
				}
			}break;
		}
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
	iConnection.Close();
	
	// Connect using UI Setting
	if(aConnectionType == EUserSelectConnection)
		{		
		User::LeaveIfError( iConnection.Open( iSocketServer ) );
		TBool selected = ConnectionSettingL();

		if ( selected )
			{
			switch ( iUserSelection.iResult )
				{
				case CMManager::EDestination:
					{					
					iSnapPreference.SetSnap( iUserSelection.iId );
					iConnection.Start( iSnapPreference, iStatus );
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
		iConnection.Start( iSnapPreference, iStatus );
		SetActive();
		}
	// Connect using mobility UI
	else
		{
		iConnection.Start( iSnapPreference, iStatus );
		SetActive();	
		}		
	
	iConnectionType = aConnectionType;
	}


