#!/bin/sh

LIBDIR=${LIBDIR:-'lib'}
CC=${CC:-'gcc'}

CopyLib () {
	if [ ! -e "/usr/$LIBDIR/$1" ]; then
		echo "   /usr/$LIBDIR/$1 doesn't exist"
		exit
	fi
	
	cp /usr/$LIBDIR/$1 $HOME/.yukon/$LIBDIR/$2
	
	if which md5sum > /dev/null; then
		md5sum /usr/$LIBDIR/$1 | cut -f 1 -d ' ' > $HOME/.yukon/$LIBDIR/$1.md5
	fi

	./patcher $HOME/.yukon/$LIBDIR/$2 $1 $2
	so=`objdump -x $HOME/.yukon/$LIBDIR/$2 | grep SONAME | awk '{ print $2 }'`
	ln -s $2 $HOME/.yukon/$LIBDIR/$so
}

RestoreLib () {
	so=`objdump -x /usr/$LIBDIR/$1 | grep SONAME | awk '{ print $2 }'`
	ln -s libyukon.so $HOME/.yukon/$LIBDIR/$so
	ln -s $so $HOME/.yukon/$LIBDIR/$1
}

echo "Cleaning directory..."
rm -Rf $HOME/.yukon/$LIBDIR && mkdir -p $HOME/.yukon/$LIBDIR

echo "Compiling patcher..."
$CC -o patcher -std=c99 -pipe -O3 src/patcher.c

echo "Duplicating libraries..."
CopyLib libGL.so libFG.so
CopyLib libX11.so libX13.so

echo "Building yukon..."
$CC -shared -o libyukon.so -Iinclude -O3 -fPIC -L$HOME/.yukon/$LIBDIR -lX13 -lFG -lseom src/yukon.c

echo "Restoring libraries..."
cp libyukon.so $HOME/.yukon/$LIBDIR/

RestoreLib libGL.so
RestoreLib libX11.so

echo "Cleaning up..."
rm patcher
rm libyukon.so

echo ""
echo "Please make sure LD_LIBRARY_PATH points to \"\$HOME/.yukon/$LIBDIR\""
echo ""
