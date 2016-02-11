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


NAME = PRUIO-DTO
VERSION = 00A0
CAPE_MGR_PATH = /sys/devices/platform/bone_capemgr/slots

$(NAME)-$(VERSION).dtbo: $(NAME).dts
	dtc -O dtb -o $(NAME)-$(VERSION).dtbo -b 0 -@ $(NAME).dts 

.PHONY: clean
clean:
	rm $(NAME)-$(VERSION).dtbo

.PHONY: install
install: $(NAME)-$(VERSION).dtbo
	cp -f $(NAME)-$(VERSION).dtbo /lib/firmware

.PHONY: uninstall
uninstall: 
	rm /lib/firmware/$(NAME)-$(VERSION).dtbo

.PHONY: load
load:
	echo $(NAME) > $(CAPE_MGR_PATH)

.PHONY: unload
unload:
	echo -`cat $(CAPE_MGR_PATH) | grep $(NAME) | sed 's/:.*//' | sed 's/ //'` > $(CAPE_MGR_PATH)
