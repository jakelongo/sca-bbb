if [ "$EUID" -ne 0 ]
  then echo "Need to run as root"
  exit
fi

echo 67  > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio67/direction

