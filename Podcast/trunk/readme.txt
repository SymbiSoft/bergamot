To build for UIQ, add the following lines to epoc32/include/variant/UIQ_3.x.hrh

#if !defined(__UIQ3_SDK__)
#define __UIQ3_SDK__
#endif

The S60 version is built with the FP1 SDK. To make the SIS files are more recent makesis is needed. The one from the FP2 SDK works fine.