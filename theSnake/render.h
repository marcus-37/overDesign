#pragma once   

#include "main.h"

enum {
    NORMAL = 0,
    LOSE = 1,
    PAUSE = 2
};

class Render{
    public:
    void ezDraw(){
        for(int i = 0; i < ROW; i++){
            for(int j = 0; j < COL; j++){
                if(map[i][j].value == floor_V::EMPTY){
                    cout << ". ";
                }
                else if(map[i][j].value == floor_V::SNAKE){
                    if(map[i][j].direction == Direction::NONE) cout << "H ";
                    else cout << "S ";
                }
                else if(map[i][j].value == floor_V::FOOD){
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
                if(map[i][j].value == floor_V::EMPTY){
                    rectangle.setFillColor(sf::Color::White);
                }
                else if(map[i][j].value == floor_V::SNAKE){
                    if(map[i][j].direction == Direction::NONE) rectangle.setFillColor(sf::Color::Green);
                    else rectangle.setFillColor(sf::Color::Blue);
                }
                else if(map[i][j].value == floor_V::FOOD){
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

        text.setString("Pause!/nPress Space to Continue, Press R to Restart, Press B to Back");

        text.setCharacterSize(24); // in pixels, not points!

        text.setFillColor(sf::Color::Yellow);
        text.setPosition({WindowRow / 2, WindowCol / 2});
        window.draw(text);
    }
};