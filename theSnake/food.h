#pragma once   

#include <random>
#include "main.h"

class Food : public Map{
    public:
    Food(int x, int y){
        this->set_pos(x, y);
        this->set_value(floor_V::FOOD);
    }
    Food(pair<int, int> pos){
        this->set_pos(pos.first, pos.second);
        this->set_value(floor_V::FOOD);
    }
    private:
};
pair<int, int> gen_pos();
void gen_food();