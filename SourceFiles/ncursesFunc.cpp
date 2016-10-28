#include <string>
#include <curses.h>
#include "ncursesFunc.h"
#include "generalFunc.h"

int TimedPrint(WINDOW* w, const std::string& s, unsigned newX, unsigned newY, unsigned period){

    /*

    The following presents two ways of waiting for the input each with their own pros and cons:

        1) The sleepMilli and wtimeout(w, 0) is for low periods (fast printing). It waits for 30 milliseconds,
        allowing the user to add to the input buffer before using wgetch (which is processed instantly since wtimeout(w, 0).
        This is not ideal for longer periods since it makes key inputs feel laggy (since a maximum time of period needs to
        be waited before the input is processed)
        2) wtimeout(w, period) is for long periods (slow printing). wtimeout cannot be used with low periods, this
        seems to be a limit of the function's capability. wtimeout(w, 1) for instance seems to wait as long as wtimeout(w, 100).
        So this function would've been perfect for all periods if not for this limiting factor.

    */

    unsigned x = 0, y = 0;
    getyx(w, y, x); //obtain current cursor position
    wmove(w, newY, newX);

    if(period < 30) wtimeout(w, 0);
    else wtimeout(w, period);

    for(auto it = s.begin(); it != s.end(); ++it){
        if(*it == ' '){ //no need to print spaces over time
            waddch(w, *it);
            wrefresh(w);
            continue;
        }
        waddch(w, *it);
        wrefresh(w);

        if(period < 30) sleepMilli(period);

        int result = wgetch(w);
        if(result != ERR){
            while(++it != s.end()){
                waddch(w, *it);
            }
            wtimeout(w, -1); //back to normal waiting
            wrefresh(w);
            wmove(w, y, x);
            return result;
        }

    }

    wtimeout(w, -1); //back to normal waiting
    wmove(w, y, x);
    return ERR;

}

void replaceLine(WINDOW* w, const std::string& s, unsigned newX, unsigned newY){

    unsigned x = 0, y = 0;
    getyx(w, y, x); //obtain current cursor position
    wmove(w, newY, newX);
    wclrtoeol(w);
    wprintw(w, "%s", s.c_str());
    wmove(w, y, x);

}

void clearLine(WINDOW* w, unsigned newX, unsigned newY){

    replaceLine(w, "", newX, newY);

}

std::string grabAndClearLine(WINDOW* w, unsigned newX, unsigned newY){

    unsigned x = 0, y = 0;
    getyx(w,y,x); //save the old position
    wmove(w, newY, newX); //move to the line we want to grab

    chtype p[500] = {}; //we assume 500 will be enough...
    winchnstr(w, p, 500); //obtain the line
    clrtoeol(); //remove the line

    std::string line;
    for(unsigned i = 0; p[i] != 0; ++i){
        line.push_back(p[i]);
    }

    wmove(w, y, x); //move back
    return line; //RVO occurs here

}

void moveLineUp(WINDOW* w, unsigned newX, unsigned newY, unsigned No){ //positions of the line we want to move and how many lines up

    if(newY == 0){ //if we are at the top, just erase the line and move back
        clearLine(w, newX, newY);
        return;
    }

    std::string line = grabAndClearLine(w, newX, newY);
    replaceLine(w, line, newX, newY-No);

}




