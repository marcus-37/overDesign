#include "main.h"
#include "food.h"
#include "snake.h"
#include "render.h"
#include "states.h"
#include "component.h"

Floor map[ROW][COL];
Direction read_direction = Direction::DOWN;
Direction use_Direction = Direction::DOWN;
Direction decode_direction(const sf::Event::KeyPressed *keyPressed, Direction lastDirection);
Direction cantBack(Direction read_direction, Direction lastDirection);
int autoPlay = 0;
float nowSpeedModifier = 1;

int main() {
    sf::RenderWindow gameWindow;
    gameWindow.create(sf::VideoMode({WindowRow, WindowCol}), "The Snake Game",
        sf::Style::Resize | sf::Style::Close);
    
    Render Rendering;
    Snake player;
    reset_map();
    //Rendering.ezDraw();
    Rendering.sfmlDraw(gameWindow);

    const sf::Time timePerFrame = sf::seconds(1.0f / 20.0f); // 每秒20次逻辑更新
    const sf::Time Three = sf::seconds(3.0f); // 3秒
    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;
    State* currentState = new RunningState();
    movable Movable;
    Movable.add_snake(&player);
    Foods nowfoods;
    needUpdate need_update(&Movable, &nowfoods);
    deque<Direction> directionQueue;
    directionQueue.push_back(read_direction);
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
                if(keyPressed->scancode == sf::Keyboard::Scan::Escape){
                    gameWindow.close();
                }
                else if(keyPressed->scancode == sf::Keyboard::Scan::R){
                    reset_map();
                    nowfoods.foods.clear();
                    gen_food(need_update.food);
                    directionQueue.clear();
                    player.init_snake(0, 0);
                    timeSinceLastUpdate = sf::Time::Zero;
                    sl = 0;
                    currentState->leave();
                    free(currentState);
                    currentState = new RunningState();
                }
                else if(keyPressed->scancode == sf::Keyboard::Scan::Q){
                    autoPlay = !autoPlay;
                    nowSpeedModifier = autoPlay ? 4 : 1;
                    player.set_SpeedModifier(nowSpeedModifier);
                }
                else if(keyPressed->scancode == sf::Keyboard::Scan::P){
                    nowSpeedModifier = 0.25;
                    player.set_SpeedModifier(nowSpeedModifier);
                }

                if(currentState == nullptr) currentState = new RunningState();
                else if(currentState->get_state() == GameState::RUNNING){
                    if(keyPressed->scancode == sf::Keyboard::Scan::Space){
                        timeSinceLastUpdate = sf::Time::Zero;
                        sl = 0;
                        currentState->leave();
                        free(currentState);
                        currentState = new PauseState();
                    }
                    else{
                        read_direction = decode_direction(keyPressed, player.get_direction());
                        if(directionQueue.size() <= 5) directionQueue.push_back(read_direction);
                    }
                }
                else if(currentState->get_state() == GameState::PAUSING){
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
                    else if(keyPressed->scancode == sf::Keyboard::Scan::Space){
                        timeSinceLastUpdate = sf::Time::Zero;
                        sl = 0;
                        currentState->leave();
                        free(currentState);
                        currentState = new RunningState();
                    }
                }
            }
        }
        
        //timeSinceLastUpdate += clock.restart();

        //timeSinceLastUpdate -= timePerFrame;
        
        if(!directionQueue.empty() && player.get_direction() == player.get_move_direction()){
            use_Direction = directionQueue.front();
            directionQueue.pop_front();
        }
        //cout<< "direction: " << use_Direction << endl;
        
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)){
            player.set_SpeedModifier(1.5*nowSpeedModifier);
            if(autoPlay) player.set_SpeedModifier(2*nowSpeedModifier);
        }
        else{
            player.set_SpeedModifier(nowSpeedModifier);
        }

        if(autoPlay){
            if(player.get_head()->get_pos().second < COL - 1 && player.get_head()->get_pos().second > 0){
                use_Direction = Direction::RIGHT;
            }
            else if(player.get_direction() == Direction::DOWN && player.get_head()->get_pos().second == COL - 1) 
                use_Direction = Direction::LEFT;
            else if(player.get_direction() == Direction::DOWN || player.get_direction() == Direction::UP)
                use_Direction = Direction::RIGHT;
            else use_Direction = Direction::DOWN;
        }

        use_Direction = cantBack(use_Direction, player.get_direction());
        player.set_move_direction(use_Direction);
        int nextState = currentState->update(&need_update, clock.restart());
        if(nextState != currentState->get_state()){
            if(nextState == GameState::LOSING){
                currentState->leave();
                free(currentState);
                currentState = new LoseState();
            }
        }

        //Rendering.ezDraw();
        if(currentState->get_state() == GameState::PAUSING){
            if(!sl) Rendering.sfmlDraw(gameWindow, PAUSE);
            else{
                if(sl == 1)
                    Rendering.sfmlDraw(gameWindow, SAVE);
                else if(sl == 2)
                    Rendering.sfmlDraw(gameWindow, LOAD);
                if(timeSinceLastUpdate >= Three) sl = 0;
            }
        }
        else if(currentState->get_state() == GameState::LOSING){
            Rendering.sfmlDraw(gameWindow, LOSE);
        }
        else{
            Rendering.sfmlDraw(gameWindow);
        }

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
    read_direction = Direction::DOWN;
    use_Direction = Direction::DOWN;
    change_map(0, 0, Map({0, 0}, floor_V::SNAKE_HEAD));
}

Direction decode_direction(const sf::Event::KeyPressed *keyPressed, Direction lastDirection){
    Direction read_direction = lastDirection;
    if(keyPressed->scancode == sf::Keyboard::Scan::W) read_direction = Direction::UP;
    else if(keyPressed->scancode == sf::Keyboard::Scan::S) read_direction = Direction::DOWN;
    else if(keyPressed->scancode == sf::Keyboard::Scan::A) read_direction = Direction::LEFT;
    else if(keyPressed->scancode == sf::Keyboard::Scan::D) read_direction = Direction::RIGHT;

    read_direction = cantBack(read_direction, lastDirection);
    return read_direction;
}

Direction cantBack(Direction read_direction, Direction lastDirection){
    if((lastDirection == Direction::UP && read_direction == Direction::DOWN) ||
        (lastDirection == Direction::DOWN && read_direction == Direction::UP) ||
        (lastDirection == Direction::LEFT && read_direction == Direction::RIGHT) ||
        (lastDirection == Direction::RIGHT && read_direction == Direction::LEFT)){
        read_direction = lastDirection;
    }
    return read_direction;
}