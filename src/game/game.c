#include "game.h"
#include <stdlib.h>

typedef struct Game {
  SDL_Window* window;
  bool keep_alive;
  bool alive;
} Game;

Game* game_new(void) {
    return malloc(sizeof(Game));
}

bool game_start(Game* game) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    printf("Failed to create SDL3 window! %s\n", SDL_GetError());
    return false;
  }

  SDL_Window* window = SDL_CreateWindow(
    "Cocoa", 
    800, 
    600, 
    SDL_WINDOW_RESIZABLE
  );
  if (window == NULL) {
    printf("Failed to create SDL3 window! %s\n", SDL_GetError());
    return false;
  }

  game->window = window;
  game->alive = true;
  game->keep_alive = true;
  return true;
}

void game_update(Game* game) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_QUIT) {
      game->keep_alive = false;
      break;
    }
  }
}

bool game_is_alive(Game* game) {
  return game->keep_alive && game->alive;
}

void game_close(Game* game) {
  if (!game->alive) {
    return;
  }

  if (game->window != NULL) {
    SDL_DestroyWindow(game->window);
  }

  free(game);
  SDL_Quit();
}