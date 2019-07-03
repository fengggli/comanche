## Prerequisite
1. build comanche.
2. sudo ./tools/prepare_nvmestore.sh (This check kernel boot options, spdk setup etc..)
3. Prepare mcas
  * Compile and build mcas module available at https://github.ibm.com/Daniel-Waddington/mcas/tree/unstable/src/kernel/modules/mcas
  * load mcas kernel module: sudo insmod mcasmod.ko

## Run
1. run the executable:
  ``build/src/components/store/nvmestore/testing/mcas-nvmestore/test-mcas-nvme --pci pci-addr``, where
  * pci-addr can be obtained by : lspci|grep Non
  * By default it will bypass the mcas module and register memory from producer directly this case works fine.
  * To register memory from consumer, uncomment ``#define USE_MCAS_MMAP`` in mcas-nvmestore/testcli.cc,recompile and rerun the test-mcas-nvme executable, then it will fail at *spdk_mem_register* .
