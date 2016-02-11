#! /bin/bash

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

echo ""
echo "1. Fetching dependencies"
cd ..
git submodule update --init --recursive > /dev/null

echo ""
echo "2. Installing beaglebone_pruio library."
cd library
PREFIX=/usr make uninstall > /dev/null
PREFIX=/usr make install > /dev/null

echo ""
echo '3. Disabling HDMI and "universal" cape.'
sed -i.bak 's/#dtb=am335x-boneblack-emmc-overlay.dtb/dtb=am335x-boneblack-emmc-overlay.dtb/' /boot/uEnv.txt
sed -i.bak 's/cape_universal=enable/cape_universal=disable/' /boot/uEnv.txt

echo ""
echo "4. Assigning index zero to USB sound card in ALSA."
echo 'options snd-usb-audio index=0' > /etc/modprobe.d/audio.conf

echo ""
echo "5. Installing the correct version of the Linux kernel. This will take a few minutes."
cd /opt/scripts/tools
git pull
apt-get update
apt-get install linux-headers-4.1.17-bone-rt-r19
./update_kernel.sh --bone-rt-kernel --lts-4_1


echo ""
echo ""
echo ""
echo "Done. Reboot now if there are no error messages above."

