#include "food.h"

random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> row_dist(0, ROW - 1);
uniform_int_distribution<int> col_dist(0, COL - 1);

pair<int, int> gen_pos() {
    bool hasEmpty = false;
    for(int i = 0; i < ROW; i++){
        for(int j = 0; j < COL; j++){
            if(get_map_class(i, j).get_value() == floor_V::EMPTY){
                hasEmpty = true;
                break;
            }
        }
        if(hasEmpty) break;
    }
    if(!hasEmpty) return {-1, -1};

    int r = row_dist(gen);
    int c = col_dist(gen);
    while(get_map_class({r, c}).get_value() != floor_V::EMPTY){
        if(c < COL - 1) c++;
        else{
            c = 0;
            if(r < ROW - 1) r++;
            else r = 0;
        }
    }
    return {r, c};
}

void gen_food(Foods* foods){
    pair<int, int> pos = gen_pos();
    if(pos.first == -1) return;
    change_map(pos, Food(pos));
    foods->add_food(new Food(pos));
    //cout<< "(" << pos.first << ", " << pos.second << ")" << endl;
}