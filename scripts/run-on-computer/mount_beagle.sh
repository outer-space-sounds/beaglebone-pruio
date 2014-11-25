#! /bin/bash
./set_beagle_clock.sh
sshfs -o reconnect root@192.168.7.2:/ /media/beagle
