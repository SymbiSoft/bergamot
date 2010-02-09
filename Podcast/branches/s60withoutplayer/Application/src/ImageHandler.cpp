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
* Example code from OcrExample Copyright (c) 2006 Nokia Corporation.
* Description:
*
*/

#include "ImageHandler.h"

// =============================================================================
// CImageHandler, a utility class for loading images.
// =============================================================================

// ============================= MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CImageHandler::CImageHandler
// C++ constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CImageHandler::CImageHandler( RFs& aFs,
                              MImageHandlerCallback& aCallback )
    : CActive(CActive::EPriorityStandard)
    , iDecoder(NULL)
    , iScaler(NULL)
    , iCallback(aCallback)    
    , iFs(aFs)
    , iSize(0,0)
    {
    }

// -----------------------------------------------------------------------------
// CImageHandler::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CImageHandler::ConstructL()
    {
    CActiveScheduler::Add(this);
    iBitmap = new (ELeave) CFbsBitmap;
    }

// -----------------------------------------------------------------------------
// CImageHandler::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CImageHandler* CImageHandler::NewL(RFs& aFs,
                                   MImageHandlerCallback& aCallback)
    {
    CImageHandler* self = NewLC(aFs, aCallback);
    CleanupStack::Pop();
    return self;
    }

// -----------------------------------------------------------------------------
// CImageHandler::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CImageHandler* CImageHandler::NewLC(RFs& aFs,
                                    MImageHandlerCallback& aCallback)
    {
    CImageHandler* self = new (ELeave) CImageHandler(aFs,
                                                     aCallback);
    CleanupStack::PushL( self );
    self->ConstructL();

    return self;
    }

// Destructor
CImageHandler::~CImageHandler()
    {
    delete iDecoder;
    delete iScaler;
    delete iBitmap;
    }

// -----------------------------------------------------------------------------
// CImageHandler::LoadFileL
// Loads a selected frame from a named file
// -----------------------------------------------------------------------------
//
void CImageHandler::LoadFileL(const TFileName& aFileName, TInt aSelectedFrame)
    {
    __ASSERT_ALWAYS(!IsActive(),User::Invariant());
    if ( iDecoder )
        {
        delete iDecoder;
        iDecoder = NULL;
        }
   
    iDecoder = CImageDecoder::FileNewL(iFs, aFileName);
    // Get image information
    iFrameInfo = iDecoder->FrameInfo(aSelectedFrame);
    // Resize to fit.
    TRect bitmap = iFrameInfo.iFrameCoordsInPixels;

    // release possible previous image
    iBitmap->Reset();
    iBitmap->Create(bitmap.Size(), EColor16M);

    // Decode as bitmap.
    iDecoder->Convert(&iStatus, *iBitmap, aSelectedFrame);
    SetActive();
    }

void CImageHandler::LoadFileAndScaleL(CFbsBitmap* aScaledBitmap, 
								      const TFileName& aFileName,
                                      const TSize &aSize,
                                      TInt aSelectedFrame)
    {
    __ASSERT_ALWAYS(!IsActive(),User::Invariant());
    iSize = aSize;
    iScaledBitmap = aScaledBitmap;
    iScaledBitmap->Reset();
    iScaledBitmap->Create(aSize, EColor16M);
    LoadFileL(aFileName, aSelectedFrame);
    }

CFbsBitmap* CImageHandler::ScaledBitmap()
	{
	return iScaledBitmap;
	}

void CImageHandler::ScaleL()
    {
    __ASSERT_ALWAYS(!IsActive(),User::Invariant());
    if ( iScaler )
        {
        delete iScaler;
        }
    iScaler = NULL;
    iScaler = CBitmapScaler::NewL();
    iScaler->Scale(&iStatus, *iBitmap, *iScaledBitmap, ETrue);
    SetActive();
    }

// -----------------------------------------------------------------------------
// CImageHandler::FrameInfo
// Get the current frame information.
// -----------------------------------------------------------------------------
//
const TFrameInfo& CImageHandler::FrameInfo() const
    {
    return iFrameInfo;
    }

// -----------------------------------------------------------------------------
// CImageHandler::RunL
// CActive::RunL() implementation. Called on image load success/failure.
// -----------------------------------------------------------------------------
//
void CImageHandler::RunL()
    {
    if ((! iStatus.Int()) && (iSize.iWidth != 0) && (iSize.iHeight != 0))
        {
        ScaleL();
        }
    else
        {
        // Invoke callback.
        iCallback.ImageOperationCompleteL(iStatus.Int());
        }
    iSize.SetSize(0,0);
    }

// -----------------------------------------------------------------------------
// CImageHandler::DoCancel
// CActive::Cancel() implementation. Stops decoding.
// -----------------------------------------------------------------------------
//
void CImageHandler::DoCancel()
    {
    if ( iDecoder )
        {
        iDecoder->Cancel();
        }
    if ( iScaler )
        {
        iScaler->Cancel();
        }
    }



