#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
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
#define NUM_SUB_SYSTEMS 3

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
    for (unsigned int i = 0; i < NUM_SUB_SYSTEMS; ++i)
        if (pthread_create(&threads[i], NULL, SUBSYS_ROUTINES[i], NULL) != 0)
            DIE_HARD("pthread_create");

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
    for (unsigned int i = 0; i < NUM_SUB_SYSTEMS; ++i)
        if (pthread_cancel(threads[i]) != 0)
            DIE_HARD("pthread_cancel");

    if (system("rm " FRAME_BUFFER_PATH "") != 0)  DIE_HARD("rm");
    if (system("rm " DSP_PATH "") != 0) DIE_HARD("rm");
    if (system("rm " INPUT_DRIVER_PATH) != 0) DIE_HARD("rm");

    return EXIT_SUCCESS;
}

static void *run_mock_fb(__attribute__((unused)) void *___)
{
    if (system("rm -f " FRAME_BUFFER_PATH "") != 0)
        DIE_HARD("rm");
    if (system("dd if=/dev/zero of=" FRAME_BUFFER_PATH
               "  bs=1M count=2") != 0)
        DIE_HARD("dd");

    SDL_Surface *screen = SDL_SetVideoMode(FB_WIDTH, FB_HEIGHT, 32,
                                           SDL_SWSURFACE);

    int fb_file = open_or_die(FRAME_BUFFER_PATH, O_RDWR);

    struct pixel *fb = mmap(NULL, FB_SIZE_BYTES, PROT_READ | PROT_WRITE,
                            MAP_SHARED, fb_file, 0);
    if (fb == MAP_FAILED)
        DIE_HARD("mmap");

    puts("Mock framebuffer ready.");

    for (;;) {
        if (SDL_MUSTLOCK(screen))
            if (SDL_LockSurface(screen) < 0)
                DIE_HARD("SDL_LockSurface");

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
        DIE_HARD("rm");
    if (system("mkfifo " DSP_PATH) != 0)
        DIE_HARD("mkfifo");

    puts("Mock DSP (almost) ready.");

    if (system("cat " DSP_PATH " | aplay -r 22100 -c 2") != 0)
        DIE_HARD("");

    return NULL;
}

static void *run_mock_input(__attribute__((unused)) void *___) {
    static const int keys_in_use[] = {
        SDLK_a, SDLK_s, SDLK_d, SDLK_f,
        SDLK_g, SDLK_h, SDLK_j, SDLK_k };

    if (system("rm -f " INPUT_DRIVER_PATH) != 0)
        DIE_HARD("rm");

    int fd = open(INPUT_DRIVER_PATH,
                  O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    if (fd < 0)
        DIE_HARD("Failed to open " INPUT_DRIVER_PATH);

    for (;;) {
        uint8_t *key_state = SDL_GetKeyState(NULL);

        unsigned char v = 0;
        for (unsigned int i = 0; i < 8; ++i)
            if (key_state[keys_in_use[i]])
                v |= 1U << i;

        if (lseek(fd, 0, SEEK_SET) != 0)
            DIE_HARD("lseek");
        if (write(fd, &v, sizeof v) != sizeof v)
            DIE_HARD("write");

        usleep(1000);
    }
}
