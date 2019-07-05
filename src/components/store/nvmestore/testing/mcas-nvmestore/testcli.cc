#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdint.h>
#include <signal.h>
#include <inttypes.h>
#include "mcas.h"
#include <string>
#include <boost/program_options.hpp> 
#include <api/components.h>
#include <api/block_itf.h>
#include <core/dpdk.h>
#include <spdk/env.h>
#include <core/xms.h>

#define START_IO_CORE 2
// prints LSB to MSB
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>

#include <nupm/nupm.h>
#include "dma-mapping.h"

#define REDUCE_KB(X) (X >> 10)
#define REDUCE_MB(X) (X >> 20)
#define REDUCE_GB(X) (X >> 30)
#define REDUCE_TB(X) (X >> 40)

#define KB(X) (X << 10)
#define MB(X) (X << 20)
#define GB(X) (((unsigned long) X) << 30)
#define TB(X) (((unsigned long) X) << 40)

#ifndef MAP_HUGETLB
#define MAP_HUGETLB 0x40000 /* arch specific */
#endif

#ifndef MAP_HUGE_SHIFT
#define MAP_HUGE_SHIFT 26
#endif

#ifndef MAP_HUGE_MASK
#define MAP_HUGE_MASK 0x3f
#endif

#define MAP_HUGE_2MB (21 << MAP_HUGE_SHIFT)
#define MAP_HUGE_1GB (30 << MAP_HUGE_SHIFT)

// #define USE_AEP
#define USE_MCAS_MMAP
#define USE_SPDK_REGISTER

static constexpr uint64_t TOKEN = 0xFEEB;
static constexpr size_t k_region_size = MB(2);

class Main
{
public:
  Main(const std::string& pci_id_vector);
  ~Main();

  void warmup();
  void run();
  
private:
  void create_block_component(const std::string vs);

  Component::IBlock_device * _block;
};

void Main::create_block_component(std::string pci_id)
{
  using namespace Component;
  IBase * comp = load_component("libcomanche-blknvme.so", block_nvme_factory);
  assert(comp);
  IBlock_device_factory * fact = (IBlock_device_factory *) comp->query_interface(IBlock_device_factory::iid());

  cpu_mask_t m;
  m.add_core(START_IO_CORE);

  _block = fact->create(pci_id.c_str(), &m, nullptr);
  fact->release_ref();
}

Main::Main(const std::string& pci_id_vector)
{
  DPDK::eal_init(64);

  create_block_component(pci_id_vector);
}

Main::~Main()
{
  _block->release_ref();
}

/** Warm the block device.*/
void Main::warmup(){
  Component::io_buffer_t iob = _block->allocate_io_buffer(KB(4),KB(4),Component::NUMA_NODE_ANY);
  void * vaddr = _block->virt_addr(iob);
  
  memset(vaddr, 0xf, KB(4));
  sleep(1);
  _block->write(iob, 0, 1, 1, 0 /* qid */);

  memset(vaddr, 0, KB(4));
  _block->read(iob, 0, 1, 1, 0 /* qid */);
}

void Main::run(){


  // size_t size = 1000000;
  size_t size = k_region_size;

  //  Devdax_manager ddm({{"/dev/dax0.3", 0x9000000000, 0},
  int fd = open("/dev/mcas", O_RDWR);
  assert(fd != -1);

  void * ptr = nullptr;
  void * pm_addr = ((void*) 0x900000000);
  
#ifdef USE_AEP
  nupm::Devdax_manager pm({{"/dev/dax1.0", ((uint64_t)pm_addr), 0}}, true);
  ptr = pm.create_region(1, 0, size);
#else
  int flags = MAP_PRIVATE | MAP_ANONYMOUS;
  flags |= MAP_HUGETLB | MAP_HUGE_2MB; // | MAP_FIXED;

  ptr = mmap(pm_addr,
             size,
             PROT_READ|PROT_WRITE,
             flags,
             -1,
             0);
    
#endif

  // touch memory from producer
  memset(ptr, 0, size);

  PINF("touched ptr=%p", ptr);  
  int rc;

#ifdef USE_MCAS_MMAP

  IOCTL_EXPOSE_msg ioparam;
  ioparam.token = TOKEN;
  ioparam.vaddr = ptr;
  ioparam.vaddr_size = size;

  /* expose memory */
  rc = ioctl(fd, IOCTL_CMD_EXPOSE, &ioparam);  //ioctl call
  if(rc != 0) {
    PWRN("ioctl to expose failed: rc=%d", rc);
  }

  /* map exposed memory */
  {
    int fd = open("/dev/mcas", O_RDWR, 0666);
    assert(fd != -1);
    offset_t offset = ((offset_t)TOKEN) << 12; /* must be 4KB aligned */

    void * target_addr = ((void*)0x700000000);
    void * ptr = ::mmap(target_addr,
                        size,
                        PROT_READ | PROT_WRITE,
                        // MAP_PRIVATE | MAP_FIXED, // | MAP_HUGETLB, // | MAP_HUGE_2MB,
                        MAP_SHARED | MAP_FIXED ,
                        fd,
                        offset); 

    if(ptr != ((void*) -1)) {
      PLOG("Map Expose Success!! (ptr=%p)", ptr);

      PLOG("press return to contrine ...");
      getchar();

      addr_t paddr = xms_get_phys(ptr);
      PLOG("[before touch]:(virt = %p,phys = 0x%lx,)", ptr, paddr);

      /* touch memory */
      memset(ptr, 0xee, size);
      paddr = xms_get_phys(ptr);
      // if it's a private mapping  paddr can be changed.
      PLOG("[after touch]: (virt = %p,phys = 0x%lx,)", ptr, paddr);
      PLOG(" press return to register io mem");
      getchar();

#ifndef USE_SPDK_REGISTER
      int vfio_container = find_vfio_fd();
      PINF("trying to register with vfio fd = %d", vfio_container);
      rc = comanche_dma_register(ptr, k_region_size, vfio_container);
#else
      rc = spdk_mem_register(ptr, k_region_size);
#endif

      if(rc != 0)
        PERR("memory register failed failed with rc %d", rc);
    }
    else {
      PLOG(" Map Expose Failed! %s", strerror(errno));
    }

    close(fd);
  }

#else
  // if register the memory from producer without going through mcas
  PLOG(" press return to register io mem");
  getchar();

#ifndef USE_SPDK_REGISTER
      int vfio_container = find_vfio_fd();
      PINF("trying to register with vfio fd = %d", vfio_container);
      rc = comanche_dma_register(ptr, k_region_size, vfio_container);
#else
      rc = spdk_mem_register(ptr, k_region_size);
#endif

  // rc = spdk_mem_register(ptr, k_region_size);
  if(rc != 0)
    PERR("memory register failed failed with rc %d", rc);
#endif

  if(rc == 0){
  PLOG(" press return to do block io");
  getchar();
  /* warm up */
  using io_buffer_t = uint64_t;
  int tag;
  io_buffer_t mem = (io_buffer_t)(ptr);
  for (unsigned i = 0; i*4096 < k_region_size; i++) tag = _block->async_write(mem, 0, i, 1);
  while (!_block->check_completion(tag))
    ; /* we only have to check the last completion */
  }

#ifdef USE_MCAS_MMAP
  PLOG("removing exposure...");

  {
    int rc;
    IOCTL_REMOVE_msg ioparam;
    ioparam.token = TOKEN;
    rc = ioctl(fd, IOCTL_CMD_REMOVE, &ioparam);  //ioctl call
    if(rc != 0)
      PWRN("ioctl IOCTL_CMD_REMOVE failed");
    else
      PLOG("ioctl IOCTL_CMD_REMOVE OK!");
  }
#endif

  PLOG("parent closing");
  munmap(ptr, size);
  
  close(fd);
}

int main(int argc, char * argv[]){
  Main * m = nullptr;
  
  try {
    namespace po = boost::program_options; 
    po::options_description desc("Options"); 
    desc.add_options()
      ("pci", po::value<std::string >(), "PCIe id for NVMe drive")
      ;
 
    po::variables_map vm; 
    po::store(po::parse_command_line(argc, argv, desc),  vm);

    if(vm.count("pci")) {
      m = new Main(vm["pci"].as<std::string>());
      m->warmup();
      m->run();
      delete m;
    }
    else {
      printf("mcas-dma [--pci 8b:00.0 ]\n");
      return -1;
    }
  }
  catch(...) {
    PERR("unhandled exception!");
    return -1;
  }

  return 0;
}
