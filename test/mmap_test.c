#include <stdio.h>
#include <maps.h>

#define DOMAP 21835 * 2
map * ptr[DOMAP];

int main()
{
    printf("Test map/unmap memory region\n");
    printf("-=ALLOCATOR SETTINGS=-\n");
    //    dumpAllocSettiongs();

    map *m1 = do_map(32768,0);
    map *m = do_map(32768,0);
    do_unmap(m);
    do_unmap(m1);
    int i =0;
    int j = 0;
    for(j=0;j < 10000;j++)
    {
	for(i=0;i<257;i++)
	{
	    ptr[i] = do_map(32768,0);
	}


	for(i=0;i<257;i++)
	{
	    do_unmap(ptr[i]);
	}
    }

    //while(1);
}
