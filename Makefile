# Makefile

CC = gcc
CPP = g++
LD = g++
CP = cp
RM = rm -f
MD = mkdir
SREC = srec_cat

SYS = apple2
AAS = ca65
ALD = ld65

SDIR = src
ODIR = obj
IDIR = inc

CFLAGS = -g -Wall -I$(IDIR) -I../libct2/inc
LDFLAGS = -lm

ALDIR = ../apple2lib
AIDIR = $(ALDIR)/inc
ACFLAGS  = -t $(SYS) -O -I$(AIDIR)
AAFLAGS  = -t $(SYS) -I$(AIDIR)
ALDFLAGS = -C $(ALDIR)/_config

SRCS = $(wildcard $(SDIR)/*.cpp)
OBJS = $(SRCS:$(SDIR)/%.cpp=$(ODIR)/%.o)

ASMCODES = $(IDIR)/tk2kCr.h $(IDIR)/A2Cr.h

all: $(ODIR) $(ASMCODES) tkbinplay

tkbinplay: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ ../libct2/libct2.a

$(IDIR)/tk2kCr.h: $(SDIR)/cr.a65
	$(AAS) -DTAPEIN=$$C010 $(AAFLAGS) -l $(ODIR)/tk2kCr.lst -o $(ODIR)/tk2kCr.o $<
	$(ALD) $(ALDFLAGS) -o $(ODIR)/tk2kCr#060300 $(ODIR)/tk2kCr.o
	$(SREC) $(ODIR)/tk2kCr#060300 -bin -o $@ -C-Array tk2kCr

$(IDIR)/A2AutoLoad.h: $(SDIR)/A2AutoLoad.a65
	$(AAS) $(AAFLAGS) -l $(ODIR)/A2AutoLoad.lst -o $(ODIR)/A2AutoLoad.o $<
	$(ALD) $(ALDFLAGS) -o $(ODIR)/A2AutoLoad#060801 $(ODIR)/A2AutoLoad.o
	$(SREC) $(ODIR)/A2AutoLoad#060801 -bin -o $@ -C-Array A2AutoLoad

$(IDIR)/A2Cr.h: $(SDIR)/cr.a65
	$(AAS) -DTAPEIN=$$C060 $(AAFLAGS) -l $(ODIR)/A2Cr.lst -o $(ODIR)/A2Cr.o $<
	$(ALD) $(ALDFLAGS) -o $(ODIR)/A2Cr#060300 $(ODIR)/A2Cr.o
	$(SREC) $(ODIR)/A2Cr#060300 -bin -o $@ -C-Array A2Cr

$(ODIR):
	$(MD) $(ODIR)

.PHONY: clean install tests

clean:
	$(RM) *.exe tkbinplay $(ODIR)/* tests/*.wav *.wav

install: tkbinplay
	$(CP) $< /usr/local/bin

tests: tkbinplay
	make -C tests

# dependencies
$(ODIR)/functions.o: $(IDIR)/functions.h
$(ODIR)/ini.o: $(IDIR)/ini.h
$(ODIR)/machine.o: $(IDIR)/machine.h
$(ODIR)/tk2k.o: $(IDIR)/machine.h $(IDIR)/tk2k.h $(IDIR)/tk2kAutoLoad.h $(IDIR)/tk2kCr.h
$(ODIR)/apple2.o: $(IDIR)/machine.h $(IDIR)/apple2.h $(IDIR)/A2AutoLoad.h $(IDIR)/A2Cr.h
$(ODIR)/main.o: $(IDIR)/ini.h $(IDIR)/machine.h $(IDIR)/tk2k.h $(IDIR)/functions.h $(IDIR)/version.h

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(ODIR)/%.o: $(SDIR)/%.cpp
	$(CPP) $(CFLAGS) -c -o $@ $<
