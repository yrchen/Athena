ps -ax | grep mbbsd | awk '{print $1}' | xargs kill -9
ipcrm -M 1515
ipcrm -M 1517
ipcrm -M 1519
ipcrm -M 1521
ipcrm -M 1523
ipcrm -M 1225

ipcrm -S 2505
ipcrm -S 2500
ipcrm -S 2503
ipcrm -S 2222

~/innd/ctlinnbbsd shutdown

ipcs
ps -al | grep bbs
#/home/bbs/bin/repasswd
