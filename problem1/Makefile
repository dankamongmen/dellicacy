.DELETE_ON_ERROR:
.PHONY: all test clean

# Our external module is structured according to
# http://www.tldp.org/LDP/lkmpg/2.6/html/x181.html and
# http://www.mjmwired.net/kernel/Documentation/kbuild/modules.txt.

# We require a prebuilt kernel that contains the configuration and header files
# used during the build. This kernel must support modules. To prepare a
# virgin kernel tree, run "make modules_prepare". A full kernel build is
# necessary if module versioning is to work. See Section 2 of the kernel
# documentation referenced above for build targets.

# Example build:
#  [skynet](0) $ make -C ~/linux-stable/ M=$PWD clean
#  make: Entering directory `/home/dank/linux-stable'
#    CLEAN   /home/dank/dellicacy/problem1/.tmp_versions
#    CLEAN   /home/dank/dellicacy/problem1/Module.symvers
#  make: Leaving directory `/home/dank/linux-stable'
#  [skynet](0) $ make -C ~/linux-stable/ M=$PWD 
#  make: Entering directory `/home/dank/linux-stable'
#    LD      /home/dank/dellicacy/problem1/built-in.o
#    CC [M]  /home/dank/dellicacy/problem1/dellicacy.o
#    Building modules, stage 2.
#    MODPOST 1 modules
#    CC      /home/dank/dellicacy/problem1/dellicacy.mod.o
#    LD [M]  /home/dank/dellicacy/problem1/dellicacy.ko
#  make: Leaving directory `/home/dank/linux-stable'
#  [skynet](0) $ 

# This will produce, among other files, dellicacy.ko:
#  [skynet](0) $ sudo make -C ~/linux-stable/ M=$PWD modules_install
#  make: Entering directory `/home/dank/linux-stable'
#    INSTALL /home/dank/dellicacy/problem1/dellicacy.ko
#    DEPMOD  3.9.0
#  make: Leaving directory `/home/dank/linux-stable'
#  [skynet](0) $ 
#  [skynet](0) $ lsmod | grep dell
#  [skynet](1) $ sudo modprobe dellicacy
#  [skynet](0) $ lsmod | grep dell
#  dellicacy               8217  0 
#  [skynet](0) $ 

obj-m := dellicacy.o
