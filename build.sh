#!/bin/sh

LIBDIR=${LIBDIR:-'lib'}
USRDIR="${HOME}/.yukon/${LIBDIR}"
CC=${CC:-'gcc'}

if ! which "echo" &>/dev/null; then
	exit
fi

TOOLS="rm mkdir ${CC} cp ln"
for tool in ${TOOLS}; do
	if ! which ${tool} &>/dev/null; then
		echo "Can't find '${tool}'" && exit
	fi
done

if ! which "md5sum" &>/dev/null; then
	echo -e "md5sum not found!\n"
fi

CFLAGS="-Iinclude -L${USRDIR} -lX13 -lFG -lseom"
if [ -n ${SEOM} ]; then
	CFLAGS="${CFLAGS} -I${SEOM}/include -L${SEOM}/.libs"
fi

abort() {
	echo -e "\nABORT: ${1}" && exit
}

LibPrepare() {
	echo -n "."

	[ ! -e "/usr/${LIBDIR}/${1}" ] && abort "/usr/${LIBDIR}/${1} doesn't exist"

	cp /usr/${LIBDIR}/${1} ${USRDIR}/${2} || abort "Couldn't copy system library ${1}."
	./patcher ${USRDIR}/${2} ${1} ${2} || abort "Couldn't patch ${1}"

	local SONAME=$(objdump -x ${USRDIR}/${2} | grep SONAME | awk '{ print $2 }')
	[ -z ${SONAME} ] && abort "${1} doesn't seem to have a SONAME."

	ln -s ${2} ${USRDIR}/${SONAME}  || abort "Couldn't create symlink ${SONAME} -> ${2}"

	if which md5sum &>/dev/null; then
		md5sum /usr/${LIBDIR}/${1} | cut -f 1 -d ' ' >${USRDIR}/${1}.md5
	fi
}

LibRestore() {
	echo -n "."

	local SONAME=$(objdump -x /usr/${LIBDIR}/${1} | grep SONAME | awk '{ print $2 }')
	[ -z ${SONAME} ] && abort "${1} doesn't seem to have a SONAME."

	ln -s libyukon.so ${USRDIR}/${SONAME}  || abort "Couldn't create symlink libyukon.so -> ${SONAME}"
	ln -s ${SONAME} ${USRDIR}/${1}  || abort "Couldn't create symlink ${1} -> ${SONAME}"
}

echo -n "."
rm -rf ${USRDIR} && mkdir -p ${USRDIR}

echo -n "."
${CC} -std=c99 -pipe -O3 -o patcher src/patcher.c || abort "Failed to compile the patcher"

echo -n "."
LibPrepare libGL.so libFG.so
LibPrepare libX11.so libX13.so

echo -n "."
${CC} -shared -fPIC -std=c99 -pipe -O3 ${CFLAGS} -o libyukon.so src/yukon.c || abort "Failed to compile yukon"

echo -n "."
cp libyukon.so ${USRDIR}/

LibRestore libGL.so
LibRestore libX11.so

echo -n "."
rm -f patcher
rm -f libyukon.so

echo " yukon was installed successfully"
echo ""
echo "Before you start your application, please make sure that"
echo "LD_LIBRARY_PATH points to \"${USRDIR}\""
echo "or use the \"yukon\" script located in the \"tools\" subdirectory."
echo ""
