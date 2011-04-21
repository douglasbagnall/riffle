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

PY_FLAGS =  -pthread -DNDEBUG  -fwrapv -Wstrict-prototypes  -I/usr/include/$(PYTHON) -fPIC
#-DPy_BUILD_CORE

ALL_CFLAGS = -march=native -O3 -g $(PY_FLAGS) $(VECTOR_FLAGS) $(WARNINGS) -pipe  -D_GNU_SOURCE -std=gnu99 $(CFLAGS) -DDSFMT_MEXP=19937 -DHAVE_SSE2 -I.

#drop some PY_FLAGS
EXE_CFLAGS = -march=native -O3 -g  -fwrapv -Wstrict-prototypes -fPIC $(VECTOR_FLAGS) $(WARNINGS) -pipe  -D_GNU_SOURCE -std=gnu99 $(CFLAGS) -DDSFMT_MEXP=19937 -DHAVE_SSE2 -I.


clean:
	rm -f *.so *.[oadsi] dSFMT/*.[do]
	rm -f */*.[do]

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
#	$(CC)  -E  -DMODULE_NAME=qqq -Iinclude -I$(SALSA20_12_DIR) $(ALL_CFLAGS) $(CPPFLAGS) -o $@ $<

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
all::	salsa20_12.so
all::	salsa20_8.so

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

sosemanuk_pipe: $(SOSEMANUK_dir)/sosemanuk.o sosemanuk_pipe.c
	$(CC) -Iinclude -I$*  $(EXE_CFLAGS) $(CPPFLAGS) -DMODULE_NAME=$*  -Wl,-O1 -o $@ $+

isaac64.so isaac.so: %.so: %.o sha1.o ccan/isaac/%.o
	$(CC) -fPIC -pthread -shared -Wl,-O1 -o $@ $+

hc128.so: hc128.o  sha1.o
	$(CC) -fPIC -pthread -shared -Wl,-O1 -o $@ $+


##### ecrypt modules

SALSA_DIR_SUFFIX = regs
SALSA20_12_DIR = salsa20_12_$(SALSA_DIR_SUFFIX)
SALSA20_8_DIR = salsa20_8_$(SALSA_DIR_SUFFIX)

$(SALSA20_12_DIR)/salsa20.o $(SALSA20_8_DIR)/salsa20.o:  %.o:%.c
	$(CC)  -Iinclude -I$(@D) -fno-strict-aliasing  -MD $(ALL_CFLAGS)  -fvisibility=hidden  $(CPPFLAGS) -c -o $@ $<

salsa20_12.o salsa20_8.o: ecrypt_generic.c
	$(CC) -Iinclude -I$*_$(SALSA_DIR_SUFFIX) -c -MD $(ALL_CFLAGS) $(CPPFLAGS) -DMODULE_NAME=$* -o $@ $<

salsa20_8.so salsa20_12.so:  %.so: %.o  sha1.o %_$(SALSA_DIR_SUFFIX)/salsa20.o
	$(CC) -fPIC -pthread -shared -Wl,-O1 -o $@ $+


trivium_KEY_BYTES = '(80/8)'
trivium_IV_BYTES =  '(64/8)'
grain128_KEY_BYTES='(128/8)'
grain128_IV_BYTES='(96/8)'
grain_KEY_BYTES='(128/8)'
grain_IV_BYTES='(96/8)'
snow2_KEY_BYTES='(128/8)'
snow2_IV_BYTES='(64/8)'
tpy6_KEY_BYTES='(128/8)'
tpy6_IV_BYTES='(64/8)'
hc_128_KEY_BYTES='(128/8)'
hc_128_IV_BYTES='(128/8)'

ECRYPT_ROOT = tpy6 snow2 grain grain128 trivium sosemanuk2 rabbit hc_128
ECRYPT_OBJECTS = $(ECRYPT_ROOT:=/ecrypt.o)
ECRYPT_SO = $(ECRYPT_ROOT:=.so)
ECRYPT_O = $(ECRYPT_ROOT:=.o)
ECRYPT_GSL_O = $(ECRYPT_ROOT:=-gsl.o)
ECRYPT_GSL_SO = $(ECRYPT_ROOT:=-gsl.so)
ECRYPT_PIPE = $(ECRYPT_ROOT:=_pipe)

.PHONY: all pipe gsl objects

objects:: $(ECRYPT_O)
gsl::     $(ECRYPT_GSL_SO)
pipe::    $(ECRYPT_PIPE)
all::     $(ECRYPT_SO)

#tpy6/tpy6.o snow2/snow2.o grain/grain.o grain128/grain128.o trivium/trivium.o: %.o: %.c
$(ECRYPT_OBJECTS): %/ecrypt.o:
	$(CC)  -Iinclude -I$(@D)  -fno-strict-aliasing  -MD $(ALL_CFLAGS)  -fvisibility=hidden  $(CPPFLAGS) -c -o $@ $*/$*.c

$(ECRYPT_O): %.o: ecrypt_generic.c
	$(CC) -Iinclude -I$*  -c -MD $(ALL_CFLAGS) $(CPPFLAGS) -DMODULE_NAME=$* \
	-DKEY_BYTES=$($*_KEY_BYTES) -DIV_BYTES=$($*_IV_BYTES) -o $@ $<

$(ECRYPT_GSL_O): %-gsl.o: ecrypt_gsl_generic.c
	$(CC) -Iinclude -I$*  -c -MD $(ALL_CFLAGS) $(CPPFLAGS) -DMODULE_NAME=$*  -o $@ $<

 $(ECRYPT_SO):  %.so: %.o sha1.o %/ecrypt.o
	$(CC) -fPIC -pthread -shared -Wl,-O1 -o $@ $+

 $(ECRYPT_GSL_SO):  %-gsl.so: %-gsl.o %/ecrypt.o
	$(CC) -fPIC -pthread -shared -Wl,-O1 -o $@ $+


 $(ECRYPT_PIPE): %_pipe:  estream_pipe.c %/ecrypt.o
	$(CC) -Iinclude -I$*  $(EXE_CFLAGS) $(CPPFLAGS) -DMODULE_NAME=$*  -Wl,-O1 -o $@ $+

