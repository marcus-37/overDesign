#include "main.h"
#include "food.h"
#include "snake.h"
#include "render.h"
#include "states.h"

Floor map[ROW][COL];

int main() {
    sf::RenderWindow gameWindow;
    gameWindow.create(sf::VideoMode({WindowRow, WindowCol}), "The Snake Game",
        sf::Style::Resize | sf::Style::Close);
    
    Render Rendering;
    Snake player;
    player.init_snake(0, 0);
    reset_map();
    //Rendering.ezDraw();
    Rendering.sfmlDraw(gameWindow);

    const sf::Time timePerFrame = sf::seconds(1.0f / 10.0f); // 每秒10次逻辑更新
    const sf::Time Three = sf::seconds(3.0f); // 3秒
    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;
    Direction read_direction = Direction::DOWN;
    Direction lastDirection = Direction::DOWN;
    Direction use_Direction = Direction::DOWN;
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
                    else if(keyPressed->scancode == sf::Keyboard::Scan::R){
                        reset_map();
                        lose = 0;
                        read_direction = Direction::DOWN;
                        lastDirection = Direction::DOWN;
                        use_Direction = Direction::DOWN;
                        directionQueue.clear();
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
                            save_state(&player, "save.txt");
                            sl = 1;
                            timeSinceLastUpdate = sf::Time::Zero;
                        }
                        else if(keyPressed->scancode == sf::Keyboard::Scan::L){
                            load_state(&player, "save.txt");
                            use_Direction = player.get_direction();
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
            player.set_direction(use_Direction);
            int todo = snake_move(&player);
            if(todo == -1){
                cout<< "Game Over!" << endl;
                //gameWindow.close();
                lose = 1;
            }
            else if(todo == 1){
                gen_food();
            }
        }

        //Rendering.ezDraw();
        Rendering.sfmlDraw(gameWindow, lose);

    }
}

void change_map(int x, int y, Map value){
    map[x][y].value = value;
}

void change_map(pair<int, int> pos, Map value){
    map[pos.first][pos.second].value = value;
}

Map get_map_class(int x, int y){
    return map[x][y].value;
}

Map get_map_class(pair<int, int> pos){
    return map[pos.first][pos.second].value;
}

void reset_map(){
    for(int i = 0; i < ROW; i++){
        for(int j = 0; j < COL; j++){
            map[i][j].value.set_value(floor_V::EMPTY);
            map[i][j].value.set_pos(i, j);
        }
    }
    change_map(0, 0, Sbody(0, 0, 1));
    gen_food();
}