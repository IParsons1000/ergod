#! /bin/bash
# 
#   (c)2022-2025 Ira Parsons
#   ergod - the computer ergonomics daemon
# 

STARTED="\e[38;5;12mSTARTED\e[0m"
NOTE="\e[38;5;10mNOTE\e[0m"
ERROR="\e[38;5;9mERROR\e[0m"
FINISHED="\e[38;5;12mFINISHED\e[0m"

echo -e "$STARTED: ergod installation is commenced!"

if pgrep -x "ergod" > /dev/null # See if an instance of ergod is running and
then
	pkill ergod # Kill it
	echo -e "$NOTE: Killed running instance of ergod"
fi
	
make # Build the daemon

case $? in # Check for various errors

	0)
	  echo -e "$NOTE: Built ergod successfully"
	  ;;

	2)
	  echo -e "$ERROR: Makefile erroneous"
	  exit 1
	  ;;
	
	4)
	  echo -e "$ERROR: Make ran out of memory"
	  exit 1
	  ;;
	
	15)
	  echo -e "$ERROR: Problem with memory miser"
	  exit 1
	  ;;
	
	*)
	  echo -e "$ERROR: Failed to build ergod"
	  exit 1
	  ;;
esac

mv ergod /usr/local/sbin # Move the binary

if [ $? -eq 0 ] # Test for success
then
	echo -e "$NOTE: ergod binary moved succesfully to /usr/local/sbin"
else
	echo -e "$ERROR: Could not move ergod binary to /usr/local/sbin"
	exit 1
fi

selinuxenabled

if [ $? -eq 0 ] # Test for SELinux
then
	semanage fcontext -d -t bin_t -s system_u '/usr/local/sbin/ergod' # Change contexts, etc.
	semanage fcontext -a -t bin_t -s system_u '/usr/local/sbin/ergod' # Change contexts, etc.
	
	if [ $? -eq 0 ] # Test for success
	then
		echo -e "$NOTE: Succesfully changed ergod file context (SELinux)"
	else
		echo -e "$ERROR: Could not change ergod file context (SELinux)"
		exit 1
	fi
	
	restorecon /usr/local/sbin/ergod # Save changes
	
	if [ $? -eq 0 ] # Test for success
	then
		echo -e "$NOTE: Succesfully restored ergod file context (SELinux)"
	else
		echo -e "$ERROR: Could not restore ergod file context (SELinux)"
		exit 1
	fi
	
fi

cp ergod.service /usr/lib/systemd/system # Copy systemd.service file to appropriate location

if [ $? -eq 0 ] # Test for success
then
	echo -e "$NOTE: ergod.service systemd unit file succesfully moved to /usr/lib/systemd/system (systemd)"
else
	echo -e "$ERROR: Could not move ergod.service systemd unit file to /usr/lib/systemd/system (systemd)"
	exit 1
fi

rm /etc/systemd/system/ergod.service # Remove existing symbolic link if present
ln -s /usr/lib/systemd/system/ergod.service /etc/systemd/system/ergod.service # Create symbolic link to unit file in appropriate location

if [ $? -eq 0 ] # Test for success
then
	echo -e "$NOTE: Symbolic link to ergod.service succesfully created in /etc/systemd/system (systemd)"
else
	echo -e "$ERROR: Could not create symbolic link to ergod.service in /etc/systemd/system (systemd)"
	exit 1
fi

systemctl enable /usr/lib/systemd/system/ergod.service # Enable ergod

if [ $? -eq 0 ] # Test for success
then
	echo -e "$NOTE: ergod.service succesfully enabled (systemd)"
else
	echo -e "$ERROR: Could not enable ergod.service (systemd)"
fi

systemctl daemon-reload # Reload the daemons to make ergod immediately usable

if [ $? -eq 0 ] # Test for success
then
	echo -e "$NOTE: Daemons succesfully reloaded (systemd)"
else
	echo -e "$ERROR: Could not reload daemons (systemd)"
	echo -e "$ERROR: Failed to install ergod - exiting (1)"
	exit 1
fi

cp ergod.1.gz /usr/local/share/man/man1 # Install man page

if [ $? -eq 0 ] # Test for success
then
	echo -e "$NOTE: Man page succesfully installed: ergod(1)"
else
	echo -e "$ERROR: Could not install man pages.  Please install manually."
fi

echo -e "$FINISHED: ergod is succesfully installed! Reboot to make changes take effect."
exit 0
