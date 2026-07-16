#pragma once   

#include "snake.h"

void save_state(Snake *player, string filename);
void load_state(Snake *player, string filename);
floor_V load_s(string cell_str);