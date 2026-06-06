// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} cpus_kmem[NCPU];

void
kinit()
{
  uint64 start = (uint64) end;
  for (int i = 0; i < NCPU; i++) {
    initlock(&cpus_kmem[i].lock, "kmem");
    printf("%lxSTART", PGROUNDUP((uint64)end) );
    
    uint64 cpu_range_start = start + ((PHYSTOP - start) + (NCPU - 1) * PGSIZE) / NCPU * i;
    uint64 cpu_range_end = start + ((PHYSTOP - start) + (NCPU - 1) * PGSIZE) / NCPU * (i + 1);
    if(cpu_range_end > PHYSTOP) {
      cpu_range_end = PHYSTOP;
    }
    freerange((void*)cpu_range_start, (void*)cpu_range_end);
  }
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  push_off();
  int id = cpuid();

  acquire(&cpus_kmem[id].lock);
  r->next = cpus_kmem[id].freelist;
  cpus_kmem[id].freelist = r;
  release(&cpus_kmem[id].lock);

  pop_off();
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  push_off();
  int id = cpuid();

  int j = id;
  for (int i = 0; i < NCPU; i++) {
    acquire(&cpus_kmem[j].lock);
    r = cpus_kmem[j].freelist;
    if(r) {
      cpus_kmem[j].freelist = r->next;
      release(&cpus_kmem[j].lock);
      break;
    } else {
      release(&cpus_kmem[j].lock);
      j = (j + 1) % NCPU; 
    }
  }

  pop_off();

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
