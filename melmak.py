#!/usr/bin/env python3
import os
import argparse
from subprocess import call, check_output

KMOD_NAME = 'tickle'

current_version = ""

def unload_kmod():
    call(["sudo", "rmmod", KMOD_NAME])


def build_kmod():
    current_version = check_output(["git", "describe"]).decode().strip()
    gitversion_h = open("./src/gitversion.h", "w")
    gitversion_h.write("#define GITVERSION \"{}\"".format(current_version))
    gitversion_h.close()     
    call(["make"], shell=True, cwd="./src")


def load_kmod():
    call(["sudo", "insmod", "./src/{}.ko".format(KMOD_NAME)])
    call(["sudo", "chmod", "666", "/dev/tickle"])


def main():
    unload_kmod()
    build_kmod()
    load_kmod()


if __name__ == '__main__':
    main()
