#! /bin/bash

pd -nogui -rt -audiodev 0 -r 48000 -alsa -outchannels 2 -blocksize 128 $1
