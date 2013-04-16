#include <assert.h>
#include <fcntl.h>
#include <linux/soundcard.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "audio.h"
#include "profiling.h"
#include "utils.h"


#define AUDIO_BUF_SIZE   16384

static pthread_t audio_thread;
static uint8_t audio_buf[AUDIO_BUF_SIZE];
static volatile bool should_shutdown = false;

static void *audio_run(void *);
static int open_dsp_or_die(void);
static void set_dsp_options(int dsp_fd);

void audio_init(void)
{
    if (pthread_create(&audio_thread, NULL, audio_run, NULL) != 0)
        DIE_HARD("pthread_create");
}

void audio_cleanup(void)
{
    should_shutdown = true;

    void *retval;
    if (pthread_join(audio_thread, &retval) != 0)
        DIE_HARD("pthread_join");
}

static void *audio_run(__attribute__((unused)) void *___)
{
    int dsp_fd = open_dsp_or_die();

    set_dsp_options(dsp_fd);

    int audio_fd = open_or_die("./shared/arve_demotest1.mp3.raw", O_RDONLY);

    while (!should_shutdown) {
        ssize_t num_read = read(audio_fd, audio_buf,
                                sizeof audio_buf);
        if (num_read == 0)
            break;

        if (num_read < 0)
            DIE_HARD("read");

        ssize_t num_written = 0;
        while (!should_shutdown && num_written < num_read) {
            ssize_t num_wr = write(dsp_fd, audio_buf+num_written,
                                   (size_t) (num_read-num_written));
            if (num_wr < 0)
                DIE_HARD("write");
            num_written += num_wr;
        }
    }

    if (close(dsp_fd)   != 0) perror("close(dsp_fd)");
    if (close(audio_fd) != 0) perror("close(audio_fd)");

    pthread_exit(NULL);
}

static int open_dsp_or_die(void)
{
    for (int num_attempts = 0; num_attempts < 5; ++num_attempts) {
        int dsp_fd = open(DSP_PATH, O_WRONLY);
        if (dsp_fd >= 0)
            return dsp_fd;
        else
            sleep(1);
    }

    DIE_HARD("open");
}

static void set_dsp_options(int dsp_fd)
{
#ifdef HOST_BUILD
    if (dsp_fd != -42) // Silence the compiler...
        return;
#endif

    int dsp_rate = 22100;
    if (ioctl(dsp_fd, SNDCTL_DSP_SPEED, &dsp_rate) != 0)
        DIE_HARD("ioctl");

    int dsp_fmt = AFMT_U8;
    if (ioctl(dsp_fd, SNDCTL_DSP_SETFMT, &dsp_fmt) != 0)
        DIE_HARD("ioctl");

    int num_chans = 2;
    if (ioctl(dsp_fd, SNDCTL_DSP_CHANNELS, &num_chans) != 0)
        DIE_HARD("ioctl");
}
