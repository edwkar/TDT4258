#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include "SDL/SDL.h"
#include "screen.h"
#include "utils.h"

static void *run_mock_fb(void *);
static void *run_mock_dsp(void *);
static void *run_mock_input(void *);

static void *(*const SUBSYS_ROUTINES[]) (void *) = {
    run_mock_fb,
    run_mock_dsp,
    run_mock_input
};
static const int NUM_SUB_SYSTEMS = 3;

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [command]\n", argv[0]);
        return EXIT_FAILURE;
    }

    SDL_Init(SDL_INIT_EVERYTHING);

    pthread_t threads[NUM_SUB_SYSTEMS];

    /* Start subsystems.
     */
    for (int i = 0; i < NUM_SUB_SYSTEMS; ++i)
        if (pthread_create(&threads[i], NULL, SUBSYS_ROUTINES[i], NULL) != 0)
            die_hard("pthread_create");

    /* Wait for subsystems to get ready. (Hopefully...)
     */
    sleep(1);

    /* Wrap.
     */
    printf("Wrapping '%s'...\n", argv[1]);
    int wrapped_ret = system(argv[1]);
    printf("Wrapped exited with return code %d, quitting...\n", wrapped_ret);

    /* Kill subsystems.
     */
    for (int i = 0; i < NUM_SUB_SYSTEMS; ++i)
        if (pthread_cancel(threads[i]) != 0)
            die_hard("pthread_cancel");

    if (system("rm " FRAME_BUFFER_PATH "") != 0)  die_hard("rm");
    if (system("rm " DSP_PATH "") != 0) die_hard("rm");
    if (system("rm " INPUT_DRIVER_PATH) != 0) die_hard("rm");

    return EXIT_SUCCESS;
}


static void *run_mock_fb(__attribute__((unused)) void *___)
{
    if (system("rm -f " FRAME_BUFFER_PATH "") != 0)
        die_hard("rm");
    if (system("dd if=/dev/zero of=" FRAME_BUFFER_PATH
               "  bs=1M count=2") != 0)
        die_hard("dd");

    SDL_Surface *screen = SDL_SetVideoMode(FB_WIDTH, FB_HEIGHT, 32,
                                           SDL_SWSURFACE);

    int fb_file = open(FRAME_BUFFER_PATH, O_RDWR);
    if (fb_file < 0)
        die_hard("open");

    struct pixel *fb = mmap(NULL, FB_SIZE_BYTES, PROT_READ | PROT_WRITE,
                            MAP_SHARED, fb_file, 0);
    if (fb == MAP_FAILED)
        die_hard("mmap");

    puts("Mock framebuffer ready.");

    for (;;) {
        if (SDL_MUSTLOCK(screen))
            if (SDL_LockSurface(screen) < 0)
                die_hard("SDL_LockSurface");

        for (unsigned int i = 0; i < FB_SIZE; ++i) {
            unsigned int v = (((unsigned int) fb[i].red)   << 16) |
                             (((unsigned int) fb[i].green) <<  8) |
                             ( (unsigned int) fb[i].blue);
            ((unsigned int*)screen->pixels)[i] = v;
        }

        if (SDL_MUSTLOCK(screen))
            SDL_UnlockSurface(screen);

        SDL_UpdateRect(screen, 0, 0, FB_WIDTH, FB_HEIGHT);
        SDL_PumpEvents();

        usleep(1000);
    }
}

static void *run_mock_dsp(__attribute__((unused)) void *___)
{
    if (system("rm -f " DSP_PATH) != 0)
        die_hard("rm");
    if (system("mkfifo " DSP_PATH) != 0)
        die_hard("mkfifo");

    puts("Mock DSP (almost) ready.");

    if (system("cat " DSP_PATH " | aplay -r 22100 -c 2") != 0)
        die_hard("");

    return NULL;
}

static void *run_mock_input(__attribute__((unused)) void *___) {
    static const int keys_in_use[] = {
        SDLK_a, SDLK_s, SDLK_d, SDLK_f,
        SDLK_g, SDLK_h, SDLK_j, SDLK_k };

    if (system("rm -f " INPUT_DRIVER_PATH) != 0)
        die_hard("rm");

    int fd = open(INPUT_DRIVER_PATH,
                  O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    if (fd < 0)
        die_hard("open");

    for (;;) {
        uint8_t *key_state = SDL_GetKeyState(NULL);

        unsigned char v = 0;
        for (unsigned int i = 0; i < 8; ++i)
            if (key_state[keys_in_use[i]])
                v |= 1U << i;

        if (lseek(fd, 0, SEEK_SET) != 0)
            die_hard("lseek");
        if (write(fd, &v, sizeof v) != sizeof v)
            die_hard("write");

        usleep(1000);
    }
}
