What's VEPC
================

VEPC is a implementation of the 3GPP Evolved Packet Core.

VEPC include four basic network element functions: MME, SPGW, HSS and PCRF.

MME(Mobility Management Engine), which terminates the S1 interfaces from the eNodeBs cells in the cellular network, and interfaces via S11 to the SPGW as well as via S6a to the HSS.

SPGW (Serving Gateway and Packet Data Network Gateway) implements the S11 interface to the MME, the SGi interface towards the Internet, and the S7 interface towards the PCRF.

HSS (Home Subscriber Server) is the central database of mobile network subscribers, with their IMSI, MSISDN, cryptographic key materials, service subscription information, etc. It implements the S6a interface towards the MME using the DIAMETER protocol.

PCRF (Policy and Charging Rules Function), which controls the service quality (QoS) of individual connections and how to account/charge related traffic.  It implements the Gx interface towards the SPGW using the DIAMETER protocol.

Installation 
============

## Install Mongo DB
```bash
sudo apt-get update
sudo apt-get -y install mongodb
sudo systemctl start mongodb (if '/usr/bin/mongod' is not running)
```

## Configure tun device.

```bash
sudo apt-get -y install udev net-tools
sudo systemctl start systemd-udevd (if '/lib/systemd/systemd-udevd' is not running)
```

```bash
sudo systemctl enable systemd-networkd
sudo systemctl restart systemd-networkd
```


Write the configuration file for the TUN deivce.
```bash
sudo sh -c "cat << EOF > /etc/systemd/network/99-vepc.netdev
[NetDev]
Name=pgwtun
Kind=tun
EOF"
```

```bash
sudo sh -c "cat << EOF > /etc/systemd/network/99-vepc.network
[Match]
Name=pgwtun
[Network]
Address=41.1.0.1/16
EOF"
```

```bash
sudo systemctl restart systemd-networkd
```


Disable IPv6 Kernel Configuration.
```bash
sysctl -n net.ipv6.conf.pgwtun.disable_ipv6

sudo sh -c "echo 'net.ipv6.conf.pgwtun.disable_ipv6=1' > /etc/sysctl.d/99-vepc.conf"
sudo sysctl -p /etc/sysctl.d/99-vepc.conf
```


## Build VEPC
```bash
sudo apt-get -y install autoconf libtool gcc pkg-config git flex bison libsctp-dev libgnutls28-dev libgcrypt-dev libssl-dev libidn11-dev libmongoc-dev libbson-dev libyaml-dev
autoreconf -iv
./configure --prefix=`pwd`/install
make
make install
```
