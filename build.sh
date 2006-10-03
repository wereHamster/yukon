#!/bin/sh

LIBDIR=${1:-'lib'}

CopyLib () {
	echo "   $1"
	cp /usr/$LIBDIR/$1 $HOME/.yukon/$LIBDIR/$2
	./patcher $HOME/.yukon/$LIBDIR/$2 $1 $2
	so=`objdump -x $HOME/.yukon/$LIBDIR/$2 | grep SONAME | awk '{ print $2 }'`
	ln -s $2 $HOME/.yukon/$LIBDIR/$so
}

RestoreLib () {
	echo "   $1"
	so=`objdump -x /usr/$LIBDIR/$1 | grep SONAME | awk '{ print $2 }'`
	ln -s libyukon.so $HOME/.yukon/$LIBDIR/$1
	ln -s libyukon.so $HOME/.yukon/$LIBDIR/$so
}

echo "Cleaning directory..."
rm -Rf $HOME/.yukon/$LIBDIR && mkdir -p $HOME/.yukon/$LIBDIR

echo "Compiling patcher..."
scons -s patcher

echo "Duplicating libraries..."
CopyLib libGL.so libFG.so
CopyLib libX11.so libX13.so

echo "Building yukon..."
LIBPATH=$HOME/.yukon/$LIBDIR scons -s libyukon.so

echo "Restoring libraries..."
cp libyukon.so $HOME/.yukon/$LIBDIR/

RestoreLib libGL.so
RestoreLib libX11.so

echo "Cleaning up..."
scons -s -c

echo ""
echo "Please make sure LD_LIBRARY_PATH points to \"\$HOME/.yukon/$LIBDIR\""
echo ""
