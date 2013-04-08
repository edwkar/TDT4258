#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <linux/soundcard.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "audio.h"
#include "utils.h"

#define AUDIO_BUF_SIZE   (1024 << 5)

static pthread_t audio_thread;
static void *audio_run(void *);
static uint8_t audio_buf[AUDIO_BUF_SIZE];
static volatile bool should_shutdown = false;

void audio_init(void) {
    if (pthread_create(&audio_thread, NULL, audio_run, NULL) != 0)
        die_hard("pthread_create");
}

void audio_cleanup(void) {
    should_shutdown = true;
    void *retval;
    if (pthread_join(audio_thread, &retval) != 0)
        die_hard("pthread_join");
}

static void *audio_run(__attribute__((unused)) void *___)
{
    printf("Trying to open DSP... ");
    int dsp_file = open(DSP_PATH, O_WRONLY);
    if (dsp_file < 0) {
        perror("dsp open");
        exit(EXIT_FAILURE);
    }
    printf("[DSP opened]\n");

#ifndef HOST_BUILD
    int dsp_rate = 22100;
    if (ioctl(dsp_file, SNDCTL_DSP_SPEED, &dsp_rate) != 0)
        die_hard("ioctl");

    int dsp_fmt = AFMT_U8;
    if (ioctl(dsp_file, SNDCTL_DSP_SETFMT, &dsp_fmt) != 0)
        die_hard("ioctl");

    int yes = 2;
    if (ioctl(dsp_file, SNDCTL_DSP_CHANNELS, &yes) != 0)
        die_hard("ioctl");
#endif
    int audio_file = open("./shared/arve_demotest1.mp3.raw", O_RDONLY);
    if (audio_file < 0)
        die_hard("open");

    while (!should_shutdown) {
        ssize_t num_read = read(audio_file, audio_buf,
                                sizeof audio_buf);
        if (num_read == 0)
            break;

        if (num_read < 0)
            die_hard("read");

        ssize_t num_written = 0;
        while (!should_shutdown && num_written < num_read) {
            int num_wr = write(dsp_file, audio_buf+num_written,
                               num_read-num_written);
            if (num_wr < 0)
                die_hard("write");
            num_written += num_wr;
        }
    }

    if (close(dsp_file)   != 0) perror("close(dsp_file)");
    if (close(audio_file) != 0) perror("close(audio_file)");

    pthread_exit(NULL);
}
