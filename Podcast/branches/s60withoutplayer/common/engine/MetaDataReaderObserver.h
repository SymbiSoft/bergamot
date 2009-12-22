#ifndef METADATAREADEROBSERVER_H_
#define METADATAREADEROBSERVER_H_

class MMetaDataReaderObserver {
public:
	virtual void ReadMetaData(CShowInfo *aShowInfo) = 0;
	virtual void ReadMetaDataComplete() = 0;
};

#endif /*METADATAREADEROBSERVER_H_*/
