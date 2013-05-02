rm -f /dev/stk1000switches
rm -f /dev/stk1000leds
rmmod stk1000io.ko > /dev/null 2> /dev/null

insmod stk1000io.ko

MAJOR=`dmesg | grep STK1000 | tail -n 1 | awk '{print $7}'`
MINOR=`dmesg | grep STK1000 | tail -n 1 | awk '{print $8}'`

mknod /dev/stk1000switches c $MAJOR $MINOR
mknod /dev/stk1000leds     c $MAJOR $(expr $MINOR + 1)
