ifeq (WINS,$(findstring WINS, $(PLATFORM)))
ZDIR=$(EPOCROOT)epoc32\release\$(PLATFORM)\$(CFG)\Z
else
ZDIR=$(EPOCROOT)epoc32\data\z
endif

TARGETDIR=$(ZDIR)\resource\apps
ICONTARGETFILENAME=$(TARGETDIR)\Podcastsvg.mif
HEADERFILE=$(EPOCROOT)epoc32\include\Podcastsvg.mbg

do_nothing :
	@rem do_nothing

MAKMAKE : do_nothing

BLD : do_nothing

CLEAN : do_nothing

LIB : do_nothing

CLEANLIB : do_nothing

RESOURCE :	
		mifconv $(ICONTARGETFILENAME) /h$(HEADERFILE) /c16,1 .\podcast.svg
		
FREEZE : do_nothing

SAVESPACE : do_nothing

RELEASABLES :
	@echo $(ICONTARGETFILENAME)

FINAL : do_nothing

