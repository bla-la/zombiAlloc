#ifndef __MAPS_H__
#define __MAPS_H__

#define unlikely(x) __builtin_expect(!!(x), 0)
#define likely(x) __builtin_expect(!!(x), 1)

typedef struct __map map;
struct __map
{
    size_t size;
    void * addr;
    int fd;
    int magic;
    void * reserved;
}__attribute__ ((aligned (16)));

/*
#define ffz(bits) \
({ \
    int mask = 0x1; \
    int i = -1; \
    if( !(bits & 0xffffffff)) \
    { i = 0; \
    while( (bits & mask)) \
    { \
	mask = mask << 1; \
	i++; \
    } \
    }\
    i;\
})
*/

void dumpAllocSettiongs();
map * do_map(size_t size,int fd);
void do_unmap(map *m);

#endif
