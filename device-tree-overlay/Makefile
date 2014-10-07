NAME = PRUIO-DTO
VERSION = 00A0

$(NAME)-$(VERSION).dtbo: $(NAME).dts
	dtc -O dtb -o $(NAME)-$(VERSION).dtbo -b 0 -@ $(NAME).dts 

.PHONY: clean
clean:
	rm $(NAME)-$(VERSION).dtbo

.PHONY: install
install: $(NAME)-$(VERSION).dtbo
	cp $(NAME)-$(VERSION).dtbo /lib/firmware

.PHONY: uninstall
uninstall: 
	rm /lib/firmware/$(NAME)-$(VERSION).dtbo

.PHONY: load
load:
	echo $(NAME) > /sys/devices/bone_capemgr.9/slots

.PHONY: unload
unload:
	echo -`cat /sys/devices/bone_capemgr.9/slots | grep $(NAME) | sed 's/:.*//' | sed 's/ //'` > /sys/devices/bone_capemgr.9/slots
