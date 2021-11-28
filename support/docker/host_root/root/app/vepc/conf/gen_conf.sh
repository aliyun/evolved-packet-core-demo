#!/bin/bash

if [ $# != 2 -a $# != 4 ] ; then
echo "Usage: $0 [intfName vip/mask] [intfName vip/mask]"
echo " e.g.: $0 eth0 192.168.1.100/24"
echo "       $0 eth0 192.168.1.100/24 eth1 192.168.2.100/24"
exit 1;
fi

if [ $# == 2 ] ; then
s1_intf=$1
s1_vip=$2

cat > /root/app/vepc/conf/keepalived.conf << EOF
vrrp_script chk_sys {
    script "/epc/cfg/sys_check.sh"
    interval 5
}

vrrp_instance VI_1 {
    state BACKUP
    nopreempt
    interface $s1_intf
    virtual_router_id 50
    priority 90
    advert_int 1
    authentication {
        auth_type PASS
        auth_pass 1111
    }

    track_interface {
        $s1_intf
        $sgi_intf
    }

    virtual_ipaddress {
        $s1_vip dev $s1_intf
    }

    track_script {
       chk_sys
    }

    notify_master "/etc/keepalived/notify.sh master"
    notify_backup "/etc/keepalived/notify.sh backup"
    notify_fault  "/etc/keepalived/notify.sh fault"
}
EOF
fi


if [ $# == 4 ] ; then
s1_intf=$1
s1_vip=$2
sgi_intf=$3
sgi_vip=$4

cat > example.conf << EOF
vrrp_script chk_sys {
    script "/epc/cfg/sys_check.sh"
    interval 5
}

vrrp_instance VI_1 {
    state BACKUP
    nopreempt
    interface $s1_intf
    virtual_router_id 50
    priority 90
    advert_int 1
    authentication {
        auth_type PASS
        auth_pass 1111
    }

    track_interface {
        $s1_intf
        $sgi_intf
    }

    virtual_ipaddress {
        $s1_vip dev $s1_intf
        $sgi_vip dev $sgi_intf
    }

    track_script {
       chk_sys
    }

    notify_master "/etc/keepalived/notify.sh master"
    notify_backup "/etc/keepalived/notify.sh backup"
    notify_fault  "/etc/keepalived/notify.sh fault"
}
EOF
fi
