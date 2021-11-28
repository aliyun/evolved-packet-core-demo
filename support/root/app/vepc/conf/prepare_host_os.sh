#!/bin/bash

echo "net.ipv4.ip_forward=1" >> /etc/sysctl.conf

rm /etc/network/interfaces.d/enp2s0
rm /etc/network/interfaces.d/enp3s0

ln -s /root/app/vepc/conf/enp2s0 /etc/network/interfaces.d/enp2s0
ln -s /root/app/vepc/conf/enp3s0 /etc/network/interfaces.d/enp3s0


cat << EOF > /etc/network/interfaces.d/enp7s0f1
auto enp7s0f1
iface enp7s0f1 inet static
address 192.168.222.101
mask 255.255.255.0
EOF


cat << EOF > /etc/rc.local
#!/bin/sh -e
#
# rc.local
#
# This script is executed at the end of each multiuser runlevel.
# Make sure that the script will "exit 0" on success or any other
# value on error.
#
# In order to enable or disable this script just change the execution
# bits.
#
# By default this script does nothing.
/root/app/vepc/conf/reboot_check.sh &
#/root/app/vepc/conf/ipsec_check.sh &
#/usr/bin/python2.7   /home/wnet/check_master.py  > /home/wnet/check_master_console.log 2>&1 &
exit 0
EOF
