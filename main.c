#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <SDL3/SDL.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef float    f32;
typedef double   f64;

typedef size_t   usize;
typedef ptrdiff_t isize;

typedef struct {
  SDL_Window* window;
  bool keep_alive;
  bool alive;
} Game;

int game_init(Game* game) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    printf("Failed to create SDL3 window! %s\n", SDL_GetError());
    return -1;
  }

  SDL_Window* window = SDL_CreateWindow(
    "Cocoa", 
    800, 
    600, 
    SDL_WINDOW_RESIZABLE
  );
  if (window == NULL) {
    printf("Failed to create SDL3 window! %s\n", SDL_GetError());
    return -1;
  }

  game->window = window;
  return 0;
}

void game_start(Game* game) {
  game->alive = true;
  game->keep_alive = true;
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

  game->keep_alive = false;
  game->alive = false;
  SDL_Quit();
}

int main() {
  Game game;
  game_init(&game);
  game_start(&game);
  while (game_is_alive(&game)) {
    game_update(&game);
  }
  game_close(&game);
  return 0;
}