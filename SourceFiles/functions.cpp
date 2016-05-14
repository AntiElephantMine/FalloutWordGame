#include <curses.h>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <string>
#include <random>
#include <utility>
#include <ctime>
#include <map>
#include <utility>
#include <sstream>
#include "functions.h"


unsigned randomUns(unsigned minV, unsigned maxV){

    static std::default_random_engine generator(time(0));
    static bool first = true;
    static std::map<std::pair<unsigned, unsigned>, std::uniform_int_distribution<unsigned>> distributions;

    if(first){
        generator.discard(10); /*the first output of generator does not really change with time, so we discard the first few values.
        The 10th value almost certainly changes with time*/
        first = false;
    }

    std::pair<unsigned, unsigned> range(minV, maxV);

    if(distributions.find(range) == distributions.end())
       distributions.insert({range, std::uniform_int_distribution<unsigned>(range.first, range.second)});

    return distributions[range](generator);
}

unsigned randomNormalUns(unsigned average, unsigned standard){

    static std::default_random_engine generator(time(0));
    static bool first = true;
    static std::map<std::pair<unsigned, unsigned>, std::normal_distribution<double>> distributions;

    if(first){
        generator.discard(10); /*the first output of generator does not really change with time, so we discard the first few values.
        The 10th value almost certainly changes with time*/
        first = false;
    }

    std::pair<unsigned, unsigned> range(average, standard);

    if(distributions.find(range) == distributions.end())
       distributions.insert({range, std::normal_distribution<double>(range.first, range.second)});

    return distributions[range](generator);
}


std::string paddedString(std::vector<std::string>::const_iterator b, std::vector<std::string>::const_iterator e, const std::string& filler, unsigned s){ //the vector, the filler and the amount to pad

    unsigned currentLength = 0; //how much of the current gridSpace has been used up
    std::for_each(b, e, [&currentLength](const std::string& s){currentLength += s.size();}); //count up how much space is taken up by the non-filler words

    /*Each non-filler word will be preceded by a random amount of random filler characters. However we must ensure that we don't use up s characters. Any remaining
    space will be taken up by filler characters too. */

    unsigned upperLim = (s - currentLength)/(e-b+1); //the number of filler sequences is e-b+1
    if(upperLim < 1) throw std::runtime_error("Grid too small to provide at least one filler character between words."); //because randomUns(1,upperLim) might not do what we require if upperLim == 0.

    std::string words;

    for(auto it = b; it != e; ++it){
        unsigned fillerAmount = randomNormalUns(upperLim, 1);
        for(unsigned i = 0; i < fillerAmount; ++i){
            words.push_back(filler[randomUns(0, filler.size()-1)]); //generate the filler characters preceding the word
        }
        currentLength += fillerAmount;
        words += *it;
    }

    if(currentLength > s) throw std::logic_error("The program creator gon' dun' fucked up with his darn logic."); //I can remove this once I'm confident I've done everything right

    for(unsigned i = 0; i < (s - currentLength); ++i){ //generate the remaining filler characters if necessary
        words.push_back(filler[randomUns(0, filler.size()-1)]);
    }

    return words;

}

void printGrid(WINDOW* w, const std::string& words, unsigned relX, unsigned relY, unsigned W, unsigned H){

    wmove(w, relY, relX);

    for(unsigned y = 0; y < H; ++y){
        for(unsigned x = 0; x < W; ++x){
            waddch(w, words[y*W + x]);
        }
        wmove(w, relY+(y+1), relX);
    }
    wmove(w, relY+H-1, relX+W-1);

}

std::string boldWord(WINDOW* w, const std::string& filler, unsigned W, unsigned H){ //bolds the word the cursor is on and returns the entire bolded word

    unsigned x = 0, y = 0;
    getyx(w, y, x); //obtain current cursor position
    std::string newWord;

    wattron(w, A_BLINK);
    //Are we on a filler character already?
    if(std::find(filler.begin(), filler.end(), winch(w)) != filler.end()){ //bold the filler character and leave
        char c = winch(w);
        waddch(w, c);
        wmove(w, y, x);
        wattroff(w, A_BLINK);
        newWord.push_back(c);
        if(newWord == "%") newWord = "%%"; //issue with printw not printing the percentage sign placeholder
        return newWord;
    }
    else{ //We are on a word
        char c = winch(w);
        waddch(w, c);
        wmove(w, y, x);
        newWord.push_back(c);
    }

    unsigned newX = x-1, newY = y;

    while(true){ //moving leftwards
        char c = mvwinch(w, newY, newX);
        if(c == ' '){ //we left the grid, move up a row.
            newX += W;
            --newY;
            if(mvwinch(w, newY, newX) == ' ') break; //because the next row above is empty space
        } else if(std::find(filler.begin(), filler.end(), c) != filler.end()){ //we hit a filler character, time to end
            break;
        } else{//presumably we've hit a character
            waddch(w, c);
            wmove(w, newY, newX);
            newWord.insert(newWord.begin(), c);
            --newX;
        }
    }

    newX = x+1, newY = y;

    while(true){ //moving rightwards
        char c = mvwinch(w, newY, newX);
        if(c == ' '){ //we left the grid, move down a row.
            newX -= W;
            ++newY;
            if(mvwinch(w, newY, newX) == ' ') break; //because the next row above is empty space
        } else if(std::find(filler.begin(), filler.end(), c) != filler.end()){ //we hit a filler character, time to end
            break;
        } else{//presumably we've hit a character
            waddch(w, c);
            wmove(w, newY, newX);
            newWord.push_back(c);
            ++newX;
        }
    }

    wattroff(w, A_BLINK);
    wmove(w, y, x);
    return newWord;

    //now build up the word by moving to the left.

}

std::string unboldWord(WINDOW* w, const std::string& filler, unsigned W, unsigned H){ //unbolds the word the cursor is on and returns the entire unbolded word

    unsigned x = 0, y = 0;
    getyx(w, y, x); //obtain current cursor position
    std::string oldWord;

    wattroff(w, A_BLINK);
    //Are we on a filler character already?
    if(std::find(filler.begin(), filler.end(), winch(w)) != filler.end()){ //bold the filler character and leave
        char c = winch(w);
        waddch(w, c);
        wmove(w, y, x);
        oldWord.push_back(c);
        if(oldWord == "%") oldWord = "%%"; //issue with printw not printing the percentage sign placeholder
        return oldWord;
    }
    else{ //We are on a word
        char c = winch(w);
        waddch(w, c);
        wmove(w, y, x);
        oldWord.push_back(c);
    }

    unsigned newX = x-1, newY = y;

    while(true){ //moving leftwards
        char c = mvwinch(w, newY, newX);
        if(c == ' '){ //we left the grid, move up a row.
            newX += W;
            --newY;
            if(mvwinch(w, newY, newX) == ' ') break; //because the next row above is empty space
        } else if(std::find(filler.begin(), filler.end(), c) != filler.end()){ //we hit a filler character, time to end
            break;
        } else{//presumably we've hit a character
            waddch(w, c);
            wmove(w, newY, newX);
            oldWord.insert(oldWord.begin(), c);
            --newX;
        }
    }

    newX = x+1, newY = y;

    while(true){ //moving rightwards
        char c = mvwinch(w, newY, newX);
        if(c == ' '){ //we left the grid, move down a row.
            newX -= W;
            ++newY;
            if(mvwinch(w, newY, newX) == ' ') break; //because the next row above is empty space
        } else if(std::find(filler.begin(), filler.end(), c) != filler.end()){ //we hit a filler character, time to end
            break;
        } else{//presumably we've hit a character
            waddch(w, c);
            wmove(w, newY, newX);
            oldWord.push_back(c);
            ++newX;
        }
    }

    wmove(w, y, x);//return cursor to original position
    return oldWord;

    //now build up the word by moving to the left.

}


void moveLeft(WINDOW* w, std::pair<unsigned, unsigned> borderTop, unsigned gridDistance){ //grid distance is the number of characters between grids, normally 8. BorderTop is the top-left corner of the first grid.

    unsigned x = 0, y = 0;
    getyx(w, y, x); //obtain current cursor position
    unsigned newX = x, newY = y;
    const std::string alpha = "abcedfghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if(std::find(alpha.begin(), alpha.end(), winch(w)) != alpha.end()){ //our cursor is already on a word
        /*Two cases here:
        1) The word section is at the beginning of the line. In which case we skip over the rest of the word, and continue on until we hit a new grid or the end of the screen
        2) The word is not at the beginning of the line. In which case we move to the previous filler character. */
        while(std::find(alpha.begin(), alpha.end(), winch(w)) != alpha.end()){
            wmove(w, newY, --newX);
        } //ends when it doesn't encounter an alpha numeric character, this is either when it hits a filler character or goes off the grid
        if(winch(w) == ' '){ //checking whether we went off the grid, if we did then carry on
            if(newX < borderTop.first){ //then there is no previous grid and we need to return
                wmove(w, y, x);
                return;
            } else wmove(w, newY, newX-gridDistance);
        }
    }
    else{
        /*our cursor is on a filler character. Two cases here:
        1) The filler character is at the beginning of the line, in which case we skip to the next grid or until we hit the end of the screen.
        2) The filler character is not at the beginning of the line and we just move to the previous character. */
        wmove(w, newY, --newX);
        if(winch(w) == ' '){ //checking whether we went off the grid, if we did then carry on
            if(newX < borderTop.first){ //then there is no previous grid and we need to return
                wmove(w, y, x);
                return;
            } else wmove(w, newY, newX-gridDistance);
        }
    }

}

void moveRight(WINDOW* w, std::pair<unsigned, unsigned> borderBot, unsigned gridDistance){

    unsigned x = 0, y = 0;
    getyx(w, y, x); //obtain current cursor position
    unsigned newX = x, newY = y;
    const std::string alpha = "abcedfghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if(std::find(alpha.begin(), alpha.end(), winch(w)) != alpha.end()){ //our cursor is already on a word
        /*Two cases here:
        1) The word section is at the beginning of the line. In which case we skip over the rest of the word, and continue on until we hit a new grid or the end of the screen
        2) The word is not at the beginning of the line. In which case we move to the previous filler character. */
        while(std::find(alpha.begin(), alpha.end(), winch(w)) != alpha.end()){
            wmove(w, newY, ++newX);
        } //ends when it doesn't encounter an alpha numeric character, this is either when it hits a filler character or goes off the grid
        if(winch(w) == ' '){ //checking whether we went off the grid, if we did then carry on until we hit something that isn't a space or recieve ERR
            if(newX > borderBot.first){ //then there is no next grid and we need to return
                wmove(w, y, x);
                return;
            } else wmove(w, newY, newX+gridDistance);
        }
    }
    else{
        /*our cursor is on a filler character. Two cases here:
        1) The filler character is at the beginning of the line, in which case we skip to the next grid or until we hit the end of the screen.
        2) The filler character is not at the beginning of the line and we just move to the previous character. */
        wmove(w, newY, ++newX);
        if(winch(w) == ' '){ //checking whether we went off the grid, if we did then carry on until we hit something that isn't a space or recieve ERR
            if(newX > borderBot.first){ //then there is no next grid and we need to return
                wmove(w, y, x);
                return;
            } else wmove(w, newY, newX+gridDistance);
        }
    }

}

void moveUp(WINDOW* w, std::pair<unsigned, unsigned> borderTop){

    unsigned x = 0, y = 0;
    getyx(w, y, x); //obtain current cursor position

    if((y-1) < borderTop.second) return; //cause we would leeave the grid

    wmove(w, y-1, x);
    return;


}

void moveDown(WINDOW* w, std::pair<unsigned, unsigned> borderBot){

    unsigned x = 0, y = 0;
    getyx(w, y, x); //obtain current cursor position

    if((y+1) > borderBot.second) return; //cause we would leeave the grid

    wmove(w, y+1, x);
    return;

}

void moveClearPrintMove(WINDOW* w, const std::string& s, unsigned newX, unsigned newY){

    unsigned x = 0, y = 0;
    getyx(w, y, x); //obtain current cursor position
    wmove(w, newY, newX);
    wclrtoeol(w);
    wprintw(w, s.c_str());
    wmove(w, y, x);

}

void movePrintMove(WINDOW* w, const std::string& s, unsigned newX, unsigned newY){

    unsigned x = 0, y = 0;
    getyx(w, y, x); //obtain current cursor position
    wmove(w, newY, newX);
    wprintw(w, s.c_str());
    wmove(w, y, x);

}

void moveLineUp(WINDOW* w, unsigned newX, unsigned newY, unsigned No){ //positions of the line we want to move and how many lines up

    unsigned x = 0, y = 0;
    getyx(w,y,x); //save the old position
    wmove(w, newY, newX); //move to the line we want to move up

    if(newY == 0){ //if we are at the top, just erase the line and move back
        clrtoeol();
        wmove(w, y, x);
        return;
    }

    chtype p[200]; //we assume 200 will be enough...
    winchnstr(w, p, 200); //obtain the line
    clrtoeol(); //remove the line

    std::string line;
    for(unsigned i = 0; p[i] != 0; ++i){
        line.push_back(p[i]);
    }

    if(line[0] == '%') line = "%%";

    wmove(w, newY-No, newX); //move up a line
    clrtoeol(); //erase the line
    wprintw(w, line.c_str()); //print the new line

    wmove(w, y, x); //move back



}

unsigned countMatch(const std::string& s1, const std::string& s2){

    if(s1.size() != s2.size()) throw std::runtime_error("Compared strings do not have the same size.");

    unsigned countNo = 0;

    for(unsigned i = 0; i < s1.size(); ++i)
        if(s1[i] == s2[i]) ++countNo;

    return countNo;

}

std::string& capitalised(std::string& word){

    for(auto& c : word)
        c = toupper(c);
    return word;

}

std::string int_to_hex(unsigned i){
    std::stringstream stream;
    stream << "0x" << std::uppercase << std::hex << i;
    return stream.str();
}




