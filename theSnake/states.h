#pragma once   

#include "snake.h"
#include "component.h"
#include "food.h"

void save_state(Snake *player, string filename);
void load_state(Snake *player, string filename);
floor_V load_s(string cell_str);

enum GameState{
    RUNNING,
    PAUSING,
    LOSING
};

class State{
    public:

        virtual void enter(){}
        virtual void leave(){}

        virtual GameState update(needUpdate* need_update, sf::Time dt)=0;
        virtual GameState get_state()=0;
        virtual ~State() = default;
};

class RunningState : public State{
    public:
        void enter(){
            //nothing
        }
        void leave(){
            //nothing
        }
        GameState update(needUpdate* need_update, sf::Time dt){
            for(Snake* snake : need_update->movable->snakes){
                if(snake->time2move(dt) == 0) continue;
                int todo = snake_move(snake, need_update->food);
                if(todo == -1){
                    leave();
                    return GameState::LOSING;
                }
                else if(todo == 1){
                    gen_food(need_update->food);
                    cout<<1;
                }
            }
            if(need_update->food->foods.empty()){
                gen_food(need_update->food);
                //cout<<1;
            }
            for(Food* food : need_update->food->foods){
                //nothing
            }
            //cout<<2;
            return GameState::RUNNING;
        }
        GameState get_state(){
            return GameState::RUNNING;
        }
};
class PauseState : public State{
    public:
        void enter(){
            //pause
        }
        void leave(){
            //resume
        }
        GameState update(needUpdate* need_update, sf::Time dt){
            return GameState::PAUSING;
        }
        GameState get_state(){
            return GameState::PAUSING;
        }
};
class LoseState : public State{
    public:
        void enter(){
            //fail
        }
        void leave(){
            //restart
        }
        GameState update(needUpdate* need_update, sf::Time dt){
            return GameState::LOSING;
        }
        GameState get_state(){
            return GameState::LOSING;
        }
};

