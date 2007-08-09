#!/bin/sh

# We need an OpenGL application for the tests.
GLXGEARS=`which glxgears 2>/dev/null`
if [ "$?" = "1" ]; then
	echo "glxgears not found."
	echo " - Install a package that provides the glxgears application."

	exit 1
fi

# Check whether the yukon script is installed
if ! which yukon 1>/dev/null 2>&1; then
	echo "yukon script not found."
	echo " - Add \$PREFIX/bin to \$PATH."

	exit 1
fi

# Does the yukon script set the correct LD_LIBRARY_PATH ?
RET=`yukon printenv | grep LD_LIBRARY_PATH | grep yukon:`
if [ -z "${RET}" ]; then
	echo "The yukon script doesn't work correctly."
	echo " - Copy sysconf to /etc/yukon/system/ or \$HOME/.yukon/system/"

	exit 1
fi

# Does the application pick up the modified libraries ?
RET=`yukon ldd ${GLXGEARS} | grep yukon/libGL.so.native`
if [ -z "${RET}" ]; then
	echo "glxgears doesn't appear to be using the modified libraries."
	echo " - Make sure yukon is installed (run 'make install' as root)"

	exit 1
fi

# Extract the path where the yukon libraries are installed in
YPATH=`echo ${RET} | sed -e 's#.*\s/\(.*\)/libGL.so.native.*#/\1#'`

# Check whether yukon-core-lib finds libseom.so.?
RET=`yukon ldd "${YPATH}/yukon-core-lib" | grep -E libseom.so.? | grep "not found"`
if [ -n "${RET}" ]; then
	echo "libseom.so.? not found."
	echo " - Make sure this library is installed and the linker can find it"

	exit 1
fi

echo "Yukon seems installed correctly!"
echo "  Note that this script doesn't check the runtime. Applications may still"
echo "fail to work with yukon due to various reasons (bugs etc.)."