#!/usr/bin/env python3
import os
import argparse
import subprocess
from subprocess import call, check_output, CalledProcessError

KMOD_NAME = 'tickle'

current_version = ""


def unload_kmod():
    print("1) unload kernel module")
    try:
        check_output(["sudo", "rmmod", KMOD_NAME],
                     stderr=subprocess.STDOUT)
    except CalledProcessError:
        pass
    print("...OK")
    print("")


def build_kmod():
    print("2) build kernel module")
    current_version = check_output(["git", "describe"]).decode().strip()
    gitversion_h = open("./src/gitversion.h", "w")
    gitversion_h.write("#define GITVERSION \"{}\"".format(current_version))
    gitversion_h.close()
    try:
        check_output(["make"], cwd='./src', shell=True,
                     stderr=subprocess.STDOUT)
    except CalledProcessError as e:
        print("could not build because")
        print(e.output.decode())
        return False

    print("...OK")
    print("")
    return True


def load_kmod():
    print("3) load kernel module")
    call(["sudo", "insmod", "./src/{}.ko".format(KMOD_NAME)])
    call(["sudo", "chmod", "666", "/dev/tickle"])
    print("...OK")
    print("")


def main():
    unload_kmod()
    if build_kmod():
        load_kmod()


if __name__ == '__main__':
    main()
