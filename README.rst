Riffle: using stream ciphers as random number generators for Python 3
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. This README written in reStructuredText for automated html markup.
.. Apologies to plain text readers for the occasional odd construct.

This is a collection of C and Python code that wraps various stream
ciphers as modules conforming to Python's random module API.  Most of
the wrapped ciphers are from the eSTREAM_ project, which was a attempt
to find and standardise a set of good fast stream ciphers.

.. _eSTREAM: http://www.ecrypt.eu.org/stream/

Not all of the eSTREAM ciphers are well labeled from a software
licensing point of view, though most have *hints* that free use was
intended (see `Copyright and Licenses`_ below).  The eSTREAM ciphers
without clear free licenses are not included in this repository, but
the build system contains code to automatically download and compile
them.

The implementations used were chosen for portability over speed.
Riffle has only been tested on x86_64 Linux.

Building the software
=====================

To compile python modules::

$ make

You will need the Python3 headers (called "python3-dev" or
"python3-devel" by most Linux package managers).

To also compile generators that have unclear or unfree licenses (see
below), or that work badly or slowly::

$ make everything

That will download various generators from the eSTREAM site, unless
you have got them already.  If you hate getting asked y/N questions,
use

$ ECRYPT_NO_QUESTIONS=no-questions make everything

To make executable binaries that output streams of random bytes (look
in ./bin, use --help)::

$ make emitters

To compile the generators in GSL wrappers (completely useless, because
GSL won't know they're there), check you have GSL headers and::

$ make gsl

Configuring integer to float conversion
=======================================

By default, most generators will use bit fiddling techniques to
convert 52 bit integers into the significand of a (double) number
between 1 and 2, then subtract one.  This results in 2** 52 evenly
spaced numbers -- effectively the same as a 52 bit fixed point
representation.  This technique is borrowed from dSFMT_.

.. _dSFMT: http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/SFMT/#dSFMT

If you compile thus::

$ cflags='-DDOUBLE_COERCION=2' make

the floating point number will be made by dividing a 64bit number by 2
** 64 - 1.  This results in rather more possible numbers, with greater
precision approaching zero.  For numbers over 0.5 there are 53 bits of
resolution; between 0.5 and 0.25, 54 bits; between 0.25 and 0.125, 55
bits, and so on until the full 64 bits are reached around 0.0004.

In the ``ccan/isaac`` directory, Tim Terriberry demonstrates how to
extract full resolution for numbers very close to zero, by extracting
further random integers if there are not enough significant bits.  I
haven't used that because I don't care about precision in tiny
numbers.

Because the default conversion uses 52 bits at a time, 12 bits from
each 64 bit integer are discarded.  Rather than just being thrown out,
these bits can be collected up: out of 5 64 integers, you can get 6 52
bit floats.  Thus with slow underlying generators you approach a 6/5
speed up; with quick generators, like hc128, there is a smaller gain.

This can be turned off by compiling with::

$ cflags='-DRESCUE_BITS=0' make


Copyright and Licenses
======================

This is a little complicated, with different files having different
licenses.  The short answer is all the code is safe to use in a GPLv3
project, and much of it is more liberally licensed.  The GPL code
(except ``sha1.*``, which can also be MPL) is not used in Python
modules, so they are generally compatible with the license of Python
itself.

According to http://www.ecrypt.eu.org/stream/phase3ip.html (reproduced
as text at licenses/phase3-ip-statements.txt), most eSTREAM candidates
are "free for any use".  Additionally, a table in
http://www.ecrypt.eu.org/stream/papersdir/057.pdf (reproduced as
licenses/ip-table.txt) indicates that the reference code ("submitted
material") for all the ciphers used, except Rabbit, is free.  However,
because very few of the ciphers are accompanied by definite statements
about the licenses of their reference implementations, they are *not*
distributed with this package.  See see `Building the software`_ above
for details of how to include them.

Files not otherwise specified are copyright Douglas Bagnall and have a
BSD-MIT license, except for the ones that only work with GPL code,
which are themselves GPL.

Here is a summary::

 ccan/compiler/*             [Rusty Russell; GPLv3+]
 ccan/configurator.c         [Rusty Russell; BSD-MIT]
 ccan/ilog/*                 [Timothy B. Terriberry; LGPLv2+]
 ccan/isaac/*                [Bob Jenkins, Tim Terriberry; Public Domain]
 ccan/opt/*                  [Rusty Russell; GPLv3+]
 ccan/typesafe_cb/*          [Rusty Russell; LGPLv2+]
 dSFMT-src-2.1/*             [Mutsuo Saito, Makoto Matsumoto, Hiroshima
                              University, Andrea C G Mennucci; 3 clause BSD]
 sha1.c, sha1.h              [Paul Kocher; MPLv1.1+ or GPLv2+]
 mt19937module.c             [Makoto Matsumoto, Takuji Nishimura, Raymond
                              Hettinger; 3 clause BSD]
 hc128_opt32.h               [Hongjun Wu; UNKNOWN license]
 salsa20_12/*                [D. J. Bernstein; Public Domain]
 salsa20_8/*                 [D. J. Bernstein; Public Domain]
 chacha8/*                   [D. J. Bernstein; Public Domain]
 include/*                   [eSTREAM project; UNKNOWN]
 sosemanuk-clean/*           [Thomas Pornin; MIT-ish]
 sosemanuk2/*                [Thomas Pornin; MIT-ish]
 ffcsrh/*                    [FCSR project; "no restriction"]
 phelix/*                    [Doug Whiting; Public domain]

The following directories are *not* distributed with this software,
either because they have indistinct or non-free licenses, they are
somewhat broken as ciphers, or their performance is uninteresting.
However, they will all be downloaded, patched and compiled if you run
`make everything`, and you can make just one with, for example, `make
trivium.so`.

::

 tpy6/*                      [Eli Biham, Jennifer Seberry; UNKNOWN
                              ("no royalty")]
 trivium/*                   [Christophe De Cannière; UNKNOWN]
 grain/*                     [Martin Hell, et. al.; free-ish]
 grain128/*                  [Martin Hell, et. al.; free-ish]
 hc_128/*                    [Hongjun Wu; UNKNOWN license]
 rabbit/*                    [Cryptico A/S; "solely for non-commercial
                              purposes", possibly changed since]
 snow2/*                     [Patrik Ekdahl, Thomas Johansson; UNKNOWN]
 abc3                        [Vladimir Anashin, Andrey Bogdanov, Ilya
                              Kizhvatov; unclear (BSD-ish)]


Notes about various generators
==============================

For information about these generators and stream ciphers in general,
see:

 * The `eSTREAM site <http://www.ecrypt.eu.org/stream/>`_.

 * D. J. Bernstein's `stream cipher benchmarks
   <http://bench.cr.yp.to/results-stream.html>`_, and `security
   summary <http://cr.yp.to/streamciphers/attacks.html>`_.

 * and, of course, `Wikipedia
   <http://en.wikipedia.org/wiki/Stream_cipher>`_, and you can try
   some search engines.


**abc3** is version three of Anashin, Bogdanov, and Kizhvatov's ABC
cipher. It is allegedly cryptographically weak, and appears to have
statistical problems too.  It has a BSD-ish license that refers to the
"cipher" rather than the software implementation itself.

**chacha8** is a decendant of Salsa20 family and, like them, has been
put in the public domain by its author D.J. Benstein.  By altering a
symlink in the chacha8 directory, you can choose between the "ref" and
"regs" implementations.  "regs" is default, and slightly faster.
Bernstein also offers several optimised, non-portable, versions.

**dSFMT521**, **dSFMT1279**, **dSFMT2203**, **dSFMT19937**, and
**dSFMT216091** are variations of Mutsuo Saito and Makoto Matsumoto's
double generating, SIMD oriented, successor to MT19937.  The number
refers to the period (and state size) of the generator, with dSFMT521
having a period of 2 ** 521 - 1, and so on. The versions used has been
patched by Andrea C G Mennucci to save and restore state, though this
is not yet exposed to Python.  It is also possible to make generators
of sizes 4253, 11213, 44497, 86243, and 132049, using ``make dsfmt``
for them all, or (for example) ``make dSFMT44497.so`` for one.  The
longer periods are slightly quicker in benchmarks, but are probably
slower in real world situations due to cache churn.

**dummyc** always generates 0.5, as a speed benchmark.

**dummy** is a Python module that always returns 0.5.  Despite doing
no work, it is slower than most real generators.

**dummy2** is another Python module that returns 0.5, but unlike
dummy, it uses unnatural techniques to try to reach C speed.  It ends
up being twice as slow as dummyc, and twice as quick as dummy.

**ffcsrh** is the F-FCSR-H stream cipher.  It was included in the
final eSTREAM portfolio, only to be thrown out a few months later
after a critical weakness was discovered.  This probably does not
affect its statistical utility, though the cipher is designed for
hardware, and its software performance is not compelling.  It has a
very liberal license which explicitly mentions the reference
implementation.

**grain** and **grain128** are hardware oriented ciphers with a
liberal license that seems to be referring to the implementation as
well as the algorithm.  It contains this clause: "You may include the
Grain cipher in a licensed or patented product but the Grain cipher
must then be excluded from the license or patent in question".  Their
software performance is middling.

**hc_128** and **hc128** use two different implementations of Hongjun
Wu's HC-128 cipher (hc_128 is from eSTREAM and hc128 is from Wu's
site).  They are both very fast and the cipher is unbroken.  The
eSTREAM IP statement says the *cipher* is "not patented and are
royalty free. Anyone can use HC-128 and HC-256 free of charge".  I can
find no clear license for the reference software.

**isaac** and **isaac64** are Bob Jenkins' fast cryptographic random
number generators, as adapted/rewritten (I don't know which) by Tim
Terriberry and included in the CCAN project.  Both Jenkins' and
Terriberry's versions have been offered to the public domain.  Isaac64
is very fast on my i5 reference machine.

**lcg** is a classic (bad) linear congruential generator, with the
formula x = 1103515245 * x_previous + 12345 % (2^31).  Apparently it
serves as rand() in some libc's (though not glibc, which looks only
slightly better).

**mt19937** is an exact copy of Python 3.1's _randommodule.c, with
trivial changes to accommodate the change of name.  The module file is
called mt19937module.so, but in Python you just write ``import
mt19937``; somehow Python knows how to find it.

**phelix** dropped out of eSTREAM due to moderate weaknesses which
might not affect its role as a random number generator.  It is an
authenticating cipher, meaning it calculates a sort of checksum as it
encrypts and decrypts for sender and receiver to compare.  Thus it
does more work than other stream ciphers, and this work is of little
use for generating random numbers.  Nevertheless, it is not the
slowest.

**rabbit** is in the final eSTREAM portfolio.  During much of its
eSTREAM career the algorithm was shackled to some patent, but in 2008
the authors decided to release it "`into the public domain`_".  The
eSTREAM reference implementation still carries a restrictive comment,
though, perhaps because the code was finalised before it was freed.
The code, according to the same comment, "may be used solely for
non-commercial purposes", but it is possible that has changed too.

.. _`into the public domain`: http://www.ecrypt.eu.org/stream/phorum/read.php?1,1244

**salsa20_8** and **salsa20_12** are by D. J. Bernstein, who has
declared them to be public domain.  The code variant used here is
called "regs" in eSTREAM.  There are several optimised versions that
are faster on their particular platforms, but none of them are
portable.  The only real difference between the two is the number of
salsa rounds they use (8 or 12, as you might expect).  Salsa20/8 seems
to be regarded as cutting things fine for cryptographic work; it
should be fine as a statistical random number generator.

**snow2**, by Patrik Ekdahl and Thomas Johansson, was not an eSTREAM
candidate, but was wrapped up in the eSTREAM API to serve as a
benchmark.  It proved difficult to beat, with seemingly very few
ciphers matching it for speed and security.  The algorithm is free but
the code doesn't have an explicit license.

**sosemanuk-clean** and **sosemanuk2** are based on SOSEMANUK
reference code written by Thomas Pornin and possibly others, who
describe the license as "as close to Public Domain as any software
license can be under French law".  Sosemanuk-clean uses the native
SOSEMANUK API, while sosemanuk2 uses the ecrypt API.

**tpy6** is one of many descendants of the eSTREAM candidate Py.  Py
has been broken, but Tpy6 has not.  Tpy6 is quite quick.  Its contains
the following comment: "The designers/authors of Py (pronounced Roo)
keep their rights on the design and the name. However, no royalty will
be necessary for use of Py, nor for using the submitted code."

**trivium** is in eSTREAM's final selection of hardware-oriented
ciphers, but it is also reasonably quick in software.  It has a
smaller key than most others (80 bit), which doesn't matter for
statistical purposes.  The cipher is "free available for any use", but
it is unclear whether that applies to the reference code.

**urandom** is a Python wrapper for the Python library's
random.SystemRandom class, which in turn wraps /dev/urandom.  It is 2
or 3 hundred times slower than most others.

**xxtea** encodes a buffer using the XXTEA cipher, then normalises the
buffer to a series of floating point numbers.  For the next round, it
encodes the normalised buffer.  Whether or not this is a sound
procedure, it is not particularly fast.  The XXTEA implementation was
adapted from Wikipedia, which is turn derived from David Wheeler and
Roger Needham's public domain original.

Python API
==========

The Python module code is based on Python's standard ``_random``
module (the secret C module behind the public ``random`` module), and
uses the Random class API as documented_:

.. _documented: http://docs.python.org/py3k/library/random.html

  Class ``Random`` can also be subclassed if you want to use a
  different basic generator of your own devising: in that case,
  override the ``random()``, ``seed()``, ``getstate()``, and
  ``setstate()`` methods. Optionally, a new generator can supply a
  ``getrandbits()`` method -- this allows randrange() to produce
  selections over an arbitrarily large range.

Testing
=======

*./test.py* can perform a few simple tests.  Look at ``./test.py --help``
for options.  By default it tests a selection of generators
for speed, but it takes quite a few options.

*./test-bits.py* tests internal routines for coercing integers into
floating point numbers.  If everything is working well, it won't say
much.  Don't trust it excessively: there is a certain circularity in
the way it works.

After ``make emitters``, the ``bin/*-emitter`` files can be used to test
generators using, say, dieharder.  To do that, install dieharder,
then::

$ dieharder -g -1

and look for stdin_input_raw (200 for me, but it changes).  Then::

$ bin/sosemanuk-emitter | dieharder -g 200 -a

will take an age doing a large number of tests.  There will be a
number of false alarms: dieharder raises the alarm when something
happens that has a probability of less than 5%, but it does more than
20 tests, so some warnings are to be expected.  To retest a failing
test with a different seed::

$ bin/sosemanuk-emitter -s 42 | dieharder -g 200 -r 2

You can also use the emitters to test the raw speed of the generators,
without Python and number conversion overhead.  For example::

$ time bin/salsa20_8-emitter -b 500M > /dev/null

will tell you how long it takes to send 500 random Megabytes to
/dev/null.

Test results
============

This is an extract of ``./test.py`` output, showing how long it takes
to generate 10000000 numbers on an i5-540 (best of 5 runs)::

 dummyc         0.238  *********
 lcg            0.247  **********
 dSFMT          0.249  **********
 isaac64        0.300  ************
 hc_128         0.304  ************
 abc3           0.320  *************
 tpy6           0.339  **************
 hc128          0.341  **************
 snow2          0.342  **************
 chacha8        0.343  **************
 salsa20_8      0.344  **************
 trivium        0.353  **************
 random         0.369  ***************
 mt19937        0.373  ***************
 rabbit         0.374  ***************
 salsa20_12     0.381  ***************
 sosemanuk2     0.382  ***************
 sosemanuk      0.391  ****************
 dummy2         0.422  *****************
 phelix         0.486  ********************
 grain128       0.487  ********************
 xxtea          0.648  **************************
 isaac          0.672  ***************************
 dummy          0.876  ************************************
 ffcsrh         1.077  ********************************************
 grain          1.213  **************************************************

 urandom        92.02  **************************************************[...]*

These are the Python times: you can deduce from the dummyc result that
the Python overhead dwarfs the time taken by most generators.

I haven't done extensive tests of generator quality, but it does seem that:

* **abc3** is crazy.

* **dummy**, **dummyc**, and **dummy2** always return 0.5, as expected.

* **lcg** is weak, as expected.

* the others are at least superficially good.


The name
========

`Wikipedia`__: "a riffle is a short, relatively shallow and coarse-bedded
length of stream over which the stream flows at lower velocity and
higher turbulence".

.. __: http://en.wikipedia.org/wiki/Riffle

Adding new generators
=====================

To add new synchronous ciphers from the eSTREAM collection, read the
instructions at the top of ecrypt_generic.c.  There is a chance that
all you need to do is add a line to the Makefile.

Hacking
=======

The most complicating thing is probably that, due to the essential
similarity of each Python random module, there is a lot of
repetition. Then to ameliorate that, there is quite a bit of Makefile
and pre-processor trickery that pulls each module together from
various files.

To do
=====

* More ciphers (AES, panama, cryptmt).

* Non-cipher generators. WELL, for example.

* Testing.
