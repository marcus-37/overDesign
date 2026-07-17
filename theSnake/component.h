#pragma once  

#include "snake.h"

class needUpdate{
    public:
        class movable* movable;
        class Foods* food;
        needUpdate(class movable* movable, class Foods* food){
            this->movable = movable;
            this->food = food;
        }
};

class movable{
    public:
        vector<Snake*> snakes;
        void add_snake(Snake* snake){
            snakes.push_back(snake);
        }
};