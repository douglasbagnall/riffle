#!/usr/bin/python3

import os, sys, subprocess
from imp import reload

C_HEADER = 'testbits.h'

two12 = 2 ** 12 #4096
two32 = 2 ** 32
two52 = 2 ** 52

def raw_values():
    v = (#0 based
        list(range(16)) +
        [2 ** x - 1 for x in range(4,12)] +
        [2 ** x for x in range(4,12)] +
        [2 ** x + 1 for x in range(4,12)] +
        #around 4096 (2**64 / 2**52)
        list(range(two12 - 10, two12 + 10)) +
        list(range(two12 - 1, two12 * 16, two12)) +
        list(range(two12, two12 * 16, two12)) +
        list(range(two12 + 1, two12 * 16 + 1, two12)) +
        # various points
        [2 ** x - 1 for x in range(14, 65)] +
        [2 ** x for x in range(14, 64)] +
        [2 ** x + 1 for x in range(14, 64)] +
        [10 ** x - 1 for x in range(2, 16)] +
        [10 ** x + 1 for x in range(2, 16)] +
        [13 ** x for x in range(2, 15)] +
        [3 ** x for x in range(41)] +
        # 2 ** 52 based
        [2 ** 52 - 5 + x for x in range(10)] +
        [2 ** 52 - 2 ** x for x in range(25)] +
        [2 ** 52 + 2 ** x for x in range(25)] +
        #2 ** 63 based
        [2 ** 63 + 2 ** x for x in range(13, 63)] +
        #2 ** 64 based
        [2 ** 64 - 2 ** x for x in range(63)] +
        [2 ** 64 - 3 ** x for x in range(41)] +
        [2 ** 64 - x for x in range(1, 25)] +
        #around 2 ** 64 - 2 ** 12
        [2 ** 64 - x for x in range(two12 - 10, two12 + 10)] +
        [2 ** 64 - x - 1 for x in range(two12, two12 * 16, two12)] +
        [2 ** 64 - x for x in range(two12, two12 * 16, two12)] +
        [2 ** 64 - x + 1 for x in range(two12, two12 * 16, two12)] +
        [42, 31337, 314159265359]
        )
    return sorted(set(v))
    #return v

def c_array(numbers, name, wrap=75, unused=True):
    table = ['0x%016xUL' % (x, ) for x in numbers]
    outs = []
    unused = ('UNUSED' if unused else '')
    outs.extend(("static const %s u64 %s [%s] = {" % (unused, name, len(table)),
                 '    %s' % table[0]))
    for v in table[1:]:
        if len(outs[-1]) >= wrap:
            outs[-1] += ','
            outs.append('    %s' % v)
        else:
            outs[-1] += ', %s' % v
    outs.append('};\n')

    return '\n'.join(outs)

def write_c_header(values):
    f = open(C_HEADER, 'w')
    print('/*autogenerated by %s\n Do not edit. Edit that instead! */' % __file__, file=f)
    print(c_array(values, 'values'), file=f)
    f.close()


MASKS = [ #multiplier, mask
    #first for RESCUE_BITS=0
    [(2 ** 52, 2 ** 52 - 1),
     (2 ** 64, 2 ** 64 - 1,),
     (2 ** 64, 2 ** 64 - 1,),
     ],
    #now for RESCUE_BITS=1
    [((2**52) << 12, (2 ** 52 - 1) << 12),
     ],
    ]

def run(values, double_coercion, rescue_bits=0):
        cflags = ('-DRESCUE_BITS=%s -DDOUBLE_COERCION=%s' %
                  (rescue_bits, double_coercion))
        print(cflags)
        os.environ['CFLAGS'] = cflags
        subprocess.call(['make', '-B', 'testbits.so'])
        import testbits
        rng = testbits.Random()
        rng.seed(2)
        mul, mask = MASKS[rescue_bits][double_coercion]
        for v in values:
            r = rng.random() * mul
            v = v & mask
            diff = r - v
            if diff or 1:
                print("%20s %20s %20s" % (int(r), v, diff))
        print()


def main():
    values = raw_values()
    write_c_header(values)
    if '--header-only' in sys.argv:
        return
    for dc, rescue in ((0, 1), #with rescue bits
                       (0, 0),
                       (1, 0),
                       (2, 0),
                       ):
        pid = os.fork()
        if pid == 0:
            run(values, dc, rescue)
            os._exit(0)
        os.wait()

main()
