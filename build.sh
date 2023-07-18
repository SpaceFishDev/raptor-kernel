#!/bin/bash
make
qemu-system-i386 -fda build/os.img
