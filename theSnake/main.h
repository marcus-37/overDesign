#pragma once   

#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <random>
using namespace std;
#define ROW 10
#define COL 10
#define WindowRow 600
#define WindowCol 600
enum floor_V{
    EMPTY = 0,
    SNAKE = 1,
    FOOD = 2
};
enum Direction{
    NONE = -1,
    UP = 0,
    DOWN = 1,
    LEFT = 2,
    RIGHT = 3
};
class Floor{
    public:
    int x;
    int y;
    floor_V value;
    Direction direction;//if it is snake, pointing to the forward body;
};
extern Floor map[ROW][COL];

void change_map(int x, int y, floor_V value, Direction direction = Direction::NONE);

void change_map(pair<int, int> pos, floor_V value, Direction direction = Direction::NONE);

int get_map_v(int x, int y);

int get_map_v(pair<int, int> pos);

Direction get_map_d(pair<int, int> pos);

void reset_map();

