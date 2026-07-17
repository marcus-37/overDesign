#pragma once   

#include <vector>
#include <list>
#include <utility>
#include "main.h"
#include "food.h"

class Sbody : public Map{
    public:
    int is_head;
    Sbody(int x, int y, int head = 0){
        this->set_pos(x, y);
        if(!head) this->set_value(floor_V::SNAKE_BODY);
        else this->set_value(floor_V::SNAKE_HEAD);
        this->is_head = head;
    }
    Sbody(pair<int, int> pos, int head = 0){
        this->set_pos(pos.first, pos.second);
        if(!head) this->set_value(floor_V::SNAKE_BODY);
        else this->set_value(floor_V::SNAKE_HEAD);
        this->is_head = head;
    }
    string save_string(){
        return "{ " + to_string(this->get_value()) + " " + to_string(this->get_pos().first) + " " + to_string(this->get_pos().second) + " }";
    }
    Sbody load_string(string s){
        istringstream iss(s);
        char c;
        int value, x, y;
        iss >> c >> value >> x >> y >> c;
        this->set_value(static_cast<floor_V>(value));
        this->set_pos(x, y);
        return *this;
    }

    void onLeave(){
        change_map(this->get_pos(), Map(this->get_pos(), floor_V::EMPTY));
    }
    private:

};

class Snake{
    public:
    list<Sbody*> snake_body;//front is head, back is tail
    void init_snake(int x, int y){
        snake_body.clear();
        snake_body.push_front(new Sbody(x, y, 1));
        direction = Direction::DOWN;
        this->Timer = sf::Time::Zero;
        real_direction = Direction::DOWN;
    }
    Sbody* get_head(){
        return snake_body.front();
    }
    void set_head(pair<int, int> pos){
        snake_body.front()->is_head = 0;
        snake_body.front()->set_value(floor_V::SNAKE_BODY);
        snake_body.push_front(new Sbody(pos, 1));
    }
    void set_direction(Direction dir){
        this->real_direction = dir;
    }
    void set_move_direction(Direction dir){
        this->direction = dir;
    }
    Direction get_direction(){
        return this->real_direction;
    }
    Direction get_move_direction(){
        return this->direction;
    }
    int time2move(sf::Time dt)
    {
        this->Timer += dt;
        if (Timer >= speed/SpeedModifier)
        {
            Timer -= speed/SpeedModifier;
            return 1;
        }
        return 0;
    }
    Snake(int x = 0, int y = 0){
        this->init_snake(x, y);
    }
    void set_SpeedModifier(float modifier)
    {
        SpeedModifier = modifier;
    }
    float get_SpeedModifier() const
    {
        return SpeedModifier;
    }

    private:
    Direction direction;
    sf::Time speed = sf::seconds(SNAKE_SPEED);
    sf::Time Timer;
    float SpeedModifier = 1;
    Direction real_direction;
};

int snake_move(Snake *player, Foods* foods);