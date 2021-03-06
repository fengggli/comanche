#ifndef __TX_CACHE_H__
#define __TX_CACHE_H__

#include <common/types.h>
#include <sys/mman.h>

#define MAP_HUGE_2MB (21 << MAP_HUGE_SHIFT)
#define MAP_HUGE_1GB (30 << MAP_HUGE_SHIFT)

namespace nupm
{
void *allocate_virtual_pages(size_t n_pages, size_t page_size, addr_t hint = 0);
int   free_virtual_pages(void *p);
}  // namespace nupm
#endif  // __TX_CACHE_H__
