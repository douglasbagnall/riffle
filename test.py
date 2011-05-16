#!/usr/bin/python3
#
# Copyright 2011 Douglas Bagnall <douglas@paradise.net.nz> MIT License
#
# Part of Riffle, a collection of random number generators
#
# This file tests various aspects of the generators.  Use ./test.py --help
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

import sys
from math import sqrt
import time
import traceback

NORMALISED_N = 1e7
MAX_RUNS = 999
DEFAULT_TEST = 'speed'

FAST = [
    'murmur',
    'dSFMT521',
    'dSFMT1279',
    'dSFMT2203',
    'dSFMT216091',
    'chacha8',
    'hc_128',
    'rabbit',
    'sosemanuk2',
    'salsa20_8',
    'trivium',
    'snow2',
    'salsa20_12',
    'tpy6',
    'hc128',
    'isaac64',
    'sosemanuk',
    'mt19937',
    'dSFMT19937',
    'lcg',
    'dummyc',
    ]

SLOW = [
    'xxtea',
    'grain',
    'grain128',
    'dummy',
    'dummy2',
    'random',
    'isaac',
    'ffcsrh',
    'phelix',
    ]

REALLY_SLOW = ['urandom',]

TEST = ['testbits',]

BAD = ['abc3', ]


DEFAULT = FAST
MOST = DEFAULT + SLOW + BAD
ALL = DEFAULT + SLOW + REALLY_SLOW + TEST

#test fuctions borrowed from standard random module
def _test_generator(n, func, args):
    import time
    print(n, 'times', func.__name__)
    total = 0.0
    sqsum = 0.0
    smallest = 1e10
    largest = -1e10
    t0 = time.time()
    for i in range(n):
        x = func(*args)
        total += x
        sqsum = sqsum + x*x
        smallest = min(x, smallest)
        largest = max(x, largest)
    t1 = time.time()
    print(round(t1-t0, 3), 'sec,', end=' ')
    avg = total/n
    stddev = max(sqsum/n - avg*avg, 0) ** 0.5
    print('avg %g, stddev %g, min %g, max %g' % \
              (avg, stddev, smallest, largest))

def _test(rng=None, N=2000):
    _test_generator(N, rng.random, ())
    _test_generator(N, rng.normalvariate, (0.0, 1.0))
    _test_generator(N, rng.lognormvariate, (0.0, 1.0))
    _test_generator(N, rng.vonmisesvariate, (0.0, 1.0))
    _test_generator(N, rng.gammavariate, (0.01, 1.0))
    _test_generator(N, rng.gammavariate, (0.1, 1.0))
    _test_generator(N, rng.gammavariate, (0.1, 2.0))
    _test_generator(N, rng.gammavariate, (0.5, 1.0))
    _test_generator(N, rng.gammavariate, (0.9, 1.0))
    _test_generator(N, rng.gammavariate, (1.0, 1.0))
    _test_generator(N, rng.gammavariate, (2.0, 1.0))
    _test_generator(N, rng.gammavariate, (20.0, 1.0))
    _test_generator(N, rng.gammavariate, (200.0, 1.0))
    _test_generator(N, rng.gauss, (0.0, 1.0))
    _test_generator(N, rng.betavariate, (3.0, 3.0))
    _test_generator(N, rng.triangular, (0.0, 1.0, 1.0/3.0))


def monkey_rng(module, seed=None):
    import random
    wrapper = random.Random()
    rng = module.Random()
    rng.seed(seed if seed is not None else 2)
    for a in ('random', 'seed', 'getstate', 'setstate'):
        setattr(wrapper, a, getattr(rng, a))
    try:
        wrapper.getrandbits = rng.getrandbits
    except AttributeError:
        print("skipping getrandbits")

    return wrapper


def test_variants(module, N=10000, seed=2):
    print(module)
    m = __import__(module)
    rng = monkey_rng(m, seed)
    start = time.time()
    _test(rng)
    elapsed = time.time() - start
    print("Module %s took %s seconds" % (module, elapsed))

def test_sum(module, N=1000000, cycles=5, seed=2):
    m = __import__(module)
    best = 1e999
    old_total = None
    rng = m.Random()
    r = rng.random
    for cycle in range(cycles):
        rng.seed(seed)
        start = time.time()
        total = sum(r() for x in range(N))
        elapsed = time.time() - start
        if elapsed < best:
            best = elapsed
        if old_total is not None and old_total != total:
            print("Mismatch in totals was %s, now %s" %(old_total, total))
        old_total = total

    #print(m.__doc__)
    print("Module %s (sum of %s, best of %s runs)\n Total %10.4f    seconds %10.4f" %
          (module, N, cycles, total, best))
    return (best * NORMALISED_N / N)

def test_histogram(module, N=1000000, buckets=10, cycles=None, seed=2):
    m = __import__(module)
    if isinstance(cycles, int):#hack around simple api
        buckets = cycles
    rng = m.Random()
    r = rng.random
    h = [0] * buckets
    rng.seed(seed)
    _int = int
    start = time.time()
    for i in range(N//2):
        h[_int(r() * buckets)] += 1
        h[_int(r() * buckets)] += 1
    elapsed = time.time() - start
    by_count = sorted((n * buckets / N, i) for i, n in enumerate(h))
    minimum = by_count[0]
    maximum = by_count[-1]
    m1 = by_count[(buckets - 1) // 2]
    m2 = by_count[buckets // 2]
    if m1 == m2:
        median = m1
    else:
        median = ((m1[0] + m2[0]) * 0.5, "%s/%s" % (m1[1], m2[1]))

    print("Module %s, %s buckets, N=%s, %6.3fs" %
          (module, buckets, N, elapsed))
    print(' '.join(str(x) for x in h))
    print("min: %.3f (%s), medi: %.3f (%s), max: %.3f (%s)" %
          (minimum + median + maximum))

    if True:
        power = 2
        print("exaggerated scale (x ** %s)" % power)
        #normalised = (n * buckets / N, i) for i, n in enumerate(h))
        for x in h:
            bar = '#' * int((x * buckets / maximum[0] / N) ** power  * 70)
            print(bar)
    print()
    return (elapsed * NORMALISED_N / N)

def test_speed(module, N=10000000, cycles=5, seed=2):
    try:
        m = __import__(module)
    except ImportError as e:
        print(e)
        return
    best = 1e999
    old_total = None
    rng = m.Random()
    r = rng.random
    for cycle in range(cycles):
        rng.seed(seed)
        start = time.time()
        for x in range(0, N, 25):
            r();r();r();r();r()
            r();r();r();r();r()
            r();r();r();r();r()
            r();r();r();r();r()
            r();r();r();r();r()
        elapsed = time.time() - start
        if elapsed < best:
            best = elapsed
    print("Module %-12s (best of %s runs, %s cycles) %10.4f seconds" %
          (module, N, cycles, best))
    return (best * NORMALISED_N / N)

def test_print(module, N=None, cycles=None, seed=2):
    if N is None:
        if cycles is not None:
            N = cycles
        else:
            N = 200
    m = __import__(module)
    rng = m.Random()
    r = rng.random
    rng.seed(seed)
    for x in range(N):
        print (r(), end=' ')
    print()

def test_restore(module, N=None, cycles=None, seed=2):
    if N is None:
        if cycles is not None:
            N = cycles
        else:
            N = 20
    m = __import__(module)
    rng = m.Random()
    r = rng.random
    rng.seed(seed)
    #run away from initial state
    for x in range(999):
        r()
    print(module)
    try:
        state = rng.getstate()
        first = [r() for x in range(N)]
        rng.setstate(state)
    except Exception as e:
        print(e)
        return
    second = [r() for x in range(N)]
    if first != second:
        print(first, second, len(state))
    else:
        print("Good!")
    print()


def test_getrandbits(module, N=None, cycles=None, seed=2):
    sizes = [x for x in (cycles, N) if x is not None]
    if not sizes:
        sizes = [1, 2, 5, 12, 29, 43, 64, 122, 512]
    m = __import__(module)
    rng = m.Random()
    r = rng.random
    rng.seed(seed)
    #run away from initial state
    for x in range(2999):
        r()
    print(module)
    from math import log
    failed = False
    deltas = []
    try:
        for s in sizes:
            b = rng.getrandbits(s)
            if b:
                s2 = log(b, 2)
            else:
                s2 = -1e999
            d = s - (int(max(-1, s2)) + 1)
            deltas.append(d)
            if s2 > s:
                failed = True
                print("%s %0.2f ***FAILED***" % (s, s2))
            else:
                print("%s %0.2f" % (s, s2))
    except Exception as e:
        print(e)
        return
    if not failed:
        print(sorted(deltas))
        print("Good!")
    print()


def _test_several(test=test_speed, generators=None, **kwargs):
    if generators is None:
        generators = DEFAULT
    results = {}
    for x in generators:
        r = test(x, **kwargs)
        if r is not None:
            results[x] = r

    if results:
        ordered = sorted((v, k) for k, v in results.items())
        print("Normalised time (%d iterations)" % (NORMALISED_N,))
        slowest = max(ordered)[0]
        for (dt, name) in ordered:
            bar = '*' * int(dt * 50 / slowest)
            print('%-12s %7.5s  %s' % (name, dt, bar))



def usage():
    import textwrap
    G = globals()
    groups = []
    tests = []
    for k, v in G.items():
        if k.isupper() and isinstance(v, (list, tuple)):
            groups.append((k, v))
        elif k.startswith('test_'):
            tests.append(k[5:])
    groups.sort()
    print("USAGE:\n%s [generators | groups | tests | iterations per test | test runs | +seed]\n"
          % (sys.argv[0],))
    print("generators groups are")
    for k, v in groups:
        print(textwrap.fill("%s: %s" % (k, ', '.join(v)), subsequent_indent='    '))
    print()
    print(textwrap.fill("generators can be any generator in ALL. "
                        "With no generator named, all generators in DEFAULT will be tested\n"))
    print()
    print(textwrap.fill("tests are: %s." % (', '.join(tests))))
    print("The default test is %s\n" % (DEFAULT_TEST))
    print(textwrap.fill("A string prefixed with '+' is used to seed the generators "
                        "(the '+' is dropped).  If possible the string is interpreted as an integer."))
    print()
    print(textwrap.fill("Any number over %s sets the iterations "
                        "per test (defaults are test dependant)." % MAX_RUNS))
    print()
    print(textwrap.fill("Numbers less than %s set the number of runs, or for the histogram test, "
                        "the nyumber of buckets (defaults are test dependant)." % (MAX_RUNS + 1)))
    print()
    print("Generators, generator groups, and tests, are additive")
    print("Numeric values are not: the last one wins\n")
    print(textwrap.fill("EXAMPLE: run the sum and speed tests on the generators in "
                        "DEFAULT group and 'dummy'"
                        ", 7 times, using 300000 calls per run, and report the fastest runs:"))
    print("%s DEFAULT speed dummy 7 300000 sum" % (sys.argv[0],))
    sys.exit()

def main():
    args = sys.argv[1:]

    generators = []
    tests = []
    unknown = []
    N = None
    runs = None
    seed = None
    G = globals()

    for x in args:
        if x in ('-h', '--help'):
            usage()
        elif x.isupper():
            generators += G[x]
        elif x.isdigit():
          d = int(x)
          if d <= MAX_RUNS:
              runs = d
          else:
              N = d
        elif 'test_' + x in G:
            tests.append(G['test_' + x])
        elif x in ALL:
            generators.append(x)
        elif x.startswith('+'):
            try:
                seed = int(x[1:])
            except ValueError:
                seed = x[1:]
        else:
            unknown.append(x)

    if not generators:
        generators = DEFAULT
    if not tests:
        tests = [G['test_' + DEFAULT_TEST]]

    if unknown:
        print("Unknown generators: %s" % (', '.join(unknown)))
        print('try --help for hints')
    kwargs = dict((k, v) for k, v in (('N', N),
                                      ('cycles', runs),
                                      ('seed', seed))
                  if v is not None)

    for x in tests:
        _test_several(x, generators=generators, **kwargs)

main()
