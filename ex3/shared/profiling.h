#ifndef PROFILING_H
#define PROFILING_H

#include <stdbool.h>
#include <stdint.h>

#include "utils.h"


/* PRIVATE */ struct m_profiling_data {
    char name[64];
    uint64_t sum_time_us;
    uint32_t num_calls;
    uint32_t last_time_us;
    int is_registered;
};

/* PRIVATE */ void m_profiling_register(struct m_profiling_data *pd);


/* PUBLIC */ void profiling_report(void);


#ifdef DISABLE_PROFILING

#define PROFILING_SETUP(NAME) 
#define PROFILING_ENTER(NAME)
#define PROFILING_EXIT(NAME) 

#else

#define m_PROF_TIME            get_time_us

#define PROFILING_SETUP(NAME) \
    static struct m_profiling_data m_prof##NAME = { #NAME, 0, 0, 0, 0 }

#define PROFILING_ENTER(NAME) do {\
        if (__builtin_expect((m_prof##NAME).is_registered == 0, 0))\
            m_profiling_register(&m_prof##NAME);\
        (m_prof##NAME).num_calls++;\
        (m_prof##NAME).last_time_us = m_PROF_TIME();\
    } while (false)

#define PROFILING_EXIT(NAME)  \
    ((m_prof##NAME).sum_time_us +=\
      m_PROF_TIME() - ((m_prof##NAME).last_time_us))

#endif


#endif
