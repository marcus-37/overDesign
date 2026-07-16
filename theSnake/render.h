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
                sf::RectangleShape rectangle(sf::Vector2f(WindowRow / COL, WindowCol / ROW));
                rectangle.setPosition({j * (WindowRow / COL), i * (WindowCol / ROW)});
                Map temp = get_map_class(i, j);
                if(temp.get_value() == floor_V::EMPTY){
                    rectangle.setFillColor(sf::Color::White);
                }
                else if(temp.get_value() == floor_V::SNAKE_BODY){
                    rectangle.setFillColor(sf::Color::Blue);
                }
                else if(temp.get_value() == floor_V::SNAKE_HEAD){
                    rectangle.setFillColor(sf::Color::Green);
                }
                else if(get_map_class(i, j).get_value() == floor_V::FOOD){
                    rectangle.setFillColor(sf::Color::Red);
                }
                window.draw(rectangle);
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