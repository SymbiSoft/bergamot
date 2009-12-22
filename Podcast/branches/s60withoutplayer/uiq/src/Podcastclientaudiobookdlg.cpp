#include <eikedwin.h>
#include <eiklabel.h>

#include "PodcastClient.hrh"
#include "PodcastClientAudioBookDlg.h"
#include "PodcastModel.h"
#include "FeedEngine.h"
#include "debug.h"
#include "APGCLI.H"
#include "QikMediaFileFolderUtils.h"

const TInt KNumberOfFilesMaxLength = 4;

CPodcastClientAudioBookDlg::CPodcastClientAudioBookDlg(CPodcastModel& aPodcastModel, CDesCArrayFlat& aSelectedFileNames, TBool aPlayList):iPodcastModel(aPodcastModel),iSelectedFilenames(aSelectedFileNames),iPlayList(aPlayList)
	{
	}

CPodcastClientAudioBookDlg::~CPodcastClientAudioBookDlg()
	{
	}

TDataType CPodcastClientAudioBookDlg::GetMimeTypeL(const TDesC& fileName)
{
	TDataType type;
	RFile File;
	User::LeaveIfError(File.Open(CEikonEnv::Static()->FsSession(), fileName, EFileShareExclusive|EFileRead));

	TBuf8<512> fileBuf;
	File.Read(0, fileBuf);

	File.Close();

	RApaLsSession ls;
	if (ls.Connect() == KErrNone)
		{
		TDataRecognitionResult rr;
		ls.RecognizeData(fileName, fileBuf, rr);

		if (rr.iConfidence != CApaDataRecognizerType::ENotRecognized)
			type = rr.iDataType;
		ls.Close();
		}

	return type;
}

TBool CPodcastClientAudioBookDlg::OkToExitL(TInt aCommandId)
	{
	if(aCommandId == EEikBidOk)
		{
			CEikEdwin* titleEdwin = static_cast<CEikEdwin*>(ControlOrNull(EPodcastNewAudioBookTitle));
			HBufC* title = titleEdwin->GetTextInHBufL();
			CleanupStack::PushL(title);
			if (iPlayList) {
				iPodcastModel.FeedEngine().ImportBookL(*title, iSelectedFilenames[0]);
			} else {
				iPodcastModel.FeedEngine().AddBookL(*title, &iSelectedFilenames);
			}
		
			CleanupStack::PopAndDestroy(title);
		}

	return ETrue;
	}

void CPodcastClientAudioBookDlg::PreLayoutDynInitL()
	{
	 CEikLabel* label = static_cast<CEikLabel*>(ControlOrNull(EPodcastNewAudioBookLabel));
	 HBufC* buf = HBufC::NewLC(label->Text().Length()+KNumberOfFilesMaxLength);
	 buf->Des().Format(label->Text(), iSelectedFilenames.Count());
	 if (iPlayList) {
	 	label->MakeVisible(EFalse);
	 } else {
	 	label->SetText(*buf);
	 }
	 CleanupStack::PopAndDestroy(buf);
	}
