#/bin/bash
DEST=test-results
mkdir -p $DEST

for f in $@; do
    echo $f
    results=$DEST/$(basename $f)-test.txt
    $f | dieharder -g 200 -s 1 > $results
    $f | dieharder -g 200 -r 4 -d 1 -p 25 >> $results
    $f | dieharder -g 200 -r 2 -d 2 >> $results

    grep 'Assessment: ' $results
done
