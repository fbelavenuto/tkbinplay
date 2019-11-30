# Makefile

CC = gcc
LD = gcc
RM = rm -f
MD = mkdir
SREC = srec_cat

SYS=apple2

AAS=ca65
ALD=ld65

SDIR = src
ODIR = obj
IDIR = inc
ALDIR = a2lib
AIDIR = a2lib/inc

CFLAGS = -g -Wall -I$(IDIR)
LDFLAGS = -lm
ACFLAGS  = -t $(SYS) -O -I$(AIDIR)
AAFLAGS  = -t $(SYS) -l $(ODIR)/$(*).lst -I$(AIDIR)
ALDFLAGS = -C $(ALDIR)/_config

UTILOBJ = main.o wav.o tk2000.o ini.o functions.o
OBJS = $(addprefix $(ODIR)/, $(UTILOBJ))

ASMOBJ = cr.o
AOBJS = $(addprefix $(ODIR)/, $(ASMOBJ))

all: $(ODIR) $(IDIR)/tk2000_cr.h tkbinplay

tkbinplay: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@

$(IDIR)/tk2000_cr.h: $(AOBJS)
	$(ALD) $(ALDFLAGS) -o $(ODIR)/cr#060300 $^
	$(SREC) $(ODIR)/cr#060300 -bin -o $(IDIR)/tk2000_cr.h -C-Array tk2000_cr

$(ODIR):
	$(MD) $(ODIR)

.PHONY: clean tests

clean:
	$(RM) *.exe tkbinplay $(ODIR)/*

tests: tkbinplay
	make -C tests


$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(ODIR)/%.o: $(SDIR)/%.a65
	$(AAS) $(AAFLAGS) -o $@ $<
