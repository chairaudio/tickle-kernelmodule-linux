
# udev config

## add rules file

copy `90-tickle.rules` to `/etc/udev/rules.d/`
```
$ sudo cp 90-tickle.rules /etc/udev/rules.d/90-tickle.rules
```
## reload udev
```
$ sudo udevadm control --reload-rules
$ sudo udevadm trigger
```

# build and (re)load kernel module

unplug tickle from USB port
```
$ ./melmak.py

```
# make kernelmodule persistent

udev will load the module when a tickle is connected

## bash
```
$ echo "tickle" | sudo tee /etc/modules-load.d/tickle.conf
$ sudo mkdir /lib/modules/$(uname -r)/extra
$ sudo cp ./src/tickle.ko /lib/modules/$(uname -r)/extra
$ sudo depmod -a
```
## fish (if you use fish instead of bash)
```
$ echo "tickle" | sudo tee /etc/modules-load.d/tickle.conf
$ sudo mkdir /lib/modules/(uname -r)/extra
$ sudo cp ./src/tickle.ko /lib/modules/(uname -r)/extra
$ sudo depmod -a
```

# after upgrading the kernel
```
$ cd src
$ make clean
```
# check if kernel module is loaded
```
$ lsmod | grep "tickle"
```
should output something like:
```
tickle                 16384  2
```
if kernel module is loaded. If not, there will be no output. 

# known issues

## unsigned kernel module
If you encounter the error:
```
insmod: ERROR: could not insert module ./src/tickle.ko: Required key not available
```
Your system has UEFI secure boot active. You can resolve this by either disabling secure boot in the EFI menue before booting or signing the kernel module yourself. See https://askubuntu.com/questions/762254/why-do-i-get-required-key-not-available-when-install-3rd-party-kernel-modules