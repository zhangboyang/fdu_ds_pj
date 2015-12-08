#!/bin/bash
SRCLIST=`ls | grep '\.cp*$'`
TARGETLIST=""
CXXFLAGS="-Wall -g"
LDFLAGS="$CXXFLAGS -ltinyxml2 -lGL -lGLU -lglut"
EXECNAME="pj"

function call_compiler()
{
    echo "g++ $@"
    g++ $@ || exit 1
}

for i in `echo "$SRCLIST"`; do
    SRC="$i"
    TARGET=`echo "$SRC" | sed -e 's/\.cp*/\.o/g'`
    TARGETLIST="$TARGETLIST $TARGET"
    #echo "compile: $SRC -> $TARGET"
    call_compiler -c $CXXFLAGS -o "$TARGET" "$SRC"
done

call_compiler -o "$EXECNAME" $TARGETLIST $LDFLAGS

exit 0
