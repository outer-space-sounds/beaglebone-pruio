#! /bin/sh

sudo sh -c 'echo 1 > /proc/sys/net/ipv4/ip_forward'
sudo iptables -A POSTROUTING -t nat -j MASQUERADE
echo "\n\nMake sure the option \"Use this connection only for resources on this network\" in network manager under ipv4 routes settings for the beaglebone wired connection is ENABLED"

