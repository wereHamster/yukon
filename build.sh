#!/bin/sh

LIBDIR=${LIBDIR:-'lib'}
USRDIR="${HOME}/.yukon/${LIBDIR}"
CC=${CC:-'gcc'}

LibPrepare() {
	echo -n "."

	if [ ! -e "/usr/${LIBDIR}/${1}" ]; then
		echo -e "\n '/usr/${LIBDIR}/${1}' doesn't exist"
		exit 1
	fi

	cp /usr/${LIBDIR}/${1} ${USRDIR}/${2}

	if which md5sum > /dev/null; then
		md5sum /usr/${LIBDIR}/${1} | cut -f 1 -d ' ' > ${USRDIR}/${1}.md5
	fi

	./patcher ${USRDIR}/${2} ${1} ${2}
	local SONAME=$(objdump -x ${USRDIR}/${2} | grep SONAME | awk '{ print $2 }')
	ln -s ${2} ${USRDIR}/${SONAME}
}

LibRestore() {
	echo -n "."

	local SONAME=$(objdump -x /usr/${LIBDIR}/${1} | grep SONAME | awk '{ print $2 }')
	ln -s libyukon.so ${USRDIR}/${SONAME}
	ln -s ${SONAME} ${USRDIR}/${1}
}

echo -n "."
rm -Rf ${USRDIR} && mkdir -p ${USRDIR}

echo -n "."
${CC} -o patcher -std=c99 -pipe -O3 src/patcher.c

echo -n "."
LibPrepare libGL.so libFG.so
LibPrepare libX11.so libX13.so

echo -n "."
${CC} -shared -o libyukon.so -Iinclude -O3 -fPIC -L${USRDIR} -lX13 -lFG -lseom src/yukon.c

echo -n "."
cp libyukon.so ${USRDIR}/

LibRestore libGL.so
LibRestore libX11.so

echo -n "."
rm patcher
rm libyukon.so

echo " yukon was installed successfully"
echo ""
echo "Before you start your application, please make sure that"
echo "LD_LIBRARY_PATH points to \"${USRDIR}\""
echo "or use the \"yukon\" script located in the \"tools\" subdirectory."
echo ""
