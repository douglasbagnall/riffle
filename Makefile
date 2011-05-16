# Copyright 2011 Douglas Bagnall <douglas@paradise.net.nz> MIT License
#
# Riffle: more pseudo-random number generators for Python.
# The various PRNGs themselves were written by other people.
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation files
# (the "Software"), to deal in the Software without restriction,
# including without limitation the rights to use, copy, modify, merge,
# publish, distribute, sublicense, and/or sell copies of the Software,
# and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# The Software is provided "as is", WITHOUT WARRANTY of any kind,
# express or implied, including but not limited to the warranties of
# merchantability, fitness for a particular purpose and
# noninfringement. in no event shall the authors or copyright holders
# be liable for any claim, damages or other liability, whether in an
# action of contract, tort or otherwise, arising from, out of or in
# connection with the software or the use or other dealings in the
# Software.
.PHONY: all everything
all::
everything:: all

GDB_ALWAYS_FLAGS = -g
WARNINGS = -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers

PY_VERSION = $(shell python3 -c 'import sys; print(sys.version[:3])')
PYTHON = python$(PY_VERSION)

PY_FLAGS =  -pthread -DNDEBUG  -fwrapv -Wstrict-prototypes  -I/usr/include/$(PYTHON) -fPIC
#-DPy_BUILD_CORE

ALL_CFLAGS = -march=native -O3 -g $(PY_FLAGS) $(VECTOR_FLAGS) $(WARNINGS) -pipe  -D_GNU_SOURCE -std=gnu99 $(CFLAGS) -DHAVE_SSE2 -I.

#drop some PY_FLAGS
EXE_CFLAGS = -march=native -O3 -g  -fwrapv -Wstrict-prototypes -fPIC $(VECTOR_FLAGS) $(WARNINGS) -pipe  -D_GNU_SOURCE -std=gnu99 $(CFLAGS)  -DHAVE_SSE2 -I.

OPT_OBJECTS = ccan/opt/opt.o ccan/opt/parse.o ccan/opt/helpers.o ccan/opt/usage.o

#set this to avoid y/N questions before downloading ecrypt modules
#ECRYPT_NO_QUESTIONS = "no-questions"
.PHONY: clean dist-clean really-dist-clean debug free dodgy dsfmt

clean:
	rm -f *.so *.[oadsi] dSFMT/*.[do]
	rm -f */*.[do]
	rm -f bin/*-emitter

dist-clean: clean
	rm -f ccan/configurator config.h

really-dist-clean: dist-clean
	rm -rf $(ECRYPT_DODGY)
.c.o:
	$(CC) -Iinclude  -c -MD $(ALL_CFLAGS) $(CPPFLAGS) -o $@ $<

%.so:	%.o
	$(CC) -fPIC -pthread -shared -Wl,-O1 -o $@ $+

%.s:	%.c
	$(CC)  -S  $(ALL_CFLAGS) $(CPPFLAGS) -o $@ $<

%.i:	%.c
	$(CC)  -E  $(ALL_CFLAGS) $(CPPFLAGS) -o $@ $<

debug:
	make -B CFLAGS='-g -fno-inline -fno-inline-functions -fno-omit-frame-pointer -O0'

ccan/configurator:
	$(CC) $@.c -o $@

README.html: README.rst
	rst2html $< $@

config.h: ccan/configurator
	ccan/configurator > $@

testbits.h: test-bits.py
	./test-bits.py --header-only

testbits.o: testbits.h

isaac64.o isaac.o: config.h

DSFMT_FLAGS =  -finline-functions -fomit-frame-pointer -DNDEBUG -fno-strict-aliasing --param max-inline-insns-single=1800  -Wmissing-prototypes  -std=c99


DSFMT_SIZES =  521 1279 2203 4253 11213 19937 44497 86243 132049 216091
DSFMT_O = $(patsubst %,dSFMT%.o,$(DSFMT_SIZES))
DSFMT_SO = $(patsubst %,dSFMT%.so,$(DSFMT_SIZES))
DSFMT_DSFMT_O = $(patsubst %,dSFMT/dSFMT%.o,$(DSFMT_SIZES))
DSFMT_EMITTERS = $(patsubst %,bin/dSFMT%-emitter,$(DSFMT_SIZES))

#these ones get made by `make all`
DSFMT_MAKE_ALL_SIZES =  521 1279 2203 19937 216091
DSFMT_MODULES = $(patsubst %,dSFMT%.so,$(DSFMT_MAKE_ALL_SIZES))
#DSFMT_MODULES =  dSFMT521.so dSFMT1279.so dSFMT2203.so dSFMT19937.so dSFMT216091.so
DSFMT_MAKE_EMITTERS = $(patsubst %,bin/dSFMT%-emitter,$(DSFMT_MAKE_ALL_SIZES))

emitters::   $(DSFMT_MAKE_EMITTERS)

$(DSFMT_O): dSFMT.c
	$(CC) -Iinclude  -c -MD $(ALL_CFLAGS) $(CPPFLAGS) -DDSFMT_MEXP=$(patsubst dSFMT%.o,%,$@) -o $@ $<

$(DSFMT_DSFMT_O): dSFMT/dSFMT.c
	$(CC)  $(DSFMT_FLAGS)  -MD $(ALL_CFLAGS) -DDSFMT_MEXP=$(patsubst dSFMT/dSFMT%.o,%,$@) -fvisibility=hidden  $(CPPFLAGS) -c -o $@ $<

$(DSFMT_SO):  dSFMT%.so: dSFMT/dSFMT%.o dSFMT%.o
	$(CC) -fPIC -pthread -shared -Wl,-O1 -o $@ $+

$(DSFMT_EMITTERS):  bin/dSFMT%-emitter: dSFMT/dSFMT%.o dSFMT_emitter.c $(OPT_OBJECTS)
	mkdir -p bin
	$(CC) -Iinclude  -Iccan/opt/ -I$*  $(EXE_CFLAGS) $(CPPFLAGS) \
	   -DDSFMT_MEXP=$(patsubst bin/dSFMT%-emitter,%,$@)  -Wl,-O1 -o $@ $+

dsfmt:: $(DSFMT_SO)

SPECIAL_MODULES = mt19937module.so dummyc.so testbits.so

free:: $(SPECIAL_MODULES) $(DSFMT_MODULES)
all:: free
emitters::

##### standard ecrypt modules

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

#This looks crazy and perhaps is.  The point is that some of the
#estream ciphers have vague or questionable licenses, and it would be
#complicated to package them.  So the targets to make them download an
#estream tarball, and patch and rename as necessary.  $(ESTREAM_DATA)
#says what to do for each.

############ stem : freedom : patch : rename-this-to-<stem>.c : estream path
ESTREAM_DATA = tpy6:unknown:tpy6-fix-gcc-warnings.patch::submissions/py/tpy6 \
	grain128:unknown::grain-128.c:submissions/grain/128/opt \
	grain:unknown::grain-v1.c:submissions/grain/v1/opt \
	rabbit:unknown:::submissions/rabbit/opt/1 \
	trivium:unknown:trivium-add-keystream-bytes.patch::submissions/trivium \
	hc_128:unknown:hc_128-add-keystream-bytes.patch:hc-128.c:submissions/hc-256/hc-128/200701b \
	snow2:unknown:snow2-add-keystream-bytes.patch:snow-2.0.c:benchmarks/snow-2.0 \
	sosemanuk2:free::sosemanuk.c:submissions/sosemanuk \
	salsa20_12:free:salsa20_12-shush-gcc-warning.patch:salsa.c:submissions/salsa20/reduced/12-rounds/regs \
	salsa20_8:free:salsa20_8-shush-gcc-warning.patch:salsa.c:submissions/salsa20/reduced/8-rounds/regs \
	chacha8:free::: \
	ffcsrh:free:ffcsrh-add-keystream-bytes.patch:f-fcsr-h.c:submissions/f-fcsr/f-fcsr-h \
	abc3:mixed::abc-v3.c:submissions/abc/v3 \

ECRYPT_ROOT = $(foreach x,$(ESTREAM_DATA),$(firstword $(subst :, ,$(x))))
ECRYPT_OBJECTS = $(ECRYPT_ROOT:=/ecrypt.o)
ECRYPT_SO = $(ECRYPT_ROOT:=.so)
ECRYPT_O = $(ECRYPT_ROOT:=.o)
ECRYPT_GSL_O = $(ECRYPT_ROOT:=-gsl.o)
ECRYPT_GSL_SO = $(ECRYPT_ROOT:=-gsl.so)
ECRYPT_EMITTER = $(patsubst %,bin/%-emitter,$(ECRYPT_ROOT))
ECRYPT_H = $(patsubst %,%/ecrypt-sync.h,$(ECRYPT_ROOT))

#ECRYPT_DODGY have unclear or not quite free licenses
ECRYPT_DODGY = $(foreach x,$(ESTREAM_DATA),$(if $(findstring :free:, $x),,$(firstword $(subst :, ,$(x)))))
ECRYPT_FREE =  $(foreach x,$(ESTREAM_DATA),$(if $(findstring :free:, $x),$(firstword $(subst :, ,$(x)))))
ECRYPT_EXISTING_DODGY_SO = $(filter $(subst /ecrypt-sync.h,.so, $(wildcard */ecrypt-sync.h)), $(ECRYPT_DODGY:=.so))
ECRYPT_FREE_SO = $(ECRYPT_FREE:=.so)
ECRYPT_FREE_GSL_SO = $(ECRYPT_FREE:=-gsl.so)
ECRYPT_FREE_EMITTER = $(patsubst %,bin/%-emitter,$(ECRYPT_FREE))

ECRYPT_EXISTING_DODGY_GSL_SO = $(ECRYPT_EXISTING_DODGY_SO:.so=-gsl.so)
ECRYPT_EXISTING_DODGY_EMITTER = $(patsubst %.so,bin/%-emitter,$(ECRYPT_EXISTING_DODGY_SO))


.PHONY: emitters gsl objects  emitter-test test123

test123:
	@echo free:     $(ECRYPT_FREE)
	@echo dodgy:    $(ECRYPT_DODGY)
	@echo dodgy, existing: $(ECRYPT_EXISTING_DODGY_SO)

objects::     $(ECRYPT_O)
gsl::         $(ECRYPT_FREE_GSL_SO) $(ECRYPT_EXISTING_DODGY_GSL_SO)
emitters::    $(ECRYPT_FREE_EMITTER) $(ECRYPT_EXISTING_DODGY_EMITTER)
all::	      $(ECRYPT_FREE_SO) $(ECRYPT_EXISTING_DODGY_SO)
everything::  $(ECRYPT_SO)

#tpy6/tpy6.o snow2/snow2.o grain/grain.o grain128/grain128.o trivium/trivium.o: %.o: %.c
$(ECRYPT_OBJECTS): %/ecrypt.o: %/ecrypt-sync.h
	$(CC)  -Iinclude -I$(@D)  -fno-strict-aliasing  -MD $(ALL_CFLAGS)  -fvisibility=hidden -DECRYPT_API $(CPPFLAGS) -c -o $@ $*/$*.c

$(ECRYPT_O): %.o: ecrypt_generic.c %/ecrypt-sync.h
	$(CC) -Iinclude -I$*  -c -MD $(ALL_CFLAGS) $(CPPFLAGS) -DMODULE_NAME=$* \
	-DKEY_BYTES=$($*_KEY_BYTES) -DIV_BYTES=$($*_IV_BYTES) -o $@ $<

$(ECRYPT_GSL_O): %-gsl.o: ecrypt_gsl_generic.c %/ecrypt-sync.h
	$(CC) -Iinclude -I$*  -c -MD $(ALL_CFLAGS) $(CPPFLAGS) -DMODULE_NAME=$*  -o $@ $<

$(ECRYPT_SO):  %.so: %.o sha1.o %/ecrypt.o
	$(CC) -fPIC -pthread -shared -Wl,-O1 -o $@ $+

$(ECRYPT_GSL_SO):  %-gsl.so: %-gsl.o %/ecrypt.o
	$(CC) -fPIC -pthread -shared -Wl,-O1 -o $@ $+

$(ECRYPT_EMITTER): bin/%-emitter:  estream_emitter.c %/ecrypt.o $(OPT_OBJECTS) %/ecrypt-sync.h
	mkdir -p bin
	$(CC) -Iinclude  -Iccan/opt/ -I$*  $(EXE_CFLAGS) $(CPPFLAGS) -DMODULE_NAME=$* \
	-DKEY_BYTES=$($*_KEY_BYTES) -DIV_BYTES=$($*_IV_BYTES)   -Wl,-O1 -o $@ $+

emitter-test: emitters
	./emitters-test.sh bin/*-emitter

#if an ecrypt source directory is missing, try fetching it from ecrypt svn.
$(ECRYPT_H):
	./fetch_ecrypt.sh  $(filter $(subst /ecrypt-sync.h,,$@):%,$(ESTREAM_DATA)) $(ECRYPT_NO_QUESTIONS)



phelix/phelix.o: phelix/phelix.c
	$(CC) -Iinclude -I$* -MD $(ALL_CFLAGS) -DECRYPT_API -fvisibility=hidden  $(CPPFLAGS) -c -o $@ $<

bin/phelix-emitter phelix.so: phelix/phelix.o

SOSEMANUK_dir = sosemanuk-clean
$(SOSEMANUK_dir)/sosemanuk.o: $(SOSEMANUK_dir)/sosemanuk.c
	$(CC)  -MD $(ALL_CFLAGS)  -fvisibility=hidden  $(CPPFLAGS) -c -o $@ $<

sosemanuk.so bin/sosemanuk-emitter: $(SOSEMANUK_dir)/sosemanuk.o

############ stem :
DIVERSE_DATA = isaac64: \
	isaac: \
	xxtea: \
	hc128: \
	sosemanuk: \
	phelix: \
	lcg: \

DIVERSE_ROOT = $(foreach x,$(DIVERSE_DATA),$(firstword $(subst :, ,$(x))))
DIVERSE_SO = $(DIVERSE_ROOT:=.so)
DIVERSE_O = $(DIVERSE_ROOT:=.o)
DIVERSE_EMITTER = $(patsubst %,bin/%-emitter,$(DIVERSE_ROOT))

$(DIVERSE_O): %.o: diverse_generic.c diverse/rng-%.h
	$(CC) -Iinclude -I$* $(if $(findstring phelix, $*),-DECRYPT_API) -c -MD $(ALL_CFLAGS) $(CPPFLAGS) \
	-DMODULE_NAME=$* -o $@ $<

$(DIVERSE_SO):  %.so: %.o sha1.o
	$(CC) -fPIC -pthread -shared -Wl,-O1 -o $@ $+

$(DIVERSE_EMITTER): bin/%-emitter:  diverse_emitter.c diverse/rng-%.h $(OPT_OBJECTS)
	mkdir -p bin
	$(CC) -Iinclude  -Iccan/opt/ -I$*  $(EXE_CFLAGS) $(CPPFLAGS) -DMODULE_NAME=$* \
	   -Wl,-O1 $(if $(findstring phelix, $@),-DECRYPT_API) -o $@ $+

all:: $(DIVERSE_SO)
emitters:: $(DIVERSE_EMITTER)
