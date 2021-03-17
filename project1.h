#ifndef PROJECT1_H_INCLUDED
#define PROJECT1_H_INCLUDED

#define MAP_SIZE_Y 25
#define MAP_SIZE_X 52
#define MAX_PLAYERS 4

enum one_place {
  WALL,
  FREE_SPACE,
  NEW_LINE,
  COIN,
  CAMP,
  BUSH,
  BEAST,
  PLAYER0,
  PLAYER1,
  PLAYER2,
  PLAYER3
};

enum movement {
  UP,
  DOWN,
  RIGHT,
  LEFT,
  NONE,
  QUIT
};

enum player_type {
  HUMAN,
  BOT,
  NO
};

struct player{
  int pos_x;
  int pos_y;
  int points;
  int deaths;
  int carried;
  enum one_place standing_at;
  enum player_type player_type;
};

struct beast{
  int x;
  int y;
  enum one_place standing_at;
};

struct beast_info{
  struct map_info *game;
  int number_of_beast;
  pthread_mutex_t mutex;
};

struct sight_of_player{
  enum one_place map[MAP_SIZE_Y][MAP_SIZE_X];
  enum one_place sight_map[5][5]; 
  int int_map[5][5]; 
  struct player player;
  int number_of_player;
  enum movement movement;
};

enum is_slot_empty{
  EMPTY,
  TAKEN
};

struct number_of_processes{
  enum is_slot_empty slots[MAX_PLAYERS];
  int number_of_proccesses;
  int free_number;
};

struct map_info{
  enum one_place **map; 
  int **int_map;
  int number_of_players;
  struct player *players[MAX_PLAYERS];
  struct beast beasts[MAX_PLAYERS];
  int number_of_beasts;
  struct sight_of_player *sights_of_players[MAX_PLAYERS];
  struct number_of_processes *n_o_p;
};

struct thread_move_t{
  sem_t *semaphore_in;
  sem_t *semaphore_movement1;
  sem_t *semaphore_movement2;
  struct map_info *game;
  struct sight_of_player *sight;
  pthread_mutex_t mutex;
};

struct deaths_t{
  struct map_info *game;
  pthread_mutex_t mutex;
};

struct threads_to_unlock{
  struct thread_move_t *threadstounlock[MAX_PLAYERS];
  struct deaths_t *deadthstounlock;
  struct beast_info *beaststounlock[MAX_PLAYERS];
  struct map_info *game;
};

struct coords{
  int x;
  int y;
};

struct map_info *generate_game(struct map_info *game, struct number_of_processes *n_o_p);
void generate_map(struct map_info *game);

void print_map_elements (enum one_place place, int i, int j, int value);
void print_players_beasts(struct map_info *game);
void print_map(struct map_info *game);

int is_free(struct map_info *game, struct coords *c);
void spawn_anything(struct map_info *game, struct coords *c);

void create_camp(struct map_info *game);
void create_coin(struct map_info *game);
void create_beast(struct map_info *game, struct threads_to_unlock *t);
void create_player(struct map_info *game, struct sight_of_player* sight);

void* move_player_bot_thread(void* v);
void* move_player_bot(void* v);

void moving_itself(struct map_info* game, struct sight_of_player* sight, int y, int x);

void update_sight_of_player(struct map_info *game, struct sight_of_player* sight);
void moving_itself(struct map_info* game, struct sight_of_player* sight, int y, int x);
void death_of_player(struct map_info* game, int number_of_player);
void* check_deaths(void* v);
void* check_movement(struct map_info* game, struct sight_of_player* sight, int y, int x);
void* move_player_client_thread(void* v);
void* move_player_client(void* v);
void* move_player_server(struct map_info* game, struct sight_of_player* sight);
int destroy_game(struct map_info *game);
void print_sight_of_player(struct sight_of_player *sight_of_player);

void moving_itself_beast(struct beast_info* beast_info, int y, int x);
void function_for_beast_rawr(struct beast_info *info);
void* beast_rawr(void* v);

void* thread_unlock(void* v);
void* thread_move(void *v);

void* spawn_anything_server(void* v);

#endif // PROJECT1_H_INCLUDED