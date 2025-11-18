#include "game/game.h"

int main() {
  Game* game = game_new();
  if (!game_start(game)) {
    fprintf(stderr, "Failed to start game!\n");
    return -1;
  }

  while (game_is_alive(game)) {
    game_update(game);
  }
  game_close(game);
  return 0;
}