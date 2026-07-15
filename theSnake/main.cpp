#include "main.h"
#include "snake.h"
#include "render.h"

Floor map[ROW][COL];
Direction use_Direction = Direction::DOWN;
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> row_dist(0, ROW - 1);
uniform_int_distribution<int> col_dist(0, COL - 1);

pair<int, int> gen_food() {
    int r = row_dist(gen);
    int c = col_dist(gen);
    while(map[r][c].value != floor_V::EMPTY){
        if(c < COL) c++;
        else{
            c = 0;
            if(r < ROW) r++;
            else r = 0;
        }
    }
    return {r, c};
}

int main() {
    sf::RenderWindow gameWindow;
    gameWindow.create(sf::VideoMode({WindowRow, WindowCol}), "The Snake Game",
        sf::Style::Resize | sf::Style::Close);
    
    Render Rendering;
    Snake player;
    reset_map();
    //Rendering.ezDraw();
    Rendering.sfmlDraw(gameWindow);

    const sf::Time timePerFrame = sf::seconds(1.0f / 10.0f); // 每秒10次逻辑更新
    const sf::Time Three = sf::seconds(3.0f); // 3秒
    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;
    Direction read_direction = Direction::DOWN;
    Direction lastDirection = Direction::DOWN;
    deque<Direction> directionQueue;
    directionQueue.push_back(read_direction);
    int lose = 0;
    int stop = 0;
    int sl = 0;

    // run the program as long as the window is open
    while (gameWindow.isOpen())
    {
        
        // check all the window's events that were triggered since the last iteration of the loop
        while (const optional event = gameWindow.pollEvent())
        {
            // "close requested" event: we close the window
            if (event->is<sf::Event::Closed>())
                gameWindow.close();

            if(const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()){
                
                if(keyPressed->scancode == sf::Keyboard::Scan::W) read_direction = Direction::UP;
                else if(keyPressed->scancode == sf::Keyboard::Scan::S) read_direction = Direction::DOWN;
                else if(keyPressed->scancode == sf::Keyboard::Scan::A) read_direction = Direction::LEFT;
                else if(keyPressed->scancode == sf::Keyboard::Scan::D) read_direction = Direction::RIGHT;
                else{
                    if(keyPressed->scancode == sf::Keyboard::Scan::Escape){
                        gameWindow.close();
                    }
                    else if(keyPressed->scancode == sf::Keyboard::Scan::R && lose == 1){
                        reset_map();
                        lose = 0;
                        read_direction = Direction::DOWN;
                        lastDirection = Direction::DOWN;
                        player.init_snake(0, 0);
                        timeSinceLastUpdate = sf::Time::Zero;
                        stop = 0;
                        sl = 0;
                    }
                    else if(keyPressed->scancode == sf::Keyboard::Scan::Space){
                        stop = !stop;
                        timeSinceLastUpdate = sf::Time::Zero;
                        sl = 0;
                    }
                    else if(stop){
                        if(keyPressed->scancode == sf::Keyboard::Scan::T){
                            save_snake(&player, "save.txt", use_Direction);
                            sl = 1;
                            timeSinceLastUpdate = sf::Time::Zero;
                        }
                        else if(keyPressed->scancode == sf::Keyboard::Scan::L){
                            load_snake(&player, "save.txt");
                            sl = 2;
                            timeSinceLastUpdate = sf::Time::Zero;
                        }
                    }
                    continue;
                }

                if(stop){
                    read_direction = lastDirection;
                    continue;
                }

                if((lastDirection == Direction::UP && read_direction == Direction::DOWN) ||
                    (lastDirection == Direction::DOWN && read_direction == Direction::UP) ||
                    (lastDirection == Direction::LEFT && read_direction == Direction::RIGHT) ||
                    (lastDirection == Direction::RIGHT && read_direction == Direction::LEFT)){
                    read_direction = lastDirection;
                }
                else{
                    lastDirection = read_direction;
                }
                if(directionQueue.size() <= 5) directionQueue.push_back(read_direction);
            }
        }
        
        timeSinceLastUpdate += clock.restart();
        if(lose == 1) continue;
        if(stop == 1){
            if(!sl) Rendering.sfmlDraw(gameWindow, PAUSE);
            else{
                if(sl == 1)
                    Rendering.sfmlDraw(gameWindow, SAVE);
                else if(sl == 2)
                    Rendering.sfmlDraw(gameWindow, LOAD);
                if(timeSinceLastUpdate >= Three) sl = 0;
            }
            continue;
        }

        

        // 只要积累的时间达到一步，就更新游戏逻辑（可以多步追赶）
        while (timeSinceLastUpdate >= timePerFrame) {
            timeSinceLastUpdate -= timePerFrame;
            
            if(!directionQueue.empty()){
                use_Direction = directionQueue.front();
                directionQueue.pop_front();
            }
            //cout<< "direction: " << use_Direction << endl;

            int todo = snake_move(&player, use_Direction);
            if(todo == -1){
                cout<< "Game Over!" << endl;
                //gameWindow.close();
                lose = 1;
            }
            else if(todo == 1){
                change_map(gen_food(), floor_V::FOOD);
            }
        }

        //Rendering.ezDraw();
        Rendering.sfmlDraw(gameWindow, lose);

    }
}

void change_map(int x, int y, floor_V value, Direction direction){
    map[x][y].value = value;
    map[x][y].direction = direction;
}

void change_map(pair<int, int> pos, floor_V value, Direction direction){
    map[pos.first][pos.second].value = value;
    map[pos.first][pos.second].direction = direction;
}

int get_map_v(int x, int y){
    return map[x][y].value;
}

int get_map_v(pair<int, int> pos){
    return map[pos.first][pos.second].value;
}

Direction get_map_d(pair<int, int> pos){
    return map[pos.first][pos.second].direction;
}

void reset_map(){
    for(int i = 0; i < ROW; i++){
        for(int j = 0; j < COL; j++){
            map[i][j].x = i;
            map[i][j].y = j;
            map[i][j].value = floor_V::EMPTY;
            map[i][j].direction = Direction::NONE;
        }
    }
    map[0][0].value = floor_V::SNAKE;
    change_map(gen_food(), floor_V::FOOD);
}