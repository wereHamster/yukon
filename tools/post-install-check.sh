#!/bin/sh

# We need an OpenGL application for the tests.
GLXGEARS=`which glxgears 2>/dev/null`
if [ "$?" = "1" ]; then
	echo -e "glxgears not found.\n - Install a package that provides the glxgears application." && exit 1
fi

# Check whether the yukon script is installed
if ! which yukon 1>/dev/null 2>&1; then
	echo -e "yukon script not found.\n - Add \$PREFIX/bin to \$PATH." && exit 1
fi

# Does the yukon script set the correct LD_LIBRARY_PATH ?
RET=`yukon ldd ${GLXGEARS} | grep yukon/libGL.so.native`
if [ -z "${RET}" ]; then
	echo -e "The yukon script doesn't work correctly.\n - Copy sysconf to /etc/yukon/system or \$HOME/.yukon/system" && exit 1
fi

# Extract the path where the yukon libraries are installed in
YPATH=`echo ${RET} | sed -e 's#.*\s/\(.*\)/libGL.so.native.*#/\1#'`

# Check whether yukon-core-lib finds libseom.so.?
RET=`yukon ldd "${YPATH}/yukon-core-lib" | grep -E libseom.so.? | grep "not found"`
if [ -n "${RET}" ]; then
	echo -e "libseom.so.? not found.\n - Make sure this library is installed and the linker can find it." && exit 1
fi

echo "Yukon seems installed correctly!"