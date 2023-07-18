# raptor-kernel
FAT12 based 16 bit real mode ring 0 kernel.<br>
The kernel currently has support for 3 commands
```
CD [DIR]
LS
CAT [FILENAME]
```
I am working on much more but at the moment I am working on writing files. The current function does not work.

# build
You will need Qemu, MTools and NASM installed you also will neeed Open Watcom 2.0 in the /usr/bin/ directory.
If both are installed run 
`make clean`
`make`
this will also run the Kernel in Qemu. <br>
`make run` will run the Kernel in Qemu without building it

# Boot Process
The boot process is quite simple, it loads the FAT12 file system in the most basic way possible. It then loads Kernel.bin from the file system. When kernel.bin loads it properly sets up the FAT12 system and loads the terminal into the root directory, it also sets the video mode into Mode 13h.

# Tips For Using
- 'q' is used to exit most commands. If a command seems stuck press 'q'. 
- Reboot with the escape key. 
- If you read a file that doesnt exist the message "FAT READ ERROR" will print infinitely you cannot exit this and must restart.
