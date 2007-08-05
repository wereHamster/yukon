#!/bin/sh

./filter --video "$1" | mplayer - &>/dev/null &
./filter --audio "$1" | mplayer -demuxer 35 - &>/dev/null
