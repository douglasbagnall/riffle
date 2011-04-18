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
all::	salsa20_12.so
all::	salsa20_8.so
all::	snow2.so
all::	trivium.so

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

SOSEMANUK_dir = sosemanuk-clean
$(SOSEMANUK_dir)/sosemanuk.o: $(SOSEMANUK_dir)/sosemanuk.c
	$(CC)  -MD $(ALL_CFLAGS)  -fvisibility=hidden  $(CPPFLAGS) -c -o $@ $<

sosemanuk.so: sosemanuk.o $(SOSEMANUK_dir)/sosemanuk.o sha1.o
	$(CC) -fPIC -pthread -shared -Wl,-O1 -o $@ $+

isaac64.so: isaac64.o sha1.o ccan/isaac/isaac64.o
	$(CC) -fPIC -pthread -shared -Wl,-O1 -o $@ $+

isaac.so: isaac.o sha1.o ccan/isaac/isaac.o
	$(CC) -fPIC -pthread -shared -Wl,-O1 -o $@ $+

hc128.so: hc128.o  sha1.o
	$(CC) -fPIC -pthread -shared -Wl,-O1 -o $@ $+

TPY6_INCLUDES = -Itpy6 -Iinclude

tpy6/tpy6.o: tpy6/tpy6.c
	$(CC)  $(TPY6_INCLUDES) -fno-strict-aliasing  -MD $(ALL_CFLAGS)  -fvisibility=hidden  $(CPPFLAGS) -c -o $@ $<

tpy6.o: ecrypt_generic.c
	$(CC) $(TPY6_INCLUDES)  -c -MD $(ALL_CFLAGS) $(CPPFLAGS) -DMODULE_NAME=tpy6 -o $@ $<

tpy6.so: tpy6/tpy6.o tpy6.o sha1.o
	$(CC) -fPIC -pthread -shared -Wl,-O1 -o $@ $+

SALSA20_12_DIR = salsa20_12_regs
SALSA20_12_INCLUDES =  -Iinclude -I$(SALSA20_12_DIR)

$(SALSA20_12_DIR)/salsa20.o: $(SALSA20_12_DIR)/salsa20.c
	$(CC)  $(SALSA20_12_INCLUDES) -fno-strict-aliasing  -MD $(ALL_CFLAGS)  -fvisibility=hidden  $(CPPFLAGS) -c -o $@ $<

salsa20_12.o: ecrypt_generic.c
	$(CC) $(SALSA20_12_INCLUDES)  -c -MD $(ALL_CFLAGS) $(CPPFLAGS) -DMODULE_NAME=salsa20_12  -o $@ $<

salsa20_12.so: $(SALSA20_12_DIR)/salsa20.o salsa20_12.o sha1.o
	$(CC) -fPIC -pthread -shared -Wl,-O1 -o $@ $+


SALSA20_8_DIR = salsa20_8_regs
SALSA20_8_INCLUDES =  -Iinclude -I$(SALSA20_8_DIR)

$(SALSA20_8_DIR)/salsa20.o: $(SALSA20_8_DIR)/salsa20.c
	$(CC)  $(SALSA20_8_INCLUDES) -fno-strict-aliasing  -MD $(ALL_CFLAGS)  -fvisibility=hidden  $(CPPFLAGS) -c -o $@ $<

salsa20_8.o: ecrypt_generic.c
	$(CC) $(SALSA20_8_INCLUDES)  -c -MD $(ALL_CFLAGS) $(CPPFLAGS) -DMODULE_NAME=salsa20_8  -o $@ $<

salsa20_8.so: $(SALSA20_8_DIR)/salsa20.o salsa20_8.o sha1.o
	$(CC) -fPIC -pthread -shared -Wl,-O1 -o $@ $+

TRIVIUM_INCLUDES =  -Iinclude -Itrivium

trivium/trivium.o: trivium/trivium.c
	$(CC)  $(TRIVIUM_INCLUDES) -fno-strict-aliasing  -MD $(ALL_CFLAGS)  -fvisibility=hidden  $(CPPFLAGS) -c -o $@ $<

trivium.o: ecrypt_generic.c
	$(CC) $(TRIVIUM_INCLUDES)  -c -MD $(ALL_CFLAGS) $(CPPFLAGS) -DMODULE_NAME=trivium \
	-DKEY_BYTES='(80/8)' -DIV_BYTES='(64/8)' -o $@ $<

trivium.so: trivium/trivium.o trivium.o sha1.o
	$(CC) -fPIC -pthread -shared -Wl,-O1 -o $@ $+

GRAIN128_DIR = grain128_opt
GRAIN128_INCLUDES =  -Iinclude -I$(GRAIN128_DIR)

$(GRAIN128_DIR)/grain128.o: $(GRAIN128_DIR)/grain-128.c
	$(CC)  $(GRAIN128_INCLUDES) -fno-strict-aliasing  -MD $(ALL_CFLAGS)  -fvisibility=hidden  $(CPPFLAGS) -c -o $@ $<

grain128.o: ecrypt_generic.c
	$(CC) $(GRAIN128_INCLUDES)  -c -MD $(ALL_CFLAGS) $(CPPFLAGS) -DMODULE_NAME=grain128 \
	-DKEY_BYTES='(128/8)' -DIV_BYTES='(96/8)' -o $@ $<

grain128.so: $(GRAIN128_DIR)/grain128.o grain128.o sha1.o
	$(CC) -fPIC -pthread -shared -Wl,-O1 -o $@ $+


##grain 
GRAIN_DIR = grain_opt
GRAIN_INCLUDES =  -Iinclude -I$(GRAIN_DIR)

$(GRAIN_DIR)/grain.o: $(GRAIN_DIR)/grain-v1.c
	$(CC)  $(GRAIN_INCLUDES) -fno-strict-aliasing  -MD $(ALL_CFLAGS)  -fvisibility=hidden  $(CPPFLAGS) -c -o $@ $<

grain.o: ecrypt_generic.c
	$(CC) $(GRAIN_INCLUDES)  -c -MD $(ALL_CFLAGS) $(CPPFLAGS) -DMODULE_NAME=grain \
	-DKEY_BYTES='(128/8)' -DIV_BYTES='(96/8)' -o $@ $<

grain.so: $(GRAIN_DIR)/grain.o grain.o sha1.o
	$(CC) -fPIC -pthread -shared -Wl,-O1 -o $@ $+



SNOW2_INCLUDES =  -Iinclude -Isnow2

snow2/snow-2.0.o: snow2/snow-2.0.c
	$(CC)  $(SNOW2_INCLUDES) -fno-strict-aliasing  -MD $(ALL_CFLAGS)  -fvisibility=hidden  $(CPPFLAGS) -c -o $@ $<

snow2.o: ecrypt_generic.c
	$(CC) $(SNOW2_INCLUDES)  -c -MD $(ALL_CFLAGS) $(CPPFLAGS) -DMODULE_NAME=snow2  -o $@ $<

snow2.so: snow2/snow-2.0.o snow2.o sha1.o
	$(CC) -fPIC -pthread -shared -Wl,-O1 -o $@ $+
