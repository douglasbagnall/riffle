#!/bin/bash
#load colon separated variables
IFS=':' read STEM FREEDOM PATCH RENAME_ME ESTREAM_PATH <<< "$1"
#echo "stem: $STEM, patch: $PATCH, rename: $RENAME_ME, path: $ESTREAM_PATH"

TARGET=$STEM/ecrypt-sync.h

#Don't do all this just because someone did make -B
if [ -f $STEM/ecrypt-sync.h ]; then
   echo "$STEM/ecrypt-sync.h DOES exist.  To fetch it again, move it away."
   exit 0
fi

echo "'$TARGET'" does not exist.
[ "$FREEDOM" = "free" ] || echo "The $STEM license is vague or unknown".

if [ "$2" != "no-questions" ]; then
    echo I can try to replace the whole $STEM directory with a copy
    echo from the estream repository.  That involves fetching a tarball, untarring it,
    echo and possibly patching it a bit.
    echo "Do this now (y/N)?"
    read x && [ "Xy" = "X$x" ]  || exit 99
    echo "good! here goes"
fi

# $HERE is this scripts directory (repository root)
HERE=$(readlink -f $(dirname $0))

#fetch and extract the tar in a temp dir
TMPDIR=$(mktemp -d rngXXXXXXXXXX --t)
cd $TMPDIR

# get the tarball from ecrypt viewcvs
URL=http://www.ecrypt.eu.org/stream/svn/viewcvs.cgi/ecrypt/trunk/$ESTREAM_PATH.tar.gz?view=tar
TAR=$(basename $ESTREAM_PATH).tar.gz
echo "trying $URL"

wget -q -O $TAR $URL
tar -kxzf $TAR

DIR=$(basename $ESTREAM_PATH)


[ "$PATCH" ] && (cd $DIR; patch -p1 < $HERE/$PATCH)

[ "$RENAME_ME" ] && mv $DIR/$RENAME_ME $DIR/$STEM.c

#the Makefiles are useless and often confusing
rm -f $DIR/Makefile

[ -r  $HERE/$STEM ] && mv $HERE/$STEM $HERE/$STEM.$(date +%Y.%m.%d-%H.%M.%S)

mv -n $DIR $HERE/$STEM

#cd $HERE
#rm -r $TMPDIR