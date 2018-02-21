sudo apt install usb-modeswitch
sudo usb_modeswitch -J -v 12d1 -p 1f01 -c /etc/usb_modeswitch.conf 
sudo ifconfig eth1 192.168.8.47

cat > /etc/udev/rules.d/10-Huawei.rules << "EOF1"
SUBSYSTEMS=="usb", ATTRS{modalias}=="usb:v12D1p1F01*", SYMLINK+="hwcdrom", RUN+="/usr/bin/sg_raw /dev/hwcdrom 11 06 20 00 00 00 00 00 01 00"
EOF1

echo "allow-hotplug eth1" >> /etc/network/interfaces
echo "iface eth1 inet dhcp" >> /etc/network/interfaces

sudo reboot
