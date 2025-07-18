#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096

#define PAGE_PRESENT   0x1
#define PAGE_RW        0x2
#define PAGE_USER      0x4
#define PAGE_PWT       0x8
#define PAGE_PCD       0x10
#define PAGE_ACCESSED  0x20
#define PAGE_DIRTY     0x40
#define PAGE_PS        0x80
#define PAGE_GLOBAL    0x100

void paging_map_page(uint64_t virt_addr, uint64_t phys_addr, uint64_t flags);
void paging_init(void);

#endif