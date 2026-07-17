#include "snake.h"

void update_snake(Snake *player){
    Direction direction = player->get_move_direction();
    player->set_direction(direction);

    int x = player->get_head()->get_pos().first;
    int y = player->get_head()->get_pos().second;
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
    player->set_head({x, y});
}

void update_map(pair<int, int> pos, floor_V value){
    change_map(pos, Map(pos, value));
}

int snake_move(Snake *player, Foods* foods){
    //cout<<"test"<<endl
    Sbody* head = player->get_head();

    update_map(head->get_pos(), floor_V::SNAKE_BODY);
    update_snake(player);
    head = player->get_head();//update_snake只更新了snake类

    int type = get_map_class(head->get_pos()).get_value();//map类未更新
    if(type == floor_V::FOOD){
        foods->remove_food(head->get_pos());
        get_map_class(head->get_pos()).onEnter(*head);
        change_map(head->get_pos(), *head);
        //cout<<2;
        return 1; //need to generate food
    }
    else if(type == floor_V::SNAKE_BODY){
        get_map_class(head->get_pos()).onEnter(*head);
        change_map(head->get_pos(), *head);
        return -1; //game over
    }
    else if(type == floor_V::EMPTY){
        pair<int, int> tail_pos = player->snake_body.back()->get_pos();
        player->snake_body.pop_back();
        update_map(tail_pos, floor_V::EMPTY);
        change_map(head->get_pos(), *head);
    }
    else return -1; //error
    
    return 0; //success
}
