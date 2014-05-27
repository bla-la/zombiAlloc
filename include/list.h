#ifndef __LIST_H__
#define __LIST_H__
#define unlikely(x) __builtin_expect(!!(x), 0)
#define likely(x) __builtin_expect(!!(x), 1)

#define list_next(list,entry,type)              \
    ({ type * __ret__ = NULL;                   \
    if(!entry)                                  \
    {					        \
        __ret__ = list->first;                  \
    }                                           \
    else                                        \
    {                                           \
        __ret__ = entry->next;                  \
    }                                           \
    __ret__;})

#define list_prev(list,entry,type)              \
    ({ type * __ret__ = NULL;                   \
    if(!entry)                                  \
    {                                           \
       __ret__ = list->last;                    \
    }else                                       \
    {                                           \
       __ret__ = entry->prev;                   \
    }                                           \
    __ret__;})

/*
#define list_entry_up(list,entry,type)          \
    (
    )
#define list_entry_down(list,entry,type)        \
    ()
#define list_insert(list,entry,after,type)      \
    ()
*/

#define list_append(list,entry,type)            \
    ({ type * __last__ = (list)->last;		\
	entry->prev = 0;			\
	entry->next = 0;			\
    if(likely(__last__)){			\
      __last__->next = entry;                   \
      entry->prev = __last__;                   \
    }else{                                      \
	(list)->first = entry;			\
    }                                           \
        (list)->last = entry;			\
     })

#define list_push(list,entry,type)              \
    (;)
#define list_pop(list,type)                     \
({                                              \
    type * __first__ = list->first;             \
    if(likely(__first__))			\
    {                                           \
	type * __next__ = __first__->next;      \
	__first__->next = 0;                    \
	if(__next__)                            \
	  __next__->prev = 0;                   \
	list->first = __next__;                 \
	if(list->last == __first__)             \
	    list->last = __next__;              \
    }                                           \
    __first__;                                  \
 })
#define list_rm(list,entry,type)                \
    ({;})

#endif
