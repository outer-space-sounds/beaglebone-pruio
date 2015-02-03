#! /bin/sh

### 
# Beaglebone Pru IO 
# 
# Copyright (C) 2015 Rafael Vega <rvega@elsoftwarehamuerto.org> 
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
###

sudo sh -c 'echo 1 > /proc/sys/net/ipv4/ip_forward'
sudo iptables -A POSTROUTING -t nat -j MASQUERADE
echo "\n\nMake sure the option \"Use this connection only for resources on this network\" in network manager under ipv4 routes settings for the beaglebone wired connection is ENABLED"

