# Makefile fuer Pro*C/Pro*C++
# Datum:	31.03.2025
# Autor:	Harm Knolle
# Version: 	2.0
#
# Beschreibung:
# 1. Pro*C/Pro*C++ Source-Dateien werden vom 
#    Praeprozessor bearbeitet (*.pc->c, *.pcpp->cpp)
# 2. C/C++ Source-Dateien werden unter Einbindung 
#    der Instantclient-Header kompiliert
# 3. Objekt-Dateien werden unter Einbindung der 
#    Instantclient-Librarys gebunden
#

#
# Verzeichnis des Instantclients z.B. auf inflab.inf.h-brs.de
#

INSTANTCLIENTPATH=/usr/local/instantclient_21_7

#
# Das ist der Name der ausfuehrbaren Datei die erzeugt wird
#

BINFILE=dbt_ueb3

#
# Hier die benoetigten Librarys nachtragen. Pauschal ist alles
# mit dabei, dann funktioniert alles, ist aber mehr Overhead.
#

LIBS=-lnnz21 -locci -lclntsh -lociei -locijdbc21

#
# ----- ab hier m�glichst kene Aenderungen vornehmen -----
#

PROC=$(INSTANTCLIENTPATH)/sdk/proc
LD_LIBRARY_PATH=$(INSTANTCLIENTPATH) 
PROCINCLUDE=$(INSTANTCLIENTPATH)/sdk/include

BUILDDIR=build
PCFILES = $(wildcard *.pc)
PCXXFILES = $(wildcard *.pcpp)

CFILES = $(addsuffix .c, $(basename $(PCFILES)))
CXXFILES = $(addsuffix .cpp, $(basename $(PCXXFILES)))
OBJFILES = $(addprefix $(BUILDDIR)/,$(addsuffix .o, $(basename $(PCXXFILES))) $(addsuffix .o, $(basename $(PCFILES))))

LD=g++
CC=gcc
CXX=g++
LDFLAGS=-Wall -Wextra -g -L$(INSTANTCLIENTPATH)/ $(LIBS)
CFLAGS=-Wall -Wextra -g -I$(PROCINCLUDE) -fPIC
CXXFLAGS=-Wall -Wextra -g -I$(PROCINCLUDE) -D_GLIBCXX_DEBUG

run : $(BINFILE)
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) ./$(BINFILE)

all : $(BINFILE)

$(BINFILE) : $(OBJFILES)
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) $(LD) -o $@ $^ $(LDFLAGS)

build :
	mkdir $@

build/%.o : %.c build
	$(CC) -c -w -o $@ $< $(CFLAGS)

build/%.o : %.cpp build
	$(CXX) -c -w -o $@ $< $(CFLAGS)

%.c : %.pc
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) $(PROC) CODE=ANSI_C CPP_SUFFIX=c $< 

%.cpp : %.pcpp
	$(PROC) CODE=CPP CPP_SUFFIX=cpp $< 

.PRECIOUS : $(CXXFILES) $(CFILES) $(OBJFILES)
.PHONY : run clean
clean :
	rm -f $(OBJFILES) $(CFILES) $(CXXFILES) $(BINFILE)	
