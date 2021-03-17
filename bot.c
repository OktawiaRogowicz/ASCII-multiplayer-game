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
    
  int fd2 = shm_open("shared_m2", O_CREAT | O_RDWR, 0600);
  ftruncate(fd2, sizeof(struct number_of_processes));
  struct number_of_processes *n_o_p = mmap(NULL, sizeof(struct number_of_processes), PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0);

  for(int i=0; i < MAX_PLAYERS; i++)
  {
    if( n_o_p->slots[i] == EMPTY )
    {
      n_o_p->free_number = i;
      break;
    }
  }

  char list_of_semaphores[MAX_PLAYERS][30];

  sprintf(*(list_of_semaphores+0), "semaphore_in%d", n_o_p->free_number);
  sprintf(*(list_of_semaphores+1), "semaphore_out%d", n_o_p->free_number);
  sprintf(*(list_of_semaphores+2), "semaphore_movement_1%d", n_o_p->free_number);
  sprintf(*(list_of_semaphores+3), "semaphore_movement_2%d", n_o_p->free_number);

  sem_t *semaphore_in = sem_open(*(list_of_semaphores+0), O_CREAT, 0600, 0);
  sem_t *semaphore_out = sem_open(*(list_of_semaphores+1), O_CREAT, 0600, 0);
  sem_t *semaphore_movement1 = sem_open(*(list_of_semaphores+2), O_CREAT, 0600, 0);
  sem_t *semaphore_movement2 = sem_open(*(list_of_semaphores+3), O_CREAT, 0600, 0);

  char *name = (char*)realloc(NULL, sizeof(char)*30);
  sprintf(name, "shared_mem%d", n_o_p->free_number);

  int fd = shm_open(name, O_CREAT | O_RDWR, 0600);
  ftruncate(fd, sizeof(struct sight_of_player));
  struct sight_of_player *sight = mmap(NULL, sizeof(struct sight_of_player), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  if( n_o_p->number_of_proccesses != 4 )
  {
    n_o_p->number_of_proccesses += 1;
    int sd = 0;

    sem_post(semaphore_in);
    sight->player.player_type = BOT;
    while(sight->movement != QUIT)
    {
      sem_wait(semaphore_movement2);
      print_sight_of_player(sight);
      sd++;
      move_player_bot(sight);
      sem_post(semaphore_movement1);
    }
    n_o_p->number_of_proccesses -= 1;
  }
  else
    {mvprintw(0, 0, "There are four players already!");
    refresh();}

  endwin();

  munmap(sight, sizeof(struct sight_of_player));

  shm_unlink(*(list_of_semaphores+0));
  shm_unlink(*(list_of_semaphores+1));
  sem_unlink(*(list_of_semaphores+2));
  sem_unlink(*(list_of_semaphores+3));
  sem_unlink(name);

  return 0;
}