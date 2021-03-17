#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include "project1.h"
#include <ncurses.h>
#include <stdint.h>

pthread_mutex_t mutex;

int main()
{

  char list_of_semaphores[MAX_PLAYERS * 4][30];
  for(int i=0; i< MAX_PLAYERS * 4; i++)
    {
      if( i% 4 == 0)
        sprintf(*(list_of_semaphores+i), "semaphore_in%d", i / 4);
      else if( i% 4 == 1)
        sprintf(*(list_of_semaphores+i), "semaphore_out%d", i / 4);
      else if( i% 4 == 2)
        sprintf(*(list_of_semaphores+i), "semaphore_movement_1%d", i / 4);
      else if( i% 4 == 3)
        sprintf(*(list_of_semaphores+i), "semaphore_movement_2%d", i / 4);
    }

  initscr();
  noecho();
  curs_set(FALSE);

  srand(time(NULL));

  start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
  init_pair(2, COLOR_BLACK, COLOR_WHITE);
  init_pair(3, COLOR_WHITE, COLOR_MAGENTA);
  init_pair(4, COLOR_WHITE, COLOR_YELLOW);
  init_pair(5, COLOR_WHITE, COLOR_RED);
  init_pair(6, COLOR_WHITE, COLOR_GREEN);
    
  sem_t *semaphore_in[MAX_PLAYERS];
  sem_t *semaphore_out[MAX_PLAYERS];
  sem_t *semaphore_movement1[MAX_PLAYERS];
  sem_t *semaphore_movement2[MAX_PLAYERS];

  for(int i=0; i<MAX_PLAYERS * 4; i=i+4)
  {
    semaphore_in[i/4] = sem_open(*(list_of_semaphores+i), O_CREAT, 0600, 0);
    semaphore_out[i/4] = sem_open(*(list_of_semaphores+i+1), O_CREAT, 0600, 0);  
    semaphore_movement1[i/4] = sem_open(*(list_of_semaphores+i+2), O_CREAT, 0600, 0);
    semaphore_movement2[i/4] = sem_open(*(list_of_semaphores+i+3), O_CREAT, 0600, 0);
  }

  int fd[MAX_PLAYERS];
  struct sight_of_player *sight[MAX_PLAYERS];

  char list_of_shared_mem[MAX_PLAYERS][30];
  for(int i=0; i<MAX_PLAYERS; i++)
  {
    sprintf(*(list_of_shared_mem+i), "shared_mem%d", i);
    *(fd+i) = shm_open(*(list_of_shared_mem+i), O_CREAT | O_RDWR, 0600);

    ftruncate(*(fd+i), sizeof(struct sight_of_player));
    sight[i] = mmap(NULL, sizeof(struct sight_of_player), PROT_READ | PROT_WRITE, MAP_SHARED, *(fd+i), 0);
  }

  int fd2 = shm_open("shared_m2", O_CREAT | O_RDWR, 0600);
  ftruncate(fd2, sizeof(struct number_of_processes));
  struct number_of_processes *n_o_p = mmap(NULL, sizeof(struct number_of_processes), PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0);

  struct map_info g;
  struct map_info *game = &g;

  game = generate_game(game, n_o_p);
  create_camp(game);
  for(int i=0; i<20; i++)
    create_coin(game);

  print_map(game);

  struct threads_to_unlock t;
  struct thread_move_t thread_m[MAX_PLAYERS];

  struct deaths_t checking_deaths;
  checking_deaths.game = game;

  t.game = game;
  for(int i=0; i<MAX_PLAYERS; i++)
  {
    (thread_m+i)->game = game;
    (thread_m+i)->sight = sight[i];
    (thread_m+i)->semaphore_movement1 = semaphore_movement1[i];
    (thread_m+i)->semaphore_movement2 = semaphore_movement2[i];
    (thread_m+i)->semaphore_in = semaphore_in[i];
    pthread_mutex_init(&(thread_m+i)->mutex, NULL);
    t.threadstounlock[i] = (thread_m+i);
    t.beaststounlock[i] = NULL;
  }
  pthread_mutex_init(&checking_deaths.mutex, NULL);
  t.deadthstounlock = &checking_deaths;

  pthread_t thread[MAX_PLAYERS];
  pthread_t t_for_unlocking, t_for_deaths;

  for(int i=0; i<MAX_PLAYERS; i++)
    pthread_create((thread+i), NULL, thread_move, thread_m+i);
  pthread_create(&t_for_deaths, NULL, check_deaths, &checking_deaths);
    
  pthread_create(&t_for_unlocking, NULL, thread_unlock, &t);


  spawn_anything_server(&t);

  endwin();
  destroy_game(game);
  
  munmap(n_o_p, sizeof(struct number_of_processes));  

  for(int i=0; i<MAX_PLAYERS; i++)
  {
    sem_close(*(semaphore_out+i));
    sem_close(*(semaphore_in+i));
    sem_close(*(semaphore_movement1+i));
    sem_close(*(semaphore_movement2+i));
  }
  return 0;
}
