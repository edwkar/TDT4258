#include <assert.h>
#include <pthread.h>

#include "profiling.h"


#define MAX_PROFILING_POINTS 256

static const struct m_profiling_data *profiling_points[MAX_PROFILING_POINTS];
static uint32_t num_profiling_points = 0;

static pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

void m_profiling_register(struct m_profiling_data *pd)
{
    assert(pthread_mutex_lock(&mut) == 0);

    assert(num_profiling_points < MAX_PROFILING_POINTS);
    assert(!pd->is_registered);

    profiling_points[num_profiling_points++] = pd;
    pd->is_registered = 1;

    assert(pthread_mutex_unlock(&mut) == 0);
}

void profiling_report(void)
{
    assert(pthread_mutex_lock(&mut) == 0);

    puts("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    for (uint32_t i = 0; i < num_profiling_points; ++i)
        printf("  %-28s %u ms\n",
               profiling_points[i]->name,
               (unsigned int) profiling_points[i]->sum_time_us/1000);

    puts("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    assert(pthread_mutex_unlock(&mut) == 0);
}
