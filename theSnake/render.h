#pragma once   

#include "main.h"
#include "snake.h"
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

enum {
    NORMAL = 0,
    LOSE = 1,
    PAUSE = 2,
    SAVE = 3,
    LOAD = 4
};

class Render{
    public:
    void ezDraw(){
        for(int i = 0; i < ROW; i++){
            for(int j = 0; j < COL; j++){
                if(get_map_class(i, j).get_value() == floor_V::EMPTY){
                    cout << ". ";
                }
                else if(get_map_class(i, j).get_value() == floor_V::SNAKE_BODY){
                    cout << "S ";
                }
                else if(get_map_class(i, j).get_value() == floor_V::SNAKE_HEAD){
                    cout << "H ";
                }
                else if(get_map_class(i, j).get_value() == floor_V::FOOD){
                    cout << "X ";
                }
            }
            cout << endl;
        }
    }
    void sfmlDraw(sf::RenderWindow &window, int op = NORMAL){
        window.clear(sf::Color::Black);
        for(int i = 0; i < ROW; i++){
            for(int j = 0; j < COL; j++){
                float cellW = static_cast<float>(WindowRow) / COL;
                float cellH = static_cast<float>(WindowCol) / ROW;
                sf::RectangleShape cell(sf::Vector2f(cellW, cellH));
                cell.setPosition({j * cellW + 0.5f, i * cellH + 0.5f});
                if(get_map_class(i, j).get_value() == floor_V::EMPTY){
                    cell.setFillColor(sf::Color::White);
                }
                window.draw(cell);
            }
        }
        for(int i = 0; i < ROW; i++){
            for(int j = 0; j < COL; j++){
                drawHead(window, i, j);
                drawBody(window, i, j);
                drawFood(window, i, j);
            }
        }
        switch(op){
            case NORMAL:
                break;
            case LOSE:
                sfmlGameOver(window);
                break;
            case PAUSE:
                sfmlPause(window);
                break;
            case SAVE:
                sfmlPause(window);
                sfmlSave(window);
                break;
            case LOAD:  
                sfmlPause(window);
                sfmlLoad(window);
                break;
        }
        window.display();
    }
    void drawHead(sf::RenderWindow &window, int i, int j){
        if(get_map_class(i, j).get_value() != floor_V::SNAKE_HEAD) return;
        float cellW = static_cast<float>(WindowRow) / COL;
        float cellH = static_cast<float>(WindowCol) / ROW;
        float cx = j * cellW + cellW / 2;
        float cy = i * cellH + cellH / 2;
        float r  = min(cellW, cellH) / 2;
        sf::CircleShape circle(r);
        circle.setOrigin({r, r});
        circle.setPosition({cx, cy});
        circle.setFillColor(sf::Color::Green);
        window.draw(circle);
    }
    void drawBody(sf::RenderWindow &window, int i, int j){
        if(get_map_class(i, j).get_value() != floor_V::SNAKE_BODY) return;
        float cellW = static_cast<float>(WindowRow) / COL;
        float cellH = static_cast<float>(WindowCol) / ROW;
        float cx = j * cellW + cellW / 2;
        float cy = i * cellH + cellH / 2;
        float side = min(cellW, cellH) * 0.8f;
        sf::RectangleShape rect(sf::Vector2f(side, side));
        rect.setOrigin({side / 2, side / 2});
        rect.setPosition({cx, cy});
        rect.setFillColor(sf::Color::Blue);
        window.draw(rect);
    }
    void drawFood(sf::RenderWindow &window, int i, int j){
        if(get_map_class(i, j).get_value() != floor_V::FOOD) return;
        float cellW = static_cast<float>(WindowRow) / COL;
        float cellH = static_cast<float>(WindowCol) / ROW;
        float cx = j * cellW + cellW / 2;
        float cy = i * cellH + cellH / 2;
        float r  = min(cellW, cellH) / 3;
        sf::CircleShape circle(r);
        circle.setOrigin({r, r});
        circle.setPosition({cx, cy});
        circle.setFillColor(sf::Color::Red);
        window.draw(circle);
    }
    void sfmlGameOver(sf::RenderWindow &window){
        //cout<< "Game Over!11111" << endl;

        sf::Font font;
        if (!font.openFromFile("font/arial.ttf")) {
            // Handle error
            cout<< "error" << endl;
            return;
        }
        sf::Text text(font);

        text.setString("Game Over!");

        text.setCharacterSize(24); // in pixels, not points!

        text.setFillColor(sf::Color::Red);
        text.setPosition({WindowRow / 2, WindowCol / 2});
        window.draw(text);

    }
    void sfmlPause(sf::RenderWindow &window){
        sf::Font font;
        if (!font.openFromFile("font/arial.ttf")) {
            // Handle error
            cout<< "error" << endl;
            return;
        }
        sf::Text text(font);

        text.setString("Pause!\nPress Space to Continue, Press R to Restart\nPress T to Save, Press L to load");

        text.setCharacterSize(24); // in pixels, not points!

        text.setFillColor(sf::Color::Black);
        text.setPosition({WindowRow / 4, WindowCol / 3});
        window.draw(text);
    }
    void sfmlSave(sf::RenderWindow &window){
        sf::Font font;
        if (!font.openFromFile("font/arial.ttf")) {
            // Handle error
            cout<< "error" << endl;
            return;
        }
        sf::Text text(font);

        text.setString("Game Saved!");

        text.setCharacterSize(24); // in pixels, not points!

        text.setFillColor(sf::Color::Black);
        text.setPosition({WindowRow / 4, WindowCol / 4});
        window.draw(text);
    }
    void sfmlLoad(sf::RenderWindow &window){
        sf::Font font;
        if (!font.openFromFile("font/arial.ttf")) {
            // Handle error
            cout<< "error" << endl;
            return;
        }
        sf::Text text(font);

        text.setString("Game Loaded!");

        text.setCharacterSize(24); // in pixels, not points!

        text.setFillColor(sf::Color::Black);
        text.setPosition({WindowRow / 4, WindowCol / 4});
        window.draw(text);
    }
};