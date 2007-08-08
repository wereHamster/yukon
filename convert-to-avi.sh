#!/bin/sh

./filter --audio "$1" | lame - audio.mp3
./filter --video "$1" | mencoder -vf scale=400:300 -ovc xvid -xvidencopts bitrate=1200 -oac copy -audiofile audio.mp3 -o video.avi -
