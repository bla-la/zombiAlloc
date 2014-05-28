#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>

#include <lock.h>
#include <list.h>
#include <maps.h>


//Linux uninitialized
#ifndef MAP_UNINITIALIZED
#define MAP_UNINITIALIZED 0x4000000
#endif
#define DEFAULT_MMAP_FLAGS MAP_UNINITIALIZED | MAP_PRIVATE | MAP_ANONYMOUS

#define MAP_POLL_SIZE 4096
#define MAP_META_SIZE 4096

#define MAP_POLL_MAGIC 0xccff870
#define MAP_MAGIC 0xccff871

#define SET_BIT(num,x) ((num) |= 1 << x)
#define CLE_BIT(num,x) ((num) &= ~(1 << x))

#define GET_POLL_START(m) ((unsigned long long)((-MAP_POLL_SIZE & (unsigned long long) m)))
#define GET_POLL(m) ((char *)(GET_POLL_START(m) - MAP_META_SIZE))
#define GET_MAP_IDX(m) ((((unsigned long long)m) - GET_POLL_START(m))/sizeof(map))
#define BITMAP_EMPTY(bm) (*((unsigned long long *)(bm)) == -1 && *((unsigned long long *)(bm)+1) == -1)

typedef struct __map_poll map_poll;
struct __map_poll
{
    map_poll *prev;
    map_poll *next;
    spinlock __lock;
    int magic;
} __attribute__ ((aligned (16)));

typedef struct __map_poll_head map_poll_head;
struct __map_poll_head
{
    map_poll *first;
    map_poll *last;
    size_t count;
    size_t free;
}__attribute__ ((aligned (16)));



map_poll_head pollHead __attribute__ ((aligned (16)))  = {0};
/*Fast lookup for poll with free map*/
map_poll *cacheHasFree __attribute__ ((aligned (16))) = 0;
spinlock pollLock __attribute__ ((aligned (16))) = 0;

#define POLL_BITMAP_COUNT 4
#define MAP_COUNT (MAP_POLL_SIZE/sizeof(map))
#define GLOBAL_LOCK() spin_lock(&pollLock)
#define GLOBAL_UNLOCK() spin_unlock(&pollLock)


#define get_bitmap(p) (((unsigned int *)( ((char *)p) + sizeof(map_poll) ) ))



static int
ffz(unsigned int bit_array)
{
    unsigned int pos = 0;

    __asm__ volatile ("bsfl %1,%0\n\t"
          "jne 1f\n\t"
          "movl $32, %0\n"
          "1:"
		      : "=r" (pos)
		      : "r" (~(bit_array)));

    if(pos >= 32)
    	return -1;
    return  pos;
}


void
init_poll_bitmap(map_poll *p)
{
    int i;
    unsigned int * bp = get_bitmap(p);

    for(i = 0;i < POLL_BITMAP_COUNT;i++)
    {
	*(bp+i) = 0;
    }
}


static int
poll_has_free(map_poll *p)
{
    int i;
    unsigned int * bp = get_bitmap(p);

    return !BITMAP_EMPTY(bp);
}

map_poll *
alloc_poll()
{
    map_poll * pp;
    void *ptr = mmap(0,MAP_META_SIZE+MAP_POLL_SIZE,
		      PROT_READ|PROT_WRITE, DEFAULT_MMAP_FLAGS ,0, 0);
    if(!ptr)
	abort();

    pp = (map_poll *) ptr;
    pp->prev = 0;
    pp->next = 0;
    pp->__lock = 0;
    pp->magic = MAP_POLL_MAGIC;
    init_poll_bitmap(pp);
    return pp;
}


int getIndex(unsigned int * bitmap,size_t len)
{
    int idx = -1;
    int i = 0;

    while(idx == -1 && i < len)
    {
	idx = ffz(*bitmap);
	bitmap += 1;
	i++;
    }

    if(unlikely(idx != -1))
    {
	SET_BIT(*(bitmap-1),idx);
	idx += ((i-1) * 32);
    }

    return idx;
}

map *
get_map_from_poll(map_poll *p)
{
    int idx;
    map * m = 0;
    unsigned int *bm = get_bitmap(p);
    pollHead.free--;
    idx = getIndex(bm,POLL_BITMAP_COUNT);
    m = ((map *)(((char*)p)+4096)) + idx;

    if(unlikely(BITMAP_EMPTY(bm)))
	cacheHasFree = 0;

    return m;
}


map *
get_map()
{
    map *m = 0;
    map_poll *p = 0;
    if(unlikely(!pollHead.free))
    {
	p = alloc_poll();
	list_append((&pollHead),p,map_poll);
	pollHead.free += MAP_COUNT;
	pollHead.count += MAP_COUNT;
	cacheHasFree = p;
	m = get_map_from_poll(p);
	return m;
    }

    p = pollHead.first;
    while(unlikely(p && !poll_has_free(p)))
	p = p->next;
    if(unlikely(!p))
	abort();
    m = get_map_from_poll(p);
    return m;
}

map *
do_map(size_t size,int fd)
{
    map *m = 0;
    GLOBAL_LOCK();
    if(cacheHasFree)
    {
	m = get_map_from_poll(cacheHasFree);
    }else
    {
	m = get_map();
    }
    GLOBAL_UNLOCK();

    m->addr = mmap(fd,size,
		   PROT_READ|PROT_WRITE,
		   fd == 0 ? MAP_PRIVATE | MAP_ANONYMOUS : MAP_SHARED
		   ,0, 0);
    m->size = size;
    m->fd = 0;
    m->magic = MAP_MAGIC;

    return m;
}


void
do_unmap(map *m)
{
    int ret;
    map_poll * p = 0;
    unsigned int * bp;
    int mapIdx = 0;

    ret = munmap(m->addr,m->size);
    p = GET_POLL(m);
    bp = get_bitmap(p);
    mapIdx = GET_MAP_IDX(m);

    while(mapIdx > 31)
    {
	mapIdx -= 32;
	bp += 1;
    }

    GLOBAL_LOCK();

    pollHead.free++;
    CLE_BIT((*bp),mapIdx);
    if(unlikely(!cacheHasFree))
	cacheHasFree = p;

    GLOBAL_UNLOCK();
}









