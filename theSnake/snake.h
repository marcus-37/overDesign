#pragma once   

#include <vector>
#include <deque>
#include <utility>
#include "main.h"
using namespace std;
class Snake{
    public:
    pair<int, int> snake_head = make_pair(0, 0);
    pair<int, int> snake_tail = make_pair(0, 0);
    void init_snake(int x, int y){
        snake_head.first = x;
        snake_head.second = y;
        snake_tail.first = x;
        snake_tail.second = y;
    }
};

int snake_move(Snake *player, Direction direction);