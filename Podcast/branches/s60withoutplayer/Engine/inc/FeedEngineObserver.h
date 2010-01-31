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

#ifndef FEEDENGINEOBSERVER_H_
#define FEEDENGINEOBSERVER_H_
class MFeedEngineObserver {
public:
	virtual void FeedInfoUpdated(TUint aFeedUid) = 0;
	virtual void FeedDownloadUpdatedL(TUint aFeedUid, TInt aPercentOfCurrentDownload) = 0;
	virtual void FeedUpdateCompleteL(TUint aFeedUid) = 0;
	virtual void FeedUpdateAllCompleteL() = 0;
	virtual void FeedSearchResultsUpdated() = 0;
};
#endif /*FEEDENGINEOBSERVER_H_*/
