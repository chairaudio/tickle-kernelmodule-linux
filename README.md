# udev config

## add rules file

copy 90-tickle.rules to /etc/udev/rules.d/

## reload udev

$ sudo udevadm control --reload-rules 
$ sudo udevadm trigger


# build and (re)load kernel module

./build.py


# make kernelmodule persistent

udev will load the module when a tickle is connected

## fish
$ sudo cp ./src/tickle.ko /lib/modules/(uname -r)/extra
$ sudo depmod -a

## sh
$ sudo cp ./src/tickle.ko /lib/modules/$(uname -r)/extra
$ sudo depmod -a


# after upgrading the kernel

$ cd src
$ make clean
