# Makefile

CC = gcc
LD = gcc
CP = cp
RM = rm -f
MD = mkdir
SREC = srec_cat

SYS=apple2

AAS=ca65
ALD=ld65

SDIR = src
ODIR = obj
IDIR = inc
ALDIR = ../apple2lib
AIDIR = $(ALDIR)/inc

CFLAGS = -g -Wall -I$(IDIR) -I../libct2/inc
LDFLAGS = -lm
ACFLAGS  = -t $(SYS) -O -I$(AIDIR)
AAFLAGS  = -t $(SYS) -l $(ODIR)/$(*).lst -I$(AIDIR)
ALDFLAGS = -C $(ALDIR)/_config

UTILOBJ = main.o tk2000.o ini.o functions.o
OBJS = $(addprefix $(ODIR)/, $(UTILOBJ))

ASMOBJ = cr.o
AOBJS = $(addprefix $(ODIR)/, $(ASMOBJ))

all: $(ODIR) $(IDIR)/tk2000_cr.h tkbinplay

tkbinplay: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ ../libct2/libct2.a

$(IDIR)/tk2000_cr.h: $(AOBJS)
	$(ALD) $(ALDFLAGS) -o $(ODIR)/cr#060300 $^
	$(SREC) $(ODIR)/cr#060300 -bin -o $(IDIR)/tk2kCr.h -C-Array tk2kCr

$(ODIR):
	$(MD) $(ODIR)

.PHONY: clean install tests

clean:
	$(RM) *.exe tkbinplay $(ODIR)/* tests/*.wav *.wav

install: tkbinplay
	$(CP) $< /usr/local/bin

tests: tkbinplay
	make -C tests


$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(ODIR)/%.o: $(SDIR)/%.a65
	$(AAS) $(AAFLAGS) -o $@ $<
