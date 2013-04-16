#ifndef __PROFILING_H
#define __PROFILING_H

#include <stdbool.h>
#include <stdint.h>

#include "utils.h"


/* PRIVATE */ struct _profiling_data {
    char name[64];
    uint64_t sum_time_us;
    uint32_t num_calls;
    uint32_t last_time_us;
    int is_registered;
};

/* PRIVATE */ void _profiling_register(struct _profiling_data *pd);


/* PUBLIC */ void profiling_report(void);


#ifdef DISABLE_PROFILING

#define PROFILING_SETUP(NAME) 
#define PROFILING_ENTER(NAME)
#define PROFILING_EXIT(NAME) 

#else

#define _PROF_TIME            get_time_us

#define PROFILING_SETUP(NAME) \
    static struct _profiling_data _prof__##NAME = { #NAME, 0, 0, 0, 0 }

#define PROFILING_ENTER(NAME) do {\
        if (__builtin_expect((_prof__##NAME).is_registered == 0, 0))\
            _profiling_register(&_prof__##NAME);\
        (_prof__##NAME).num_calls++;\
        (_prof__##NAME).last_time_us = _PROF_TIME();\
    } while (false)

#define PROFILING_EXIT(NAME)  \
    ((_prof__##NAME).sum_time_us +=\
      _PROF_TIME() - ((_prof__##NAME).last_time_us))

#endif


#endif
