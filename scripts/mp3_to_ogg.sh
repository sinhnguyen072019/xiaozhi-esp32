#!/bin/sh
# mp3_to_ogg.sh <input_mp3_file> <output_ogg_file>
ffmpeg -i "$1" -vn -c:a opus -strict -2 -b:a 32k -ac 1 -ar 48000 "$2"