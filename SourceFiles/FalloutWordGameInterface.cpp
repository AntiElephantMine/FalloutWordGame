#include <curses.h>
#include <vector>
#include <string>
#include <cstring> //for strlen
#include <algorithm>
#include <iostream>
#include <fstream>
#include <Windows.h> //sleep
#include "functions.h"


int main(){

    //OBTAIN THE WORDS TO USE IN THE GAME, AND POST-GAME PREPARATION

    unsigned wordSize = randomUns(4,11);

    const std::string fileName = std::string("WordFiles\\word") + std::to_string(wordSize) + std::string(".txt");
    std::ifstream input(fileName);
    std::ifstream cat("WordFiles\\asciiCat.txt");

    if(!input){
        std::cout << "Could not open file: " << fileName << ". Exiting program." << std::endl;
        Sleep(5000);
        return -1;
    }
    if(!cat){
        std::cout << "Could not open file: " << "asciiCat.txt" << ". Exiting program." << std::endl;
        Sleep(5000);
        return -1;
    }

    std::vector<std::string> allWords, selectWords;

    for(std::string s; input >> s; ){
        if(s.size() != wordSize){ //shouldn't happen but just in case
            continue;
        }
        allWords.push_back(capitalised(s));
    }

    input.close();

    for(unsigned i = 1; i < 13; ++i){ //13 words on screen
        std::vector<std::string>::iterator it = allWords.begin();
        it += randomUns(0,allWords.size()-1);
        selectWords.push_back(*it);
        allWords.erase(it);
    }

    std::vector<std::string> catBasket; //for later

    for(std::string s; getline(cat, s);)
        catBasket.push_back(s);

    cat.close();



    //START PRE-SCREEN CALCULATING

    const unsigned gridX = 12, gridY = 16; //the size of the grids
    const unsigned gridNo = 2; //number of grids on screen
    const unsigned gridPadding = 8; //distance between the grids
    const unsigned relX = gridPadding, relY = 6; //the start point on screen for the leftmost grid
    const unsigned screenX = (gridNo*(gridX+gridPadding)+25), screenY = gridY+9; //the screen size
    const std::pair<unsigned, unsigned> borderTop = {relX, relY}, borderBot = {relX + gridNo*gridX + (gridNo-1)*gridPadding-1, relY + gridY - 1}; //these two positions indicate the box we cannot move our cursor outside

    const std::string filler = "@#!:;^_+=<>$%*[]{}?-/\\|.`'"; //the filler characters
    std::string left = paddedString(selectWords.begin(), selectWords.begin() + (selectWords.end()-selectWords.begin())/2, filler, gridX*gridY);
    std::string right = paddedString(selectWords.begin() + (selectWords.end()-selectWords.begin())/2, selectWords.end(), filler, gridX*gridY);

    //START THE SCREEN

    initscr();
    resize_term(screenY, screenX); //resize screen
    keypad(stdscr, true); //enable keyboard commands on standard window
    noecho(); //don't print keyboard input
    curs_set(0);

    //PRINT OUT THE GRIDS AND THE GRID PADDING

    moveClearPrintMove(stdscr, "Welcome to ROBCO Industries (TM) Termlink.", 1, 1);
    moveClearPrintMove(stdscr, "Password Required", 1, 2);

    //GRID PADDING

    for(unsigned i = 0; i < gridNo; ++i){
        for(unsigned j = relY; j < relY+gridY; ++j){
            std::string address = int_to_hex(33856+(j-relY)*12 + gridY*12*i);
            if(address.size() != gridPadding-2) throw std::runtime_error("Word within grid padding is of incorrect size.");
            movePrintMove(stdscr, address, 1+(gridPadding+gridX)*i, j);
        }
    }


    printGrid(stdscr, left, relX, relY, gridX, gridY);
    printGrid(stdscr, right, relX+gridPadding+gridX, relY, gridX, gridY);

    //MAIN LOOP

    std::string current = boldWord(stdscr, filler, gridX, gridY);
    std::string answer = selectWords[randomUns(0,selectWords.size()-1)];

    unsigned tries = 0;
    unsigned maxTries = 4;

    while(true){
        moveClearPrintMove(stdscr, "Attempts remaining: " + std::to_string(4-tries), 1, 4);
        moveClearPrintMove(stdscr, ">" + current, relX + gridNo*gridX + (gridNo-1)*gridPadding + 1, relY + gridY -1);
        switch(wgetch(stdscr)){
            case KEY_UP:
                unboldWord(stdscr, filler, gridX, gridY);
                moveUp(stdscr, borderTop);
                current = boldWord(stdscr, filler, gridX, gridY);
                break;
            case KEY_DOWN:
                unboldWord(stdscr, filler, gridX, gridY);
                moveDown(stdscr, borderBot);
                current = boldWord(stdscr, filler, gridX, gridY);
                break;
            case KEY_LEFT:
                unboldWord(stdscr, filler, gridX, gridY);
                moveLeft(stdscr, borderTop, gridPadding);
                current = boldWord(stdscr, filler, gridX, gridY);
                break;
            case KEY_RIGHT:
                unboldWord(stdscr, filler, gridX, gridY);
                moveRight(stdscr, borderBot, gridPadding);
                current = boldWord(stdscr, filler, gridX, gridY);
                break;
            case 0xA: //this is what is returned when we press enter. It is the ASCII value for a newline character (since KEY_ENTER did not seem to provide the right value).
                if(current == answer){
                    ++tries;
                    unsigned lineMoveUp = 2; //how many lines we want to move up
                    for(unsigned i = relY+lineMoveUp; i <= relY + gridY-2; ++i){
                        moveLineUp(stdscr, relX + gridNo*gridX + (gridNo-1)*gridPadding + 1, i, lineMoveUp);
                    }
                    moveClearPrintMove(stdscr, "Attempts remaining: " + std::to_string(maxTries-tries), 1, 4);
                    moveClearPrintMove(stdscr, ">" + current, relX + gridNo*gridX + (gridNo-1)*gridPadding +1, relY + gridY-3);
                    moveClearPrintMove(stdscr, ">Password Accepted.", relX + gridNo*gridX + (gridNo-1)*gridPadding + 1, relY + gridY-2);
                    moveClearPrintMove(stdscr, ">", relX + gridNo*gridX + (gridNo-1)*gridPadding + 1, relY + gridY -1);
                    refresh();
                    Sleep(5000);

                    //PRINT THE CAT

                    for(auto it = catBasket.begin(); it != catBasket.end(); ++it){
                        for(unsigned i = 0; i < screenY; ++i){
                            moveLineUp(stdscr, 0, i, 1);
                        }
                        moveClearPrintMove(stdscr, (*it).c_str(), 1, screenY-1);
                        Sleep(100); //lower to increase speed
                        wrefresh(stdscr);
                    }

                    Sleep(10000);

                    //EXIT PROGRAM

                    endwin();
                    return 0;

                }
                else{
                    if(std::find(filler.begin(), filler.end(), current[0]) != filler.end()) {//then we are on a filler character.
                        unsigned lineMoveUp = 2; //how many lines we want to move up
                        for(unsigned i = relY+lineMoveUp; i <= relY + gridY-2; ++i){
                            moveLineUp(stdscr, relX + gridNo*gridX + (gridNo-1)*gridPadding + 1, i, lineMoveUp);
                        }
                        moveClearPrintMove(stdscr, ">" + current, relX + gridNo*gridX + (gridNo-1)*gridPadding + 1, relY + gridY-3);
                        moveClearPrintMove(stdscr, ">Error", relX + gridNo*gridX + (gridNo-1)*gridPadding + 1, relY + gridY-2);

                    }
                    else{ //we are on a word
                        ++tries;
                        if(tries == maxTries){
                            unsigned lineMoveUp = 3; //how many lines we want to move up
                            for(unsigned i = relY+lineMoveUp; i <= relY + gridY-2; ++i){
                                moveLineUp(stdscr, relX + gridNo*gridX + (gridNo-1)*gridPadding + 1, i, lineMoveUp);
                            }
                            moveClearPrintMove(stdscr, "Attempts remaining: " + std::to_string(maxTries-tries), 1, 4);
                            moveClearPrintMove(stdscr, ">" + current, relX + gridNo*gridX + (gridNo-1)*gridPadding + 1, relY + gridY-4);
                            moveClearPrintMove(stdscr, ">Entry Denied.", relX + gridNo*gridX + (gridNo-1)*gridPadding + 1, relY + gridY-3);
                            moveClearPrintMove(stdscr, ">Initialising Lockout.", relX + gridNo*gridX + (gridNo-1)*gridPadding + 1, relY + gridY-2);
                            moveClearPrintMove(stdscr, ">", relX + gridNo*gridX + (gridNo-1)*gridPadding + 1, relY + gridY -1);
                            refresh();
                            Sleep(5000);
                            endwin();
                            return 0;
                        }
                        else{
                            unsigned lineMoveUp = 3; //how many lines we want to move up
                            for(unsigned i = relY+lineMoveUp; i <= relY + gridY-2; ++i){
                                moveLineUp(stdscr, relX + gridNo*gridX + (gridNo-1)*gridPadding + 1, i, lineMoveUp);
                            }
                            moveClearPrintMove(stdscr, ">" + current, relX + gridNo*gridX + (gridNo-1)*gridPadding + 1, relY + gridY-4);
                            moveClearPrintMove(stdscr, ">Entry Denied.", relX + gridNo*gridX + (gridNo-1)*gridPadding + 1, relY + gridY-3);
                            moveClearPrintMove(stdscr, ">Likeness=" + std::to_string(countMatch(current, answer)), relX + gridNo*gridX + (gridNo-1)*gridPadding + 1, relY + gridY-2);
                        }
                    }
                }
                break;
            default:
                break;
        }

        wrefresh(stdscr);
    }

    endwin();

    /*
    1) Think if there's a way to make word at bottom left appear as if being typed.
    */

}
