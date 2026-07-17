#include "states.h"
#include "food.h"

void save_state(Snake *player, string filename){
    ofstream file(filename);    //only once save
    file << player->snake_body.size() << "\n";
    for(Sbody* body : player->snake_body){
        file << body->get_pos().first << " " << body->get_pos().second << " " << body->get_value() << "\n";
    }
    for(int i = 0; i < ROW; i++){
        for(int j = 0; j < COL; j++){
            Map cell = get_map_class(i, j);
            switch(cell.get_value()){
                case floor_V::EMPTY:
                    file << "EMPTY " + cell.save_string() << " ";
                    break;
                case floor_V::SNAKE_BODY:
                    file << "SNAKE_BODY " + cell.save_string() << " ";
                    break;
                case floor_V::FOOD:
                    file << "FOOD " + cell.save_string() << " ";
                    break;
                case floor_V::SNAKE_HEAD:
                    file << "SNAKE_HEAD " + cell.save_string() << " ";
                    break;
            }
        }
        file << "\n";
    }
    file<<player->get_direction()<<" "<<player->get_move_direction();
    file.close();
}

void load_state(Snake *player, string filename){
    ifstream file(filename);

    int snake_size;
    file >> snake_size;
    player->snake_body.clear();
    for(int i = 1; i <= snake_size; i++){
        int x, y, value;
        file >> x >> y >> value;
        player->snake_body.push_back(new Sbody(x, y, value == floor_V::SNAKE_HEAD));
    }

    for(int i = 0; i < ROW; i++){
        for(int j = 0; j < COL; j++){
            string cell_str;
            file >> cell_str;
            string cell_data = "";
            while(1){
                string temp;
                file >> temp;
                cell_data += temp;
                if(cell_data.back() == '}'){
                    break;
                }
                else cell_data += " ";
                //cout<< "temp: " << temp << endl;
            }
            Map cell;

            switch(load_s(cell_str)){
                case floor_V::EMPTY:
                    {
                        cell = Map({i, j}).load_string(cell_data);
                    }
                    break;
                case floor_V::SNAKE_BODY:
                    {
                        cell = Sbody({i, j}, 0).load_string(cell_data);
                    }
                    break;
                case floor_V::FOOD:
                    {
                        cell = Food(i, j).load_string(cell_data);
                    }
                    break;
                case floor_V::SNAKE_HEAD:
                    {
                        cell = Sbody({i, j}, 1).load_string(cell_data);
                    }
                    break;
            }
            //cout<< "cell: " << cell.save_string() << endl;
            change_map(i, j, cell);
        }
    }
    int d1, d2;
    file>>d1>>d2;
    player->set_direction(static_cast<Direction>(d1));
    player->set_move_direction(static_cast<Direction>(d2));
    file.close();
}

floor_V load_s(string cell_str){
    istringstream iss(cell_str);
    string type;
    iss >> type;
    if(type == "EMPTY"){
        return floor_V::EMPTY;
    } else if(type == "SNAKE_BODY"){
        return floor_V::SNAKE_BODY;
    } else if(type == "FOOD"){
        return floor_V::FOOD;
    } else if(type == "SNAKE_HEAD"){
        return floor_V::SNAKE_HEAD;
    }
    return floor_V::EMPTY;
}

