#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <SDL3/SDL.h>

typedef struct Game Game;

Game* game_new(void);
bool game_start(Game* game);
void game_update(Game* game);
bool game_is_alive(Game* game);
void game_close(Game* game);

#endif // GAME_H