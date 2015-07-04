#!/bin/bash

start_after=0
configfile="/etc/zncWebLog/zncWebLog.cfg"

echo -n "User that runs znc (os) []:"
read user
echo -n "Port you want zncWebLog to run on [8000]:"
read port

if [ ${#user} -eq 0 ]; then
	echo "you set no user, please change the user manually in /etc/zncWebLog/zncWebLog.cfg"
	start_after=0
else
	sed -i 's/_OSUSER_/'${user}'/g' ${configfile}
	chown ${user} ${configfile}
	chown ${user} /var/run/zwl.pid
	chown ${user} -R /etc/zncWebLog/
	chown ${user} /etc/init.d/zncWebLog
	chown ${user} /usr/local/bin/zncWebLogd
fi


if [ ${#port} -gt 1 ]; then
	sed -i 's/PortHTTP=8000;/PortHTTP='${port}';/g' ${configfile}
fi


if [ ${start_after} -eq 1 ]; then
	/etc/init.d/zncWebLog start
fi
