#pragma once   

#include <iostream>
#include <fstream>
#include <sstream>
#include <deque>

using namespace std;
#define ROW 10
#define COL 10
#define WindowRow 600
#define WindowCol 600
enum floor_V{
    EMPTY = 0,
    SNAKE_BODY = 1,
    FOOD = 2,
    SNAKE_HEAD = 3
};
enum Direction{
    NONE = -1,
    UP = 0,
    DOWN = 1,
    LEFT = 2,
    RIGHT = 3
};
class Map{
    public:
    floor_V get_value(){
        return this->value;
    }
    pair<int, int> get_pos(){
        return make_pair(this->x, this->y);
    }
    void set_value(floor_V value){
        this->value = value;
    }
    void set_pos(int x = 0, int y = 0){
        this->x = x;
        this->y = y;
    }
    Map(pair<int, int> pos = {0, 0}, floor_V value = floor_V::EMPTY){
        this->value = value;
        this->x = pos.first;
        this->y = pos.second;
    }

    string save_string(){
        return "{ " + to_string(this->value) + " " + to_string(this->x) + " " + to_string(this->y) + " }";
    }
    Map load_string(string s){
        istringstream iss(s);
        char c;
        int value, x, y;
        iss >> c >> value >> x >> y >> c;
        this->value = static_cast<floor_V>(value);
        this->x = x;
        this->y = y;
        return *this;
    }
    private:
    floor_V value;
    int x;
    int y;
};//基类
class Floor{
    public:
    Map value;
};

void change_map(int x, int y, Map value);

void change_map(pair<int, int> pos, Map value);

Map get_map_class(int x, int y);

Map get_map_class(pair<int, int> pos);

void reset_map();

