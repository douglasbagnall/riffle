#!/usr/bin/python3

import sys
from math import sqrt
import time

NORMALISED_N = 1e7

FAST = [
    'salsa20_8',
    'trivium',
    'snow2',
    'salsa20_12',
    'tpy6',
    'hc128',
    'isaac64',
    'sosemanuk',
    'mt19937',
    'dSFMT',
    'lcg',
    'dummyc',
    ]

SLOW = [
    'grain',
    'grain128',
    'dummy',
    'random',
    'isaac',
    ]

REALLY_SLOW = ['urandom',]

DEFAULT = FAST
MOST = DEFAULT + SLOW
ALL = DEFAULT + SLOW + REALLY_SLOW

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
    stddev = (sqsum/n - avg*avg) ** 0.5
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
    rng.seed(seed)
    for a in ('random', 'seed', 'getstate', 'setstate'):
        setattr(wrapper, a, getattr(rng, a))
    try:
        wrapper.getrandbits = rng.getrandbits
    except AttributeError:
        print("skipping getrandbits")

    return wrapper


def test_variants(module, N=10000):
    print(module)
    m = __import__(module)
    rng = monkey_rng(m, 1)
    start = time.time()
    _test(rng)
    elapsed = time.time() - start
    print("Module %s took %s seconds" % (module, elapsed))

def test_sum(module, N=1000000, cycles=5):
    m = __import__(module)
    best = 1e999
    old_total = None
    rng = m.Random()
    r = rng.random
    for cycle in range(cycles):
        rng.seed(2)
        start = time.time()
        total = sum(r() for x in range(N))
        elapsed = time.time() - start
        if elapsed < best:
            best = elapsed
        if old_total and old_total != total:
            print("Mismatch in totals was %s, now %s" %(old_total, total))

    #print(m.__doc__)
    print("Module %s (sum of %s, best of %s runs)\n Total %10.4f    seconds %10.4f" %
          (module, N, cycles, total, best))
    return (best * NORMALISED_N / N)

def test_speed(module, N=10000000, cycles=5):
    m = __import__(module)
    best = 1e999
    old_total = None
    rng = m.Random()
    r = rng.random
    for cycle in range(cycles):
        rng.seed(2)
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

def test_print(module, N=1000):
    m = __import__(module)
    rng = m.Random()
    r = rng.random
    rng.seed(2)
    for x in range(N):
        print (r(), end=' ')
    print()


def test_several(test=test_speed, generators=None, **kwargs):
    if generators is None:
        generators = DEFAULT
    results = {}
    for x in generators:
        results[x] = test(x, **kwargs)

    print("Normailsed time (%d iterations)" % (NORMALISED_N,))
    ordered = sorted((v, k) for k, v in results.items())
    for x in ordered:
        print('%10.5s %s' % x)


def main():
    args = sys.argv[1:] or ['speed']

    generators = []
    tests = []
    unknown = []
    N = None
    runs = None
    G = globals()

    for x in args:
        if x.isupper():
            generators += G[x]
        elif x.isdigit():
          d = int(x)
          if d < 99:
              runs = d
          else:
              N = d
        elif 'test_' + x in G:
            tests.append(G['test_' + x])
        elif x in ALL:
            generators.append(x)
        else:
            unknown.append(x)

    if not generators:
        generators = DEFAULT

    if unknown:
        print("Unknown generators: %s" % (', '.join(unknown)))
#    for k in ('args', 'generators', 'tests', 'N', 'runs',
#              'unknown',):
#        print("%10s: %s" % (k, locals()[k]))

    kwargs = dict((k, v) for k, v in (('N', N), ('cycles', runs))
                  if v is not None)

    for x in tests:
        test_several(x, generators=generators, **kwargs)

main()
