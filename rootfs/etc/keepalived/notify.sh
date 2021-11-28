#!/bin/bash
#
contact='root@localhost'
notify() {
	mailsubject="$(hostname) to be $1, vip floating"
	mailbody="$(date +'%F %T'): vrrp transition, $(hostname) changed to be $1"
	echo "$mailbody" | mail -s "$mailsubject" $contact
}
case $1 in
master)
#notify master
/epc/bin/vepc_up.sh
;;
backup)
#notify backup
/epc/bin/vepc_down.sh
;;
fault)
#notify fault
echo "fault" >> /var/log/keepalived.log
;;
*)
echo "Usage: $(basename $0) {master|backup|fault}"
exit 1
;;
esac
