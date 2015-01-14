#!/bin/sh

#setup default values
audioname="audio.mp3"
videoname="video.avi"
scriptpath=

#retrieve values.
while getopts a:v:s: name
do
  echo "$OPTARG"
  case $name in
  a)   audioname="$OPTARG";;
  v)   videoname="$OPTARG";;
  s)   scriptpath="$OPTARG";;
  ?)   printf "Usage: %s: [-a audiofile] [-v videofile] [-s scriptpath] rawfile.seom\n" $0
       exit 2;;
  esac
done

#retrieving raw file (rawfile.seom) and the encode script path [encodescript]
shift $(($OPTIND - 1))
echo "$*"
rawfile="$1"

#if no raw is given, ask for one.
if [ ! -n $rawfile ]; then
  echo "you did not give a raw file (rawfile.seom)."
  printf "Usage: %s: [-a audiofile] [-b videofile] [-s scriptpath] rawfile.seom\n" $0
  exit 2
fi

#extract audio with lame
echo "generating .mp3 audio file from $rawfile ..."
./filter --audio "$rawfile" | lame - "$audioname"
echo "generated! you can find it at $audioname"

#encode the video.
echo "encoding video from $rawfile ..."
if [ -z $scriptpath ]; then
   echo "scriptpath is empty"
   ./filter --video "$rawfile" | mencoder -vf scale=1280:720 -ovc xvid -xvidencopts bitrate=1800 -oac copy -audiofile $audioname -o $videoname -
else
   echo "scriptpath is not empty"
   ./filter --video "$rawfile" | ./"$scriptpath" "$audioname" "$videoname"
fi
echo "generated! you can find it at $videoname"
