# Copyright (C) 2011 Douglas Bagnall
#
# Alternate pseudo-random number generators for Python.
#
# The various PRNGs themselves were written by other people.
#
# This code is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# It may be distributed under the same terms as Python itself.

all::


GDB_ALWAYS_FLAGS = -g
WARNINGS = -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers

PY_VERSION = $(shell python3 -c 'import sys; print(sys.version[:3])')
PYTHON = python$(PY_VERSION)

PY_FLAGS =  -pthread -c  -DNDEBUG  -fwrapv -Wstrict-prototypes  -I/usr/include/$(PYTHON) -fPIC
#-DPy_BUILD_CORE

ALL_CFLAGS = -march=native -O3 -g $(PY_FLAGS) $(VECTOR_FLAGS) $(WARNINGS) -pipe  -D_GNU_SOURCE -std=gnu99 $(CFLAGS) -DDSFMT_MEXP=19937 -DHAVE_SSE2 -I.

clean:
	rm -f *.so *.[oadsi] dSFMT/*.[do]

dist-clean: clean
	rm ccan/configurator config.h

.c.o:
	$(CC)  -c -MD $(ALL_CFLAGS) $(CPPFLAGS) -o $@ $<

%.so:	%.o
	$(CC) -fPIC -pthread -shared -Wl,-O1 -o $@ $+

%.s:	%.c
	$(CC)  -S  $(ALL_CFLAGS) $(CPPFLAGS) -o $@ $<

%.i:	%.c
	$(CC)  -E  $(ALL_CFLAGS) $(CPPFLAGS) -o $@ $<

.PHONY: TAGS all rsync clean debug

debug:
	make -B CFLAGS='-g -fno-inline -fno-inline-functions -fno-omit-frame-pointer -O0'

all::	mt19937module.so
all::	dSFMT.so
all::	lcg.so
all::	dummyc.so
all::	isaac64.so
all::	isaac.so
all::	sosemanuk.so
all::	hc128.so
all::	tpy6.so

ccan/configurator:
	$(CC) $@.c -o $@

config.h: ccan/configurator
	ccan/configurator > $@

isaac64.o isaac.o: config.h

DSFMT_FLAGS =  -finline-functions -fomit-frame-pointer -DNDEBUG -fno-strict-aliasing --param max-inline-insns-single=1800  -Wmissing-prototypes  -std=c99

dSFMT/dSFMT.o: dSFMT/dSFMT.c
	$(CC)  $(DSFMT_FLAGS)  -MD $(ALL_CFLAGS)  -fvisibility=hidden  $(CPPFLAGS) -c -o $@ $<

dSFMT.so: dSFMT/dSFMT.o dSFMT.o
	$(CC) -fPIC -pthread -shared -Wl,-O1 -o $@ $+

sosemanuk/sosemanuk.o: sosemanuk/sosemanuk.c
	$(CC)  -MD $(ALL_CFLAGS)  -fvisibility=hidden  $(CPPFLAGS) -c -o $@ $<

sosemanuk.so: sosemanuk.o sosemanuk/sosemanuk.o sha1.o
	$(CC) -fPIC -pthread -shared -Wl,-O1 -o $@ $+

isaac64.so: isaac64.o sha1.o ccan/isaac/isaac64.o
	$(CC) -fPIC -pthread -shared -Wl,-O1 -o $@ $+

isaac.so: isaac.o sha1.o ccan/isaac/isaac.o
	$(CC) -fPIC -pthread -shared -Wl,-O1 -o $@ $+

hc128.so: hc128.o  sha1.o
	$(CC) -fPIC -pthread -shared -Wl,-O1 -o $@ $+

TPY6_INCLUDES = -Itpy6 -Iinclude

estream-tpy6.o: tpy6/tpy6.c
	$(CC)  $(TPY6_INCLUDES) -fno-strict-aliasing  -MD $(ALL_CFLAGS)  -fvisibility=hidden  $(CPPFLAGS) -c -o $@ $<

tpy6.o: tpy6.c
	$(CC) $(TPY6_INCLUDES)  -c -MD $(ALL_CFLAGS) $(CPPFLAGS) -o $@ $<

tpy6.so: estream-tpy6.o tpy6.o sha1.o
	$(CC) -fPIC -pthread -shared -Wl,-O1 -o $@ $+


SALSA20_12_INCLUDES =  -Iinclude -Isalsa20_12_regs

salsa20_12_regs/salsa20.o: salsa20_12_regs/salsa20.c
	$(CC)  $(SALSA20_12_INCLUDES) -fno-strict-aliasing  -MD $(ALL_CFLAGS)  -fvisibility=hidden  $(CPPFLAGS) -c -o $@ $<

salsa20_12.o: salsa20_12.c
	$(CC) $(SALSA20_12_INCLUDES)  -c -MD $(ALL_CFLAGS) $(CPPFLAGS) -o $@ $<

salsa20_12.so: salsa20_12_regs/salsa20.o salsa20_12.o sha1.o
	$(CC) -fPIC -pthread -shared -Wl,-O1 -o $@ $+
