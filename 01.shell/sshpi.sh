#!/bin/bash
# FILENAME: sshpi
function Usage() {
	echo "Usage: sshpi Username@pi"
	echo "Like This：sshpi UserA@pi1"
}
if [[ ! $# -eq 1  ]]; then
	Usage
	exit
fi
echo $1 | grep @ >/dev/null 2>&1
if [[ ! $? -eq 0 ]]; then
	echo "argument wrong！"
	Usage
	exit
fi
Username=`echo $1 | cut -d "@" -f 1`
if [[ ${Username}x == x ]]; then
	echo "Please input your username!"
	Usage
	exit
fi
Hostname=`echo $1 | cut -d "@" -f 2`
if [[ ${Hostname}x == x ]]; then
	echo "Please input Hostname of Pi!"
	Usage
	exit
fi
echo $Hostname | grep -w "^pi[1-9][0-9]\?" >/dev/null 2>&1
if [[ ! $? -eq 0 ]]; then
	echo "Hostname is Wrong!"
	Usage
	exit
fi
HostNum=`echo $Hostname | cut -c 3-`
if [[ $HostNum -gt 20 ]]; then
	echo "Hostname is Wrong!"
	Usage
	exit
fi


port=$[6530 + $HostNum]
echo -e "\033[46;30m You Will login\033[0m\033[46;31m $Hostname\033[0m\033[46;30m with Username \033[46;31m$Username\033[0m\033[46;30m, enjoy it!\033[0m"
ssh -p $port ${Username}@zentao.haizeix.tech
