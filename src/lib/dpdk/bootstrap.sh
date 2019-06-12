#!/bin/bash
echo "Boot-strapping DPDK with $1..."
DPDK_VERSION="$1"
if [ "$DPDK_VERSION" == 'dpdk-18.08' ]; then
  DPDK_EXECENV=linuxapp
elif [ "$DPDK_VERSION" == 'dpdk-19.05' ]; then
  DPDK_EXECENV=linux
else
  echo "dpdk version ${DPDK_VERSION} not provided or not supported"
  exit -1
fi

echo "Using DPDK version ${DPDK_VERSION}"

if [ ! -d ./$DPDK_VERSION ] ; then
    echo "Downloading DPDK source ...."
    wget https://fast.dpdk.org/rel/$DPDK_VERSION.tar.xz
    tar -xf $DPDK_VERSION.tar.xz
    ln -s ./$DPDK_VERSION dpdk
fi

cp eal_memory.c ./$DPDK_VERSION/lib/librte_eal/$DPDK_EXECENV/eal/eal_memory.c
cp defconfig_x86_64-native-${DPDK_EXECENV}-gcc ./$DPDK_VERSION/config

cd ./$DPDK_VERSION/ ;
make install T=x86_64-native-${DPDK_EXECENV}-gcc DESTDIR=./build EXTRA_CFLAGS="-g -fPIC"
