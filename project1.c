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

struct map_info *generate_game(struct map_info *game, struct number_of_processes *n_o_p){

  n_o_p->number_of_proccesses = 0;
  n_o_p->free_number = 0;
  for(int i=0; i<MAX_PLAYERS; i++)
      n_o_p->slots[i] = EMPTY;

  generate_map(game);
  game->number_of_players = 0;
  game->number_of_beasts = 0;
  game->n_o_p = n_o_p;
  return game;
}

void generate_map(struct map_info *game){

  FILE *f = fopen("labirynt.txt", "r");

  enum one_place **map= (enum one_place**)realloc(NULL, sizeof(enum one_place*)*MAP_SIZE_Y);
  int **int_map = (int**)realloc(NULL, sizeof(int*)*MAP_SIZE_Y);
  for(int i=0; i<MAP_SIZE_Y; i++)
  {
    enum one_place *line= (enum one_place*)realloc(NULL, sizeof(enum one_place)*MAP_SIZE_X);
    *(map+i) = line;
    int *int_line = (int*)realloc(NULL, sizeof(int)*MAP_SIZE_X);
    *(int_map+i) = int_line;
  }

  for(int i=0; i<MAP_SIZE_Y; i++)
    for(int j=0; j<MAP_SIZE_X; j++)
    {
        *(*(int_map+i)+j) = 0;
      
      char ch = fgetc(f);
      if(ch == 'x')
        *(*(map+i)+j) = WALL;
      else if( ch == ' ')
        *(*(map+i)+j) = FREE_SPACE;
      else if( ch == '#')
        *(*(map+i)+j) = BUSH;
      else if( ch == '\n')
        *(*(map+i)+j) = NEW_LINE;
    }
  game->map = map;
  game->int_map = int_map;
}

void print_map_elements (enum one_place place, int i, int j, int value){

  if( place == WALL)
    {
      attron(COLOR_PAIR(1));
      mvprintw(i, j, " ");
      attroff(COLOR_PAIR(1));
    }
  else if( place == FREE_SPACE )
    {
      attron(COLOR_PAIR(2));
      mvprintw(i, j, " ");
      attron(COLOR_PAIR(2));
    }
  else if( place == COIN)
    {
      attron(COLOR_PAIR(4));
      if(value == 1)
        mvprintw(i, j, "c");
      else if(value == 10)
        mvprintw(i, j, "t");
      else if(value == 50)
        mvprintw(i, j, "T");
      else
        mvprintw(i, j, "D");
      attroff(COLOR_PAIR(4));
    }
  else if( place == CAMP)
    {
      attron(COLOR_PAIR(6));
      mvprintw(i, j, " ");
      attroff(COLOR_PAIR(6));
    }
  else if( place == BUSH)
    {
      attron(COLOR_PAIR(2));
      mvprintw(i, j, "#");
      attroff(COLOR_PAIR(2));
    }
}

void print_players_beasts(struct map_info *game){
  for(int i=0; i < MAX_PLAYERS; i++)
      {
        if( game->n_o_p->slots[i] == TAKEN )
        {
          attron(COLOR_PAIR(3));
          mvprintw(game->players[i]->pos_y, game->players[i]->pos_x, "%d", i);
          attroff(COLOR_PAIR(3));
        }
      }
  for(int i=0; i<game->number_of_beasts; i++)
      {
        attron(COLOR_PAIR(5));
        mvprintw(game->beasts[i].y, game->beasts[i].x, " ");
        attroff(COLOR_PAIR(5));
      }
}

void print_map(struct map_info *game){

  int y=0, x=0;
  pthread_mutex_lock(&mutex);
  for(int i=0; i<MAP_SIZE_Y; i++)
    for(int j=0; j<MAP_SIZE_X; j++)
    {
      print_map_elements(*(*(game->map+i)+j), i, j, *(*(game->int_map+i)+j));
    }
  print_players_beasts(game);
  mvprintw(9, MAP_SIZE_X, "Player | Carried | Points | Deaths | ");
  refresh();
  pthread_mutex_unlock(&mutex);

  pthread_mutex_lock(&mutex);
  for(int i=0; i < MAX_PLAYERS; i++)
  {
    if( game->n_o_p->slots[i] == TAKEN )
      mvprintw(10+i, MAP_SIZE_X, "   %d    |    %d    |    %d   |    %d   | ", i, game->players[i]->carried, game->players[i]->points, game->players[i]->deaths);
  }

  refresh();
  pthread_mutex_unlock(&mutex);
}

void update_sight_of_player(struct map_info *game, struct sight_of_player* sight){

  for(int i=0; i<5; i++)
  {
    for(int j=0; j<5; j++)
      {
        if (game->players[sight->number_of_player]->pos_y+i-2 < 0 || game->players[sight->number_of_player]->pos_y+i-2 >= MAP_SIZE_Y || 
            game->players[sight->number_of_player]->pos_x+j-2 < 0 || game->players[sight->number_of_player]->pos_x+j-2 >= MAP_SIZE_X)
          sight->sight_map[i][j] = WALL; 
        else
        {
          enum one_place checked= game->map[game->players[sight->number_of_player]->pos_y+i-2][game->players[sight->number_of_player]->pos_x+j-2];
          sight->sight_map[i][j] = checked;

          if(checked == FREE_SPACE || checked == COIN || checked == BUSH)
            sight->map[game->players[sight->number_of_player]->pos_y+i-2][game->players[sight->number_of_player]->pos_x+j-2] = FREE_SPACE;

            sight->int_map[i][j] = game->int_map[game->players[sight->number_of_player]->pos_y+i-2][game->players[sight->number_of_player]->pos_x+j-2];

          for(int k=0; k<game->number_of_beasts; k++)
            if( game->players[sight->number_of_player]->pos_y+i-2 == game->beasts[k].y && game->players[sight->number_of_player]->pos_x+j-2 == game->beasts[k].x )
                sight->sight_map[i][j] = BEAST;

            for(int k=0; k < MAX_PLAYERS; k++)
              if( game->n_o_p->slots[k] == TAKEN && game->players[sight->number_of_player]->pos_y+i-2 == game->players[k]->pos_y && game->players[sight->number_of_player]->pos_x+j-2 == game->players[k]->pos_x )
                sight->sight_map[i][j] = PLAYER0+k;
        }
      }
  }
}

int is_free(struct map_info *game, struct coords *c){
  for(int i=0; i < MAX_PLAYERS; i++)
    if( game->n_o_p->slots[i] == TAKEN && c->x == game->players[i]->pos_x && c->y == game->players[i]->pos_y )
      return 0;
  for(int i=0; i < game->number_of_beasts; i++)
    if( c->x == game->beasts[i].x && c->y == game->beasts[i].y )
      return 0;
  return 1;
}

void spawn_anything(struct map_info *game, struct coords *c){
  
  do{
    c->x = rand() % MAP_SIZE_X;
    c->y = rand() % MAP_SIZE_Y; 
  }while( *( *(game->map + c->y) + c->x) != FREE_SPACE || is_free(game, c) != 1);

}

void create_camp(struct map_info *game){
  *(*(game->map + 12 ) + 24) = CAMP;
}

void create_coin(struct map_info *game){

  struct coords c;
  spawn_anything(game, &c);
  *(*(game->map + c.y)+c.x) = COIN;

  int value = 1;
  int random = rand() % 100;
  if(random >= 80 && random < 95)
    value = 10;
  else if(value >= 95 && random < 100)
    value = 50;

  *(*(game->int_map + c.y)+c.x) = value;
}

void create_beast(struct map_info *game, struct threads_to_unlock* t){

  if(game->number_of_beasts == MAX_PLAYERS)
    return;

  struct coords c;
  spawn_anything(game, &c);

  game->beasts[game->number_of_beasts].y = c.y;
  game->beasts[game->number_of_beasts].x = c.x;
  game->beasts[game->number_of_beasts].standing_at = game->map[c.y][c.x];

  struct beast_info *beast_info = (struct beast_info*)realloc(NULL, sizeof(struct beast_info)*1);
  beast_info->game = game;
  beast_info->number_of_beast = game->number_of_beasts;
  pthread_mutex_init(&beast_info->mutex, NULL);
  t->beaststounlock[game->number_of_beasts] = beast_info;
  
  game->number_of_beasts += 1;

  pthread_t *thread = (pthread_t*)realloc(NULL, sizeof(pthread_t)*1);
  pthread_create(thread, NULL, beast_rawr, beast_info);
  //pthread_join(thread, NULL);
}

void create_player(struct map_info *game, struct sight_of_player* sight){

  struct coords c;
  spawn_anything(game, &c);

  enum one_place standing = *(*(game->map + c.y)+c.x);

  sight->player.points = 0;
  sight->player.carried = 0;
  sight->player.deaths = 0;
  sight->player.pos_x = c.x;
  sight->player.pos_y = c.y; 
  sight->player.standing_at = standing;

  sight->number_of_player = game->n_o_p->free_number;
  sight->movement = NONE;
  sight->player.player_type = NO;

  for(int i=0; i<MAP_SIZE_Y; i++)
    for(int j=0; j<MAP_SIZE_X; j++)
      *(*(sight->map+i)+j) = WALL;

  game->players[game->n_o_p->free_number] = &sight->player;
  game->number_of_players += 1;
  game->sights_of_players[game->n_o_p->free_number] = sight;

  game->n_o_p->slots[game->n_o_p->free_number] = TAKEN;

  update_sight_of_player(game, sight);
}

void* move_player_bot_thread(void* v){

  struct sight_of_player *sight_of_player = ((struct sight_of_player*)v);

  int random = rand() % 4;

  if( random == 0 )
    sight_of_player->movement = UP;
  else if ( random == 1 )
    sight_of_player->movement = DOWN;
  else if ( random == 2 )
    sight_of_player->movement = LEFT;
  else if ( random == 3 )
    sight_of_player->movement = RIGHT;

  return NULL;
}

void* move_player_bot(void* v){

  struct sight_of_player *sight_of_player = ((struct sight_of_player*)v);
  pthread_t thread;
  pthread_create(&thread, NULL, move_player_bot_thread, v);

  return NULL;
}

void* move_player_client_thread(void* v){

  struct sight_of_player *sight_of_player = ((struct sight_of_player*)v);

  char ch = getch();

  if(ch == 'w' || ch == 'W')
    sight_of_player->movement=UP;
  else if( ch == 's' || ch == 'S')
    sight_of_player->movement=DOWN;
  else if( ch == 'a' || ch == 'A')
    sight_of_player->movement=LEFT;
  else if( ch == 'd' || ch == 'D')
    sight_of_player->movement=RIGHT;
  else if( ch == 'q' || ch == 'Q' )
    {
      mvprintw(0, 0, "QUIT");
      refresh();
      sight_of_player->movement=QUIT;
    }

  return NULL;
}

void* move_player_client(void* v){

  struct sight_of_player *sight_of_player = ((struct sight_of_player*)v);
  pthread_t thread;
  pthread_create(&thread, NULL, move_player_client_thread, v);

  return NULL;
}

void moving_itself(struct map_info* game, struct sight_of_player* sight, int y, int x){
  if( game->map[sight->player.pos_y][sight->player.pos_x] != COIN )
    game->map[sight->player.pos_y][sight->player.pos_x] = sight->player.standing_at;    
  else
    game->map[sight->player.pos_y][sight->player.pos_x] = FREE_SPACE;
    
  game->players[sight->number_of_player]->pos_y += y;
  game->players[sight->number_of_player]->pos_x += x;
  sight->player.pos_y = game->players[sight->number_of_player]->pos_y;
  sight->player.pos_x = game->players[sight->number_of_player]->pos_x;
  //sight->player.standing_at = FREE_SPACE;
  sight->player.standing_at = game->map[sight->player.pos_y][sight->player.pos_x];
}

void death_of_player(struct map_info* game, int number_of_player){

  game->sights_of_players[number_of_player]->player.deaths += 1;
  game->sights_of_players[number_of_player]->player.points = 0;
  game->int_map[game->players[number_of_player]->pos_y][game->players[number_of_player]->pos_x] = game->sights_of_players[number_of_player]->player.carried;
  game->map[game->players[number_of_player]->pos_y][game->players[number_of_player]->pos_x] = COIN;
  game->sights_of_players[number_of_player]->player.carried = 0;

  struct coords c;
  spawn_anything(game, &c);
  
  game->sights_of_players[number_of_player]->player.pos_y = c.y;
  game->sights_of_players[number_of_player]->player.pos_x = c.x;

  game->sights_of_players[number_of_player]->player.standing_at = game->map[game->sights_of_players[number_of_player]->player.pos_y][game->sights_of_players[number_of_player]->player.pos_x];
}

void* check_deaths(void* v){

  struct deaths_t *info = ((struct deaths_t*)v);

  while(1)
  {
    pthread_mutex_lock(&info->mutex);

    usleep(100000);

    for(int i=0; i < MAX_PLAYERS; i++)
    {
      for(int j=0; j < MAX_PLAYERS; j++)
      {
        if( info->game->n_o_p->slots[i] == TAKEN && info->game->n_o_p->slots[j] == TAKEN && j!=i )
        {
          if(info->game->players[i]->pos_x == info->game->players[j]->pos_x && info->game->players[i]->pos_y == info->game->players[j]->pos_y)
          {
            death_of_player(info->game, i);
            death_of_player(info->game, j);
          }
        }
      }
      for(int j=0; j<info->game->number_of_beasts; j++)
      {
        if(info->game->players[i]->pos_x == info->game->beasts[j].x && info->game->players[i]->pos_y == info->game->beasts[j].y)
        death_of_player(info->game, i);
      }
    }

  }
  return NULL;
}

void* check_movement(struct map_info* game, struct sight_of_player* sight, int y, int x){

  switch( *(*(game->map+sight->player.pos_y+y)+sight->player.pos_x+x) ){
    case WALL:

    break;
    case FREE_SPACE:
      moving_itself(game, sight, y, x);
    break;
    case COIN:
      moving_itself(game, sight, y, x);
      game->players[sight->number_of_player]->carried += *(*(game->int_map+sight->player.pos_y)+sight->player.pos_x);
      *(*(game->int_map+sight->player.pos_y)+sight->player.pos_x) = 0;
      create_coin(game);
    break;
    case CAMP:
      game->players[sight->number_of_player]->points += game->players[sight->number_of_player]->carried;
      game->players[sight->number_of_player]->carried = 0;
    break;
    case BUSH:
      moving_itself(game, sight, y, x);
    break;
    default:

    break;
  }

  return NULL;
}

void* move_player_server(struct map_info* game, struct sight_of_player* sight){

  if(sight->movement == UP)
  {
    check_movement(game, sight, -1, 0);
  }
  else if(sight->movement == DOWN)
  {
    check_movement(game, sight, 1, 0);
  }
  else if(sight->movement == LEFT)
  {
    check_movement(game, sight, 0, -1);
  }
  else if(sight->movement == RIGHT)
  {
    check_movement(game, sight, 0, 1);
  }
  if(sight->movement != QUIT)
    sight->movement = NONE;
  update_sight_of_player(game, sight);

  return NULL;
}

int destroy_game(struct map_info *game){

  for(int i=0; i<MAP_SIZE_Y; i++)
    {
      free(*(game->map+i));
    } 
  free(game->map);
  return 0;
}

void print_sight_of_player(struct sight_of_player *sight_of_player){

  pthread_mutex_lock(&mutex);
  for(int i=0; i<MAP_SIZE_Y; i++)
    for(int j=0; j<MAP_SIZE_X; j++)
      {
        print_map_elements(*(*(sight_of_player->map+i)+j), i, j, *(*(sight_of_player->int_map+i)+j));
      }
  refresh();
  pthread_mutex_unlock(&mutex);

  pthread_mutex_lock(&mutex);
  for(int i=0; i<5; i++)
  {
    for(int j=0; j<5; j++)
    {
      if(sight_of_player->player.pos_y+i-2>0 && sight_of_player->player.pos_x+j-2>0)
        {
          print_map_elements(*(*(sight_of_player->sight_map+i)+j), sight_of_player->player.pos_y+i-2, sight_of_player->player.pos_x+j-2, *(*(sight_of_player->int_map+i)+j));
          if( sight_of_player->sight_map[i][j] == BEAST )
          {
            attron(COLOR_PAIR(5));
            mvprintw(sight_of_player->player.pos_y+i-2, sight_of_player->player.pos_x+j-2, " ");
            attroff(COLOR_PAIR(5)); 
          } 
          else if( sight_of_player->sight_map[i][j] >= PLAYER0 && sight_of_player->sight_map[i][j] <= PLAYER3)
          {
            attron(COLOR_PAIR(3));
            mvprintw(sight_of_player->player.pos_y+i-2, sight_of_player->player.pos_x+j-2, "%d", sight_of_player->sight_map[i][j] - PLAYER0);
            attroff(COLOR_PAIR(3));
          }
        }
    }
  }
  refresh();
  pthread_mutex_unlock(&mutex);
}

void* thread_unlock(void *v){

  struct threads_to_unlock *thread = ((struct threads_to_unlock*)v);
  while(1)
  {
    for(int i=0; i < MAX_PLAYERS; i++)
      pthread_mutex_unlock(&thread->threadstounlock[i]->mutex);
    pthread_mutex_unlock(&thread->deadthstounlock->mutex);
    for(int i=0; i < thread->game->number_of_beasts; i++)
      pthread_mutex_unlock(&thread->beaststounlock[i]->mutex);
    usleep(1000000);
  }

  return NULL;
}

void* thread_move(void *v){

  struct thread_move_t *thread = ((struct thread_move_t*)v);

  while(1)
  {
    sem_wait(thread->semaphore_in);
    create_player(thread->game, thread->sight);  
    sem_post(thread->semaphore_movement2);

    while( thread->sight->movement != QUIT )
    {
      pthread_mutex_lock(&thread->mutex);

      sem_wait(thread->semaphore_movement1);
      move_player_server(thread->game, thread->sight);
      print_map(thread->game);
      sem_post(thread->semaphore_movement2);
    }
  }

  return NULL;
}

void* spawn_anything_server(void* v){
  struct threads_to_unlock *t = ((struct threads_to_unlock*)v);

  while(1){
    char ch = getch();
    if(ch == 'c' || ch == 't' || ch == 'T')
      create_coin(t->game);
    else if(ch == 'b' || ch == 'B')
      create_beast(t->game, t);
    else if(ch == 'q' || ch == 'Q')
      break;
  }
  return NULL;
}

void moving_itself_beast(struct beast_info* beast_info, int y, int x){
    beast_info->game->map[beast_info->game->beasts[beast_info->number_of_beast].y][beast_info->game->beasts[beast_info->number_of_beast].x] = beast_info->game->beasts[beast_info->number_of_beast].standing_at;
    beast_info->game->beasts[beast_info->number_of_beast].y += y;
    beast_info->game->beasts[beast_info->number_of_beast].x += x;
    beast_info->game->beasts[beast_info->number_of_beast].standing_at = beast_info->game->map[beast_info->game->beasts[beast_info->number_of_beast].y][beast_info->game->beasts[beast_info->number_of_beast].x];
}

void function_for_beast_rawr(struct beast_info *info){

  for(int i=0; i<5; i++)
      for(int j=0; j<5; j++)
      {
        int y = info->game->beasts[info->number_of_beast].y;
        int x = info->game->beasts[info->number_of_beast].x;

          if (! (y+i-2 < 0 || y+i-2 >= MAP_SIZE_Y || x+j-2 < 0 || x+j-2 >= MAP_SIZE_X ))
          {
            for(int k = 0; k < MAX_PLAYERS; k++)
            {
              if( info->game->n_o_p->slots[k] == TAKEN )
              {
                int py = info->game->players[k]->pos_y; 
                int px = info->game->players[k]->pos_x;

                if( y+i-2 == py && x+j-2 == px )
                {
                  int odleglosc_x = x - (x+j-2);
                  int odleglosc_y = y - (y+i-2);

                  if((odleglosc_x < 0) && info->game->map[y][x+1] != WALL ){
                          moving_itself_beast(info, 0, 1);
                          return;}
                  else if(odleglosc_y < 0 && info->game->map[y+1][x] != WALL ){
                          moving_itself_beast(info, 1, 0);
                          return;}
                  else if(odleglosc_x > 0 && info->game->map[y][x-1] != WALL ){
                          moving_itself_beast(info, 0, -1);
                          return;}
                  else if(odleglosc_y > 0 && info->game->map[y-1][x] != WALL ){
                          moving_itself_beast(info, -1, 0);
                          return;}   
                }
              }
              else{

                while(1)
                {
                  int random = rand() % 4;

                  if( random == 0 && info->game->map[y-1][x] != WALL ){
                    moving_itself_beast(info, -1, 0);
                    return;}
                  else if ( random == 1 && info->game->map[y+1][x] != WALL ){
                    moving_itself_beast(info, 1, 0);
                    return;}
                  else if ( random == 2 && info->game->map[y][x+1] != WALL ){
                    moving_itself_beast(info, 0, 1);
                    return;}
                  else if ( random == 3 && info->game->map[y][x-1] != WALL ){
                    moving_itself_beast(info, 0, -1);
                    return;}
                }

              }
            }
          }
        }
}

void* beast_rawr(void* v){

  struct beast_info *info = ((struct beast_info*)v);

  while(1)
  {
    pthread_mutex_lock(&info->mutex);
    function_for_beast_rawr(info);
  }
  return NULL;
}