#!/usr/bin/env python3
import os
from subprocess import call

KMOD_NAME = 'tickle'


def unload_kmod():
    call(["sudo", "rmmod", KMOD_NAME])


def build_kmod():
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
