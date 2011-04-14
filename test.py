#!/usr/bin/python3

import sys
from math import sqrt
import time

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
    m = __import__(module)
    rng = monkey_rng(m, 1)
    start = time.time()
    _test(r)
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

def test_list(module, N=10000000, cycles=5):
    m = __import__(module)
    best = 1e999
    old_total = None
    rng = m.Random()
    r = rng.random
    for cycle in range(cycles):
        rng.seed(2)
        start = time.time()
        #x = [r() for x in range(N)]
        for x in range(N):
            r()
        elapsed = time.time() - start
        if elapsed < best:
            best = elapsed
    print("Module %-12s (best of %s runs, %s cycles) %10.4f seconds" %
          (module, N, cycles, best))


def test_print(module, N=1000):
    m = __import__(module)
    rng = m.Random()
    r = rng.random
    rng.seed(2)
    for x in range(N):
        print (r(), end=' ')
    print()


#test_print('sosemanuk')
#sys.exit()

test = test_list

test('isaac')
test('isaac64')
test('sosemanuk')
test('random')
test('mt19937')
test('dSFMT')
test('lcg')
test('dummy')
test('dummyc')
#test('urandom')

