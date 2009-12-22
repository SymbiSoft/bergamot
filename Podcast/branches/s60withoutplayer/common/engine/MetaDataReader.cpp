#include "MetaDataReader.h"
#include "id3tag.h"
#include "field.h"
#include <utf.h>
#include <charconv.h>
#include <eikenv.h>
#include <mmf\common\mmfMeta.h>
#include "debug.h"

_LIT(KMP3Extension, ".MP3");
CMetaDataReader::CMetaDataReader(MMetaDataReaderObserver& aObserver) : iObserver(aObserver), iFs(CEikonEnv::Static()->FsSession())
{
	iShow = NULL;
}

CMetaDataReader::~CMetaDataReader()
{
	iPlayer->Close();
	delete iPlayer;
	delete iParseNextShowCallBack;
	iShowsToParse.Close();
	delete iCharConverter;
}

void CMetaDataReader::ConstructL()
{
	DP("ConstructL");
	iPlayer = CMdaAudioPlayerUtility::NewL(*this);

	TCallBack callback(ParseNextShowCallbackL, this);
	iParseNextShowCallBack = new (ELeave)CAsyncCallBack(callback, CActive::EPriorityStandard);
	iCharConverter = CCnvCharacterSetConverter::NewL();
	iCharConverter->PrepareToConvertToOrFromL(KCharacterSetIdentifierIso88591, iFs); 
	iLastConverterCharset = KCharacterSetIdentifierIso88591;
}

void CMetaDataReader::SubmitShowL(CShowInfo *aShowInfo)
{
	DP("SubmitShow");
	iShowsToParse.Append(aShowInfo);
	
	if (iShow == NULL) {
		ParseNextShowL();
	}
}

TInt CMetaDataReader::ParseNextShowCallbackL(TAny* aMetaDataReader)
{
	static_cast<CMetaDataReader*>(aMetaDataReader)->ParseNextShowL();
	return KErrNone;
}

void CMetaDataReader::ConvertToUniCodeL(TDes& aDestBuffer, TDes8& aInputBuffer, enum id3_field_textencoding aEncoding)
{
	switch(aEncoding)
	{
	case ID3_FIELD_TEXTENCODING_UTF_8:
		{		
			HBufC* tempBuffer = CnvUtfConverter::ConvertToUnicodeFromUtf8L(aInputBuffer);
			aDestBuffer.Copy(*tempBuffer);
			delete tempBuffer;
		}break;
	case ID3_FIELD_TEXTENCODING_UTF_16:
		{
			if(iLastConverterCharset != KCharacterSetIdentifierUnicodeBig)
			{
				iCharConverter->PrepareToConvertToOrFromL(KCharacterSetIdentifierUnicodeBig, iFs); 
				iLastConverterCharset = KCharacterSetIdentifierUnicodeBig;
			}
			TInt unconvertable = 0;
			TInt state = 0;
			if(iCharConverter->ConvertToUnicode(aDestBuffer, aInputBuffer, state, unconvertable) == KErrNone)
			{
				if(aDestBuffer.Length()>0 && (aDestBuffer[0] == CEditableText::EZeroWidthNoBreakSpace || aDestBuffer[0] == CEditableText::EReversedByteOrderMark) )
				{
				aDestBuffer = aDestBuffer.Right(aDestBuffer.Length()-1);
				}
			}
		}break;
	case ID3_FIELD_TEXTENCODING_UTF_16BE:
		{
			if(iLastConverterCharset != KCharacterSetIdentifierUnicodeLittle)
			{
				iCharConverter->PrepareToConvertToOrFromL(KCharacterSetIdentifierUnicodeLittle, iFs); 
				iLastConverterCharset = KCharacterSetIdentifierUnicodeLittle;
			}
			TInt unconvertable = 0;
			TInt state = 0;
			if(iCharConverter->ConvertToUnicode(aDestBuffer, aInputBuffer, state, unconvertable) == KErrNone)
			{
				if(aDestBuffer.Length()>0 && (aDestBuffer[0] == CEditableText::EZeroWidthNoBreakSpace || aDestBuffer[0] == CEditableText::EReversedByteOrderMark) )
				{
				aDestBuffer = aDestBuffer.Right(aDestBuffer.Length()-1);
				}
			}
		}break;
	case ID3_FIELD_TEXTENCODING_ISO_8859_1:
		{
			if(iLastConverterCharset != KCharacterSetIdentifierIso88591)
				{
				iCharConverter->PrepareToConvertToOrFromL(KCharacterSetIdentifierIso88591, iFs); 
				iLastConverterCharset = KCharacterSetIdentifierIso88591;
				}
			TInt unconvertable = 0;
			TInt state = 0;
			iCharConverter->ConvertToUnicode(aDestBuffer, aInputBuffer, state, unconvertable);
		}break;
	}

}

void CMetaDataReader::ParseNextShowL()
	{
	DP("ParseNextShow");
	iPlayer->Close();
	if (iShowsToParse.Count() == 0) {
		DP("No more shows, stopping");
		iObserver.ReadMetaDataComplete();
		return;
	}
	
	iShow = iShowsToParse[0];
	iShowsToParse.Remove(0);
	TParsePtrC parser(iShow->FileName());

	if(parser.Ext().CompareF(KMP3Extension()) == 0)
	{
		iTempFileName.Copy(iShow->FileName());
		iTempFileName.PtrZ();
		struct id3_file * id3_file = id3_file_open((const char*) iTempFileName.Ptr(), ID3_FILE_MODE_READONLY);
		
		if(id3_file != NULL)
		{
			id3_tag * tag = id3_file_tag(id3_file);
			id3_length_t len;
			id3_frame * frame = id3_tag_findframe(tag, ID3_FRAME_TITLE, 0);
			TUint cnt;
			char* ptr = NULL;
			enum id3_field_textencoding encoding = ID3_FIELD_TEXTENCODING_UTF_8;
			if(frame != NULL)
			{
				len = 0;
				for( cnt = 0;cnt<frame->nfields;cnt++)
				{
					ptr = (char*) iTempDataBuffer.Ptr();
					len = id3_field_render(&frame->fields[cnt], (id3_byte_t**)&ptr, &encoding,0);					
				}

				iTempDataBuffer.SetLength(len);
				ConvertToUniCodeL(iStringBuffer, iTempDataBuffer, encoding);
				iShow->SetTitleL(iStringBuffer);				
			}

			iStringBuffer.Zero();	
						
			frame = id3_tag_findframe(tag, ID3_FRAME_ALBUM, 0);

			if(frame != NULL)
			{
				len = 0;

				for( cnt = 0;cnt<frame->nfields;cnt++)
				{
					ptr = (char*) iTempDataBuffer.Ptr();
					len = id3_field_render(&frame->fields[cnt], (id3_byte_t**)&ptr, &encoding,0);					
				}
				
				iTempDataBuffer.SetLength(len);
				ConvertToUniCodeL(iStringBuffer, iTempDataBuffer, encoding);			
				iStringBuffer.Append(_L("\n\n"));
			}
			
			frame = id3_tag_findframe(tag, ID3_FRAME_ARTIST, 0);

			if(frame != NULL)
			{
				len = 0;

				for( cnt = 0;cnt<frame->nfields;cnt++)
				{
					ptr = (char*) iTempDataBuffer.Ptr();
					len = id3_field_render(&frame->fields[cnt], (id3_byte_t**)&ptr, &encoding,0);					
				}

				iTempDataBuffer.SetLength(len);
		
				ConvertToUniCodeL(iStringBuffer2, iTempDataBuffer, encoding);
				iStringBuffer.Append(iStringBuffer2);
			}		
					
			iShow->SetDescriptionL(iStringBuffer);	 			

			frame = id3_tag_findframe(tag, ID3_FRAME_TRACK, 0);
			if(frame != NULL)
			{
				len = 0;

				for( cnt = 0;cnt<frame->nfields;cnt++)
				{
					ptr = (char*) iTempDataBuffer.Ptr();
					len = id3_field_render(&frame->fields[cnt], (id3_byte_t**)&ptr, &encoding,0);					
				}
				iTempDataBuffer.SetLength(len);
				TLex8 lexer(iTempDataBuffer);
				TUint value = 0;
				if(lexer.Val(value) == KErrNone && !iIgnoreTrackNo) {
					iShow->SetTrackNo(value);
				}
			}
			
			id3_file_close(id3_file);
		}
		iObserver.ReadMetaData(iShow);
		iShow = NULL;
		iParseNextShowCallBack->CallBack();
	}
	else
	{
		iPlayer->OpenFileL(iShow->FileName());
	}
}

void CMetaDataReader::MapcPlayComplete(TInt /*aError*/)
{
	
}

void CMetaDataReader::MapcInitComplete(TInt aError, const TTimeIntervalMicroSeconds &aDuration)
{
	DP1("MapcInitComplete, file=%S", &(iShow->FileName()));


	if(aError == KErrNone)
	{
		TInt numEntries = 0;
		if (iPlayer->GetNumberOfMetaDataEntries(numEntries) == KErrNone) {
			DP1("%d meta data entries", numEntries);
			iShow->SetPlayTime((aDuration.Int64()/1000000));
			
			for (int i=0;i<numEntries;i++) {
				CMMFMetaDataEntry * entry;
				TRAPD(error, entry = iPlayer->GetMetaDataEntryL(i));
				
				if (error != KErrNone) {
					continue;
				}
				TBuf<1024> buf;
				if (entry->Name() == KMMFMetaEntrySongTitle) {
					buf.Copy(entry->Value());
					TRAP_IGNORE(iShow->SetTitleL(buf));
					DP1("title: %S", &(iShow->Title()));
				} else if (entry->Name() == _L("artist")) {
					if (iShow->Description().Length() > 0) {
						buf.Copy(iShow->Description());
					}
					buf.Append(_L("\n"));
					buf.Append(entry->Value());
					
					TRAP_IGNORE(iShow->SetDescriptionL(buf));
				} else if (entry->Name() == KMMFMetaEntryAlbum) {
					if (iShow->Description().Length() > 0) {
						buf.Copy(iShow->Description());
					}
					buf.Append(_L("\n"));
					buf.Append(entry->Value());
					
					TRAP_IGNORE(iShow->SetDescriptionL(buf));
				}	
				else if (entry->Name() == KMMFMetaEntryAlbumTrack) {
					TLex lexer(entry->Value());
					TUint value = 0;
					if(lexer.Val(value) == KErrNone) {
						iShow->SetTrackNo(value);
					}
				}
			}
		}
	}

	iObserver.ReadMetaData(iShow);
	iPlayer->Stop();
	iShow = NULL;
	TRAP_IGNORE(ParseNextShowL());
}

void CMetaDataReader::SetIgnoreTrackNo(TBool aIgnoreTrackNo)
	{
	iIgnoreTrackNo = aIgnoreTrackNo;
	}
