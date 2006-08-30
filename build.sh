#!/bin/sh

function CopyLib {
	echo "   $2"
	cp /usr/$1/$2 $HOME/.preload/$1/$3
	./patcher $HOME/.preload/$1/$3 $2 $3
	so=`objdump -x $HOME/.preload/$1/$3 | grep SONAME | awk '{ print $2 }'`
	ln -s $3 $HOME/.preload/$1/$so
}

function RestoreLib {
	echo "   $2"
	so=`objdump -x /usr/$1/$2 | grep SONAME | awk '{ print $2 }'`
	ln -s yPreload.so $HOME/.preload/$1/$2
	ln -s yPreload.so $HOME/.preload/$1/$so
}

echo "Cleaning directory..."
rm -Rf $HOME/.preload/$1 && mkdir -p $HOME/.preload/$1

echo "Compiling patcher..."
gcc -o patcher src/patcher/main.c -std=c99

echo "Duplicating libraries..."
CopyLib $1 libGL.so libFG.so
CopyLib $1 libX11.so libX13.so

echo "Building yukon..."
scons -s -c .
LIBPATH=$HOME/.preload/$1 scons -s $2

echo "Restoring libraries..."
cp build/$2/yPreload.$2.so $HOME/.preload/$1/yPreload.so

RestoreLib $1 libGL.so
RestoreLib $1 libX11.so

echo "Cleaning up..."
rm patcher

echo ""
echo "Please add \"\$HOME/.preload/$1\" to LD_LIBRARY_PATH"
echo ""
