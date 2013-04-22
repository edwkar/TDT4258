#ifndef MOVIE_H
#define MOVIE_H

#include <stdint.h>

#include "ratekeeper.h"


void movie_play(const char *path, struct rate_keeper *rk);

#endif
