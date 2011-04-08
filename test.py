#!/usr/bin/python3

import sys
from math import sqrt


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
    

def test_mt19937():
    import mt19937
    print(mt19937.__doc__)
    rng = monkey_rng(mt19937, 1)
    _test(rng, 1000)


test_mt19937()
