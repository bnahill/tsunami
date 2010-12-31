#
#	Sample Makefile for C30 based projects
#	Version 0.1
#
# (C) 2010 Matt Jervis <dev@electricrock.co.nz>
# http://www.electricrock.co.nz/blog
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 3 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
#
# Use at your own risk and make sure you backup important files first!!!!
#

# The name of the file you want to output
TARGET=tsunami.hex
COF=$(patsubst %.hex, %.cof, $(TARGET))
# Part number
PART=30f4011
#PART=33fj256gp710

# Uncomment the appropriate family
PARTFAMILY=dsPIC30F
#PARTFAMILY=dsPIC33F
#PARTFAMILY=pic24H
#PARTFAMILY=pic24L

GCC=pic30-coff-gcc
AS=pic30-coff-as
LD=pic30-coff-ld
B2H=pic30-coff-bin2hex

# The path where you installed C30 (e.g. /opt/c30)
C30PATH=/usr/local

DEMO_DIR = FreeRTOSV6.1.0/Demo/Common
SOURCE_DIR = FreeRTOSV6.1.0/Source
PORT_DIR = $(SOURCE_DIR)/portable/MPLAB/PIC24_dsPIC

# This assumes all sources are in the current directory
# Add paths to other directories if necessary, or manually specify sources
SRCS=$(wildcard *.c) \
     serial/serial.c \
     ParTest/ParTest.c \
	 $(DEMO_DIR)/Full/flash.c \
	 $(DEMO_DIR)/Full/print.c \
	 $(DEMO_DIR)/Minimal/integer.c \
	 $(DEMO_DIR)/Minimal/BlockQ.c \
	 $(DEMO_DIR)/Minimal/blocktim.c \
	 $(DEMO_DIR)/Minimal/comtest.c \
	 $(DEMO_DIR)/Full/semtest.c \
	 $(SOURCE_DIR)/tasks.c \
	 $(SOURCE_DIR)/queue.c \
	 $(SOURCE_DIR)/croutine.c \
	 $(SOURCE_DIR)/list.c \
	 $(SOURCE_DIR)/portable/MemMang/heap_1.c \
	 $(PORT_DIR)/port.c

# Add more include paths as required by your project
INCLUDEPATH=-I.\
			-I$(DEMO_DIR)/include \
			-I$(SOURCE_DIR)/include

#CFLAGS=-g -O2
# For some reason -O anything freezes shit
CFLAGS=-fno-omit-frame-pointer -g -Wall -DMPLAB_DSPIC_PORT -O2 -fno-schedule-insns -fno-schedule-insns2

# Comment out the second lib here if you don't need the periphal libraries
LIBS=	-lpic30 \
		-lc \
		-lm \
		-lq \
		-lp$(subst f,F,$(PART))
#		-lp33FJ256GP710 \

#
#
# The rest shouldn't need editing
#
# ----------------------------------------------------------------------------
#
LIBPATH=-L$(C30PATH)/share/pic30-nonfree/lib \
	-L$(C30PATH)/share/pic30-nonfree/lib/$(PARTFAMILY)

LINKERSCRIPT=$(C30PATH)/share/pic30-nonfree/support/$(PARTFAMILY)/gld/p$(PART).gld
#LINKERSCRIPT=p$(PART).gld


OBJS=$(patsubst %.c, %.o, $(SRCS)) \
     $(PORT_DIR)/portasm_dsPIC.o
LSTS=$(patsubst %.c, %.lst, $(SRCS))
ASMS=$(patsubst %.c, %.s, $(SRCS))

HEADERS=$(wildcard *.h)

.PHONY: target
target: $(TARGET)

.PHONY: cof
cof: $(COF)

.PHONY: assembly
assembly: $(ASMS)

.PHONY: objects
objects: $(OBJS)

.PHONY: all
all: cof target assembly objects

.PHONY: clean
clean:
	-rm $(OBJS)
	-rm $(TARGET)
	-rm $(ASMS)
#	-rm $(LSTS)

%.s:	%.c $(HEADERS)
	$(GCC) -S $(INCLUDEPATH) -mcpu=$(PART) $(CFLAGS) -o $@ $<

%.o:	%.c $(HEADERS)
	$(GCC) -c $(INCLUDEPATH) -mcpu=$(PART) $(CFLAGS) -o $@ $<

$(PORT_DIR)/portasm_dsPIC.o: $(PORT_DIR)/portasm_dsPIC.s
	$(GCC) -c $(INCLUDEPATH) -mcpu=$(PART) $(CFLAGS) -o $@ $<

$(COF): $(OBJS)
	$(GCC) $(LIBPATH) $(LIBS) -Xlinker -T$(LINKERSCRIPT) -O2 -o $@ -Xlinker --heap=256 $^
#	$(GCC) -Xlinker $(LIBPATH) -Xlinker $(LIBS) -Xlinker -T$(LINKERSCRIPT) -o $@ -Xlinker -Map=$(patsubst %.hex, %.map, $(TARGET)) $^

$(TARGET): $(COF)
	$(B2H) $^
