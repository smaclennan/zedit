#!/bin/sh

[ -z "$KVERS" ] && KVERS=`uname -r`
[ -z "$VMLINUX" ] && VMLINUX=/lib/modules/$KVERS/build/vmlinux
[ -f $VMLINUX ] || { echo "$VMLINUX does not exist"; exit 1; }

set -e
sudo opcontrol --vmlinux=$VMLINUX
sudo opcontrol --deinit
sudo opcontrol --separate=kernel
sudo opcontrol --init
sudo opcontrol --reset
sudo opcontrol --start
set +e

./ze $*

sudo opcontrol --stop
opreport -l ./ze > zedit.profile

less zedit.profile
