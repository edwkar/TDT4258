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


#define AUDIO_BUF_SIZE   1024
static uint8_t audio_buf[AUDIO_BUF_SIZE];

static int open_dsp_or_die(void);
static void set_dsp_options(int);
static void *audio_run(void *);
static size_t audio_advance(int, int);

static pthread_t play_thread;

static pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
static bool should_stop = false;
static const char *next_to_play = NULL;

void audio_init(void)
{
    SYNCHRONIZED(mut,
        if (pthread_create(&play_thread, NULL, audio_run, NULL) != 0)
            DIE_HARD("pthread_create");
    );
}

void audio_cleanup(void)
{
    SYNCHRONIZED(mut,
            should_stop = true;
    );

    void *retval;
    if (pthread_join(play_thread, &retval) != 0)
        DIE_HARD("pthread_join");
}

void audio_play(const char *filepath)
{
    SYNCHRONIZED(mut,
        next_to_play = filepath;
    );
}

static void *audio_run(void __attribute__((unused)) *___)
{
    int dsp_fd = open_dsp_or_die();
    set_dsp_options(dsp_fd);

    int audio_fd = -1;

    for (;;) {
        /* This is a little convoluted.
         * We are checking if we should break, or play a different song.
         */
        SYNCHRONIZED(mut,
            if (should_stop)
                break;

            if (next_to_play != NULL) {
                if (audio_fd != -1) {
                    if (close(audio_fd) != 0)
                        perror("close(audio_fd)");
                }

                puts(next_to_play);
                audio_fd = open(next_to_play, O_RDONLY);
                next_to_play = NULL;
            }
        );

        if (audio_fd == -1 || audio_advance(dsp_fd, audio_fd) == 0)
            usleep(1000);
    }

    if (audio_fd != -1)
        if (close(audio_fd) != 0)
            perror("close(audio_fd)");

    if (close(dsp_fd) != 0)
        perror("close(dsp_fd)");

    puts("closing audio");
    pthread_exit(NULL);
}

static size_t audio_advance(int dsp_fd, int audio_fd)
{
    ssize_t num_read = read(audio_fd, audio_buf, sizeof audio_buf);
    if (num_read < 0)
        DIE_HARD("read");

    if (num_read == 0)
        return 0;

    ssize_t num_written = 0;
    while (num_written < num_read) {
        ssize_t num_wr = write(dsp_fd, audio_buf+num_written,
                               (size_t) (num_read-num_written));
        if (num_wr < 0)
            DIE_HARD("write");
        num_written += num_wr;
    }

    return (size_t) num_read;
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
