#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo ,Youssef ghazaly.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # TODO: Add your kernel build steps here.
    
    # Clean the kernel build tree - removing the .config file with any existing configurations.
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper
    # build the defconfig which generates the default Configuration. 
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
    # build the binary vmlinux.
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all
    
fi

echo "Adding the Image in outdir"

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories

# Create the upper rootfs directory
mkdir rootfs
cd rootfs

# Create the necessary directories
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin
mkdir -p var/log




cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    make distclean
    make defconfig
else
    cd busybox
fi

# TODO: Make and install busybox

# make the busybox binary
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
#install the busybox on the rootdirectory 
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install

echo "Library dependencies"
#${CROSS_COMPILE}readelf -a /tmp/aeld/rootfs/bin/busybox | grep "program interpreter"
#${CROSS_COMPILE}readelf -a /tmp/aeld/rootfs/bin/busybox | grep "Shared library"

# TODO: Add library dependencies to rootfs

# Add program interpreter to out rootfs  
cp /home/ghazali/embedded_linux/arm-gnu-toolchain/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc/lib/ld-linux-aarch64.so.1 ${OUTDIR}/rootfs/lib

# Add shared libraries 
cp /home/ghazali/embedded_linux/arm-gnu-toolchain/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc/lib64/libm.so.6 ${OUTDIR}/rootfs/lib64

cp /home/ghazali/embedded_linux/arm-gnu-toolchain/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc/lib64/libresolv.so.2 ${OUTDIR}/rootfs/lib64

cp /home/ghazali/embedded_linux/arm-gnu-toolchain/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc/lib64/libc.so.6 ${OUTDIR}/rootfs/lib64



# TODO: Make device nodes
#Make the null device
cd ${OUTDIR}/rootfs
sudo mknod -m 666 dev/null c 1 3
#MAKE the Consloe device
sudo mknod -m 666 dev/console c 5 1
# TODO: Clean and build the writer utility
cd /home/ghazali/Downloads/embedded_linux_coursera/assignment-1-youssefghazali80/finder-app
make clean
make CROSS_COMPILE=aarch64-none-linux-gnu- all




# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
cp *.sh writer Makefile ${OUTDIR}/rootfs/home
cd /home/ghazali/Downloads/embedded_linux_coursera/assignment-1-youssefghazali80
cp -r conf ${OUTDIR}/rootfs/home
# TODO: Chown the root directory
cd ${OUTDIR}/rootfs
find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio
# TODO: Create initramfs.cpio.gz
cd ..
gzip -f initramfs.cpio

