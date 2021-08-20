#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <conio.h>
#include "rte_include.h"
#define SDL_MAIN_HANDLED
#include "SDL.h"
#include "shell.h"

extern void test_log(void);
extern void test_mempool(int thread_num, int max_malloc_size, int count, int loop, bool if_verbose);
extern void test_vector(void);
extern void test_ringbuffer(void);
extern void test_hashmap(void);
extern void test_linklist(void);
extern int gui_thread(void *param);

static bool running_flag = true;

void end_main_loop(void)
{
    running_flag = false;
}

static int main_thread(void *param)
{
    // test_log();
    // test_vector();
    // test_ringbuffer();
    rte_init();
    memory_demon(BANK_DEFAULT);
    test_hashmap();
    test_linklist();
    memory_demon(BANK_DEFAULT);
    // memory_demon(BANK_DEFAULT);
    // test_mempool(10, 1024 * 1024, 100, 10, true);
    // memory_demon(BANK_DEFAULT);
    RTE_LOGI("Hello world!");
    //SDL_Thread *gui_thread_handle = SDL_CreateThread(gui_thread, "gui_thread", NULL);
    while(running_flag) {
        timer_tick_handle();
        SDL_Delay(1);
    }
    rte_deinit();
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
	//Quit SDL subsystems
	SDL_Quit();
    return 0;
}
