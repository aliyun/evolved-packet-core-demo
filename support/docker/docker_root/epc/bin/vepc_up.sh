#!/bin/bash

/epc/cfg/vepc_pre_up.sh

killall -9 vepc_check.sh

/epc/bin/vepc-hssd -f /epc/cfg/vepc.conf -d
/epc/bin/vepc-pcrfd -f /epc/cfg/vepc.conf -d
/epc/bin/vepc-spgwd -f /epc/cfg/vepc.conf -d
/epc/bin/vepc-mmed -f /epc/cfg/vepc.conf -d

/epc/bin/vepc_check.sh &

/epc/cfg/vepc_post_up.sh
