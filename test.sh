#!/usr/bin/env bash

export LD_LIBRARY_PATH="${HOME}/.yukon/lib:${LD_LIBRARY_PATH}"

if [ -z "${1}" ]; then
	if which glxgears; then glxgears; fi

else
	eval "${*}"
fi
