mencoder -vf scale=960:768 -ovc xvid -xvidencopts bitrate=1800 -oac copy -audiofile "$1" -o "$2" -
