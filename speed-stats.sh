#!/bin/bash
#
# reformats a collection of fiels made thus:
#
# time ./test.py > test-results/speed-<description of the run>.txt
#
cd $(dirname $0)

(egrep "$1 +0" test-results/speed*) | while read name time bar 
do
    name2=${name:19}
    echo $time ${name2%.*$1}
done | sort
