#!/bin/sh

./filter --audio "$1" | lame - audio.mp3
./filter --video "$1" | mencoder -vf scale=1280:720 -ovc xvid -xvidencopts bitrate=1800 -oac copy -audiofile audio.mp3 -o video.avi -
