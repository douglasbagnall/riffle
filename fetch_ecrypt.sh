#!/bin/bash

echo "fetch_ecrypt.sh"
# $HERE is this scripts directory (repository root)
HERE=$(readlink -f $(dirname $0))
ESTREAM_DIR=$HERE/estream

#load colon separated variables
IFS=':' read STEM PATCH RENAME_ME ESTREAM_PATH <<< "$1"

echo "stem: $STEM, patch: $PATCH, rename: $RENAME_ME, path: $ESTREAM_PATH"

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

mv -n $DIR $HERE/$STEM

#cd $HERE
#rm -r $TMPDIR