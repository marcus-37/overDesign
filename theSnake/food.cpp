#include "food.h"

random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> row_dist(0, ROW - 1);
uniform_int_distribution<int> col_dist(0, COL - 1);

pair<int, int> gen_pos() {
    int r = row_dist(gen);
    int c = col_dist(gen);
    while(get_map_class({r, c}).get_value() != floor_V::EMPTY){
        if(c < COL) c++;
        else{
            c = 0;
            if(r < ROW) r++;
            else r = 0;
        }
    }
    return {r, c};
}

void gen_food() {
    change_map(gen_pos(), Food(gen_pos()));
}