#ifndef METADATAREADER_H_
#define METADATAREADER_H_

#include <e32base.h>
#include <mdaaudiosampleplayer.h>
#include "ShowInfo.h"
#include "MetaDataReaderObserver.h"

class CCnvCharacterSetConverter;
#define KMetaDataBufferLength  1024
class CMetaDataReader : public CBase, public MMdaAudioPlayerCallback
{
public:
	CMetaDataReader(MMetaDataReaderObserver &aObserver);
	void SubmitShowL(CShowInfo *aShowInfo);
	virtual ~CMetaDataReader();
	void ConstructL();
	void SetIgnoreTrackNo(TBool aIgnoreTrackNo);
	
protected:
	void MapcPlayComplete(TInt aError);
	void MapcInitComplete(TInt aError, const TTimeIntervalMicroSeconds &aDuration);
	static TInt ParseNextShowCallbackL(TAny* aMetaDataReader);	
private:
	void ParseNextShowL();
	
private:
	CMdaAudioPlayerUtility *iPlayer;
	RShowInfoArray iShowsToParse;
	CShowInfo *iShow;
	MMetaDataReaderObserver &iObserver;
	TBuf8<KMaxFileName> iTempFileName;	
	TBuf8<KMetaDataBufferLength> iTempDataBuffer;
	TBuf<KMetaDataBufferLength> iStringBuffer;
	TBuf<KMetaDataBufferLength> iStringBuffer2;
	CAsyncCallBack* iParseNextShowCallBack;
	CCnvCharacterSetConverter* iCharConverter;
	TUint iLastConverterCharset;
	TBool iIgnoreTrackNo;
	RFs &iFs;
};

#endif /*METADATAREADER_H_*/
