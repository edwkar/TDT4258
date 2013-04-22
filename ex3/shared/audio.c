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

static void open_dsp_or_die(void);
static void set_dsp_options(void);
static void *audio_run(void *path);
static void audio_stop_cur(void);

static pthread_t play_thread;
static uint8_t audio_buf[AUDIO_BUF_SIZE];

static volatile bool has_played = false;
static volatile bool should_stop = false;

static int dsp_fd = -1;

void audio_init(void)
{
    open_dsp_or_die();
    set_dsp_options();
}

void audio_cleanup(void)
{
    assert(dsp_fd != -1);

    audio_stop_cur();

    if (close(dsp_fd) != 0)
        perror("close(dsp_fd)");
    dsp_fd = -1;
}

void audio_play(const char *filepath)
{
    assert(dsp_fd != -1);

    if (has_played)
        pthread_cancel(play_thread);
        //audio_stop_cur();
    has_played = true;

    if (pthread_create(&play_thread, NULL, audio_run, (void*) filepath) != 0)
        DIE_HARD("pthread_create");
}

static void audio_stop_cur(void)
{
    should_stop = true;

    void *retval;
    if (pthread_join(play_thread, &retval) != 0)
        DIE_HARD("pthread_join");

    should_stop = false;
}

static void *audio_run(void *path)
{
    int audio_fd = open_or_die(path, O_RDONLY);

    while (!should_stop) {
        ssize_t num_read = read(audio_fd, audio_buf,
                                    sizeof audio_buf);
        if (num_read < 0)
            DIE_HARD("read");

        if (num_read == 0)
            break;

        ssize_t num_written = 0;
        while (!should_stop && num_written < num_read) {
            ssize_t num_wr = write(dsp_fd, audio_buf+num_written,
                                       (size_t) (num_read-num_written));
            fprintf(stderr, "wrote %s\n", (char*)path);
            if (num_wr < 0)
                DIE_HARD("write");
            num_written += num_wr;
        }
    }

    if (close(audio_fd) != 0)
        perror("close(audio_fd)");

    puts("closing audio");
    pthread_exit(NULL);
}

static void open_dsp_or_die(void)
{
    for (int num_attempts = 0; num_attempts < 5; ++num_attempts) {
        dsp_fd = open(DSP_PATH, O_WRONLY);
        if (dsp_fd >= 0)
            return;
        else
            sleep(1);
    }

    DIE_HARD("open");
}

static void set_dsp_options(void)
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
