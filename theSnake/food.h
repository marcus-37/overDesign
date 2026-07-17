#pragma once   

#include <random>
#include "main.h"
#include <set>

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
    void onEnter(Map who){
        this->set_value(floor_V::EMPTY);
    }
    private:
};
class Foods{
    public:
    set<Food*> foods;
    void add_food(Food* food){
        foods.insert(food);
    }
    void remove_food(pair<int, int> pos){
        for(auto it = foods.begin(); it != foods.end(); ++it){
            if((*it)->get_pos() == pos){
                delete *it;
                foods.erase(it);
                break;
            }
        }
    }
};
pair<int, int> gen_pos();
void gen_food(Foods* foods);