/*
 * Description:
 *
 * Author: Feng Li
 * e-mail: fengggli@yahoo.com
 */

#ifndef DMA_MAPPING_H_
#define DMA_MAPPING_H_

#include <linux/vfio.h>
#include <sys/ioctl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <common/logging.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#define VFIO_GROUP_PATH ("/dev/vfio/16")
#define DEVICE_PCI ("0000:20:00.0")

int find_vfio_fd(){
  int fd = -1;
	char proc_fd_path[PATH_MAX + 1];
	char link_path[PATH_MAX + 1];
	const char vfio_path[] = "/dev/vfio/vfio";
	DIR *dir;
	struct dirent *d;


	dir = opendir("/proc/self/fd");
	if (!dir) {
		PERR("Failed to open /proc/self/fd (%d)\n", errno);
		return -1;
	}

	while ((d = readdir(dir)) != NULL) {
		if (d->d_type != DT_LNK) {
			continue;
		}

		snprintf(proc_fd_path, sizeof(proc_fd_path), "/proc/self/fd/%s", d->d_name);
		if (readlink(proc_fd_path, link_path, sizeof(link_path)) != (sizeof(vfio_path) - 1)) {
			continue;
		}

		if (memcmp(link_path, vfio_path, sizeof(vfio_path) - 1) == 0) {
			sscanf(d->d_name, "%d", &fd);
			break;
		}
	}

	closedir(dir);

	if (fd < 0) {
		PERR("Failed to discover DPDK VFIO container fd.\n");
		return -1;
	}
  return fd;
}


/** Register a region of memory for DMA*/
int comanche_dma_register(void * vaddr, size_t size, int vfio_fd){
  // TODO: check alignment?

	int container, group, device, i;
	struct vfio_group_status group_status =
					{ .argsz = sizeof(group_status) };
	struct vfio_iommu_type1_info iommu_info = { .argsz = sizeof(iommu_info) };
	struct vfio_iommu_type1_dma_map dma_map = { .argsz = sizeof(dma_map) };
	struct vfio_device_info device_info = { .argsz = sizeof(device_info) };

#if 0
	/* Create a new container */
	container = open("/dev/vfio/vfio", O_RDWR);

	if (ioctl(container, VFIO_GET_API_VERSION) != VFIO_API_VERSION){
		/* Unknown API version */
    PERR("VFIO API unmatch");
    return -1;
  }

	if (!ioctl(container, VFIO_CHECK_EXTENSION, VFIO_TYPE1_IOMMU)){
		/* Doesn't support the IOMMU driver we want. */
    PERR("VFIO extension unmatch");
    return -1;
  }

	/* Open the group */
	group = open(VFIO_GROUP_PATH, O_RDWR);
  if(group <0){
    PERR("open group path failed at %s", VFIO_GROUP_PATH);
    return -1;
  }

	/* Test the group is viable and available */
	if(0 > ioctl(group, VFIO_GROUP_GET_STATUS, &group_status)){
    PERR("ioctl failed with VFIO_GROUP_GET_STATUS");
    return -1;
  }

	if (!(group_status.flags & VFIO_GROUP_FLAGS_VIABLE)){
		/* Group is not viable (ie, not all devices bound for vfio) */
    PERR("Group is not viable (ie, not all devices bound for vfio)");
    return -1;
  }

	/* Add the group to the container */
	if(0>ioctl(group, VFIO_GROUP_SET_CONTAINER, &container)){
      PERR("Add the group to the container")
      return -1;
  };

	/* Enable the IOMMU model we want */
	ioctl(container, VFIO_SET_IOMMU, VFIO_TYPE1_IOMMU);

#else

  container = vfio_fd;
#endif
	/* Get addition IOMMU info */
	ioctl(container, VFIO_IOMMU_GET_INFO, &iommu_info);

	/* Allocate some space and setup a DMA mapping */
	dma_map.vaddr = uint64_t(vaddr);
	dma_map.size = size;
	dma_map.iova = uint64_t(vaddr); /* 1MB starting at 0x0 from device view */
	dma_map.flags = VFIO_DMA_MAP_FLAG_READ | VFIO_DMA_MAP_FLAG_WRITE;

	if(0> ioctl(container, VFIO_IOMMU_MAP_DMA, &dma_map)){
      PERR("VFIO_IO_MMU_MAP_DMA failed with errno %s", strerror(errno));
      return -1;
  }
  else{
    PINF("DMA memory setup succeed!");
  }
  return 0;
}

#endif
