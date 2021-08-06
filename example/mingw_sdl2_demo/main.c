#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "rte_include.h"
#define SDL_MAIN_HANDLED
#include "SDL.h"

extern void test_log(void);
extern void test_mempool(int thread_num, int max_malloc_size, int count, int loop, bool if_verbose);
extern void test_vector(void);
extern void test_ringbuffer(void);

static void running_timer(void *param)
{
    RTE_LOGI("system running!");
}

static int main_thread(void *param)
{
//   test_log();
//    memory_demon(BANK_DEFAULT);
//    test_mempool(10, 1024 * 1024, 100, 10, true);
//    memory_demon(BANK_DEFAULT);
//    test_vector();
//    test_ringbuffer();
    rte_init();
    RTE_LOGI("Hello world!");
    timer_id_t running_timer_id = 0;
    timer_configuration_t timer_config = TIMER_CONFIG_INITIALIZER;
    timer_config.repeat_period_tick = 2000;
    timer_config.timer_callback = running_timer;
    timer_create_new(rte_get_main_timergroup(), &timer_config, &running_timer_id);
    for (;;) {
        timer_tick_handle();
        SDL_Delay(1);
    }
    return 0;
}

int main(int argc, char *argv[])
{
	//Initialize SDL
	if( SDL_Init( SDL_INIT_EVERYTHING ) < 0 )
	{
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		return -1;
	}
	printf( "SDL initialize successfully!\n");
    SDL_Thread *main_thread_handle = SDL_CreateThread(main_thread, "main_thread", NULL);
    SDL_WaitThread(main_thread_handle, NULL);
    return 0;
}
