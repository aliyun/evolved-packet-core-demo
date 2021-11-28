#!/bin/bash

/epc/cfg/vepc_pre_down.sh

killall -9 vepc-mmed
killall -9 vepc-spgwd
killall -9 vepc-pcrfd
killall -9 vepc-hssd

/epc/cfg/vepc_post_down.sh
