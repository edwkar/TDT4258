rm -f /dev/stk1000io
rmmod stk1000io.ko
insmod stk1000io.ko
MAJOR=`dmesg | grep "stk1000io" | tail -n 1 | awk '{print $6}'`
MINOR=`dmesg | grep "stk1000io" | tail -n 1 | awk '{print $7}'`
mknod /dev/stk1000switches c $MAJOR $MINOR
