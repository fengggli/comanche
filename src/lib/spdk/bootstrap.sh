#!/bin/bash
#
# version should correspond to DPDK
#
echo "Boot-strapping SPDK ..."

SPDK_VERSION=$2

DPDK_INSTALL_DIR="$1/share/dpdk/x86_64-native-linuxapp-gcc/"


if [ ! -d ./spdk ] ; then
    git clone -b $SPDK_VERSION https://github.com/spdk/spdk.git
fi

cd spdk ; ./configure --with-dpdk=${DPDK_INSTALL_DIR} --without-vhost --without-virtio
make DPDK_DIR=${DPDK_INSTALL_DIR} CONFIG_RDMA=y
