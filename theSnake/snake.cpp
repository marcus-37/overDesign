#include "snake.h"

void pair_update(pair<int, int> *snake, Direction direction){
    int x = snake->first;
    int y = snake->second;
    if(direction == UP){ // up
        x--;
    } else if(direction == DOWN){ // down
        x++;
    } else if(direction == LEFT){ // left
        y--;
    } else if(direction == RIGHT){ // right
        y++;
    }
    if(x < 0) x = ROW - 1;
    else if(x >= ROW) x = 0;
    if(y < 0) y = COL - 1;
    else if(y >= COL) y = 0;
    snake->first = x;
    snake->second = y;
}

int snake_move(Snake *player, Direction direction){
    change_map(player->snake_head, floor_V::SNAKE, direction);
    //cout<<get_map_d(player->snake_tail)<<endl;

    pair_update(&player->snake_head, direction);
    int type = get_map_v(player->snake_head);
    if(type == floor_V::FOOD){
        return 1; //need to generate food
    }
    else if(type == floor_V::SNAKE){
        return -1; //game over
    }
    else if(type == floor_V::EMPTY){
        pair<int, int> temp = player->snake_tail;
        pair_update(&player->snake_tail, get_map_d(player->snake_tail));
        change_map(temp, floor_V::EMPTY);
        //cout<< "tail: " << player->snake_tail.first << " " << player->snake_tail.second << endl;
        
    }
    else return -1; //error

    change_map(player->snake_head, floor_V::SNAKE);
    return 0; //success
}

void save_snake(Snake *player, string filename, Direction d){
    ofstream file(filename);    //only once save
    file << player->snake_head.first << " " << player->snake_head.second << "\n";
    file << player->snake_tail.first << " " << player->snake_tail.second << "\n";
    for(int i = 0; i < ROW; i++){
        for(int j = 0; j < COL; j++){
            file << map[i][j].value << " ";
            file << map[i][j].direction << " ";
        }
        file << "\n";
    }
    file<<d;
    file.close();
}

void load_snake(Snake *player, string filename){
    ifstream file(filename);
    file >> player->snake_head.first >> player->snake_head.second;
    file >> player->snake_tail.first >> player->snake_tail.second;
    for(int i = 0; i < ROW; i++){
        for(int j = 0; j < COL; j++){
            int value, direction;
            file >> value >> direction;
            map[i][j].value = static_cast<floor_V>(value);
            map[i][j].direction = static_cast<Direction>(direction);
        }
    }
    int d;
    file>>d;
    use_Direction = static_cast<Direction>(d);
    file.close();
}