#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <set>
#include "generalFunc.h"
#include "ncursesFunc.h"
#include "Grid.h"


Grid::Grid(std::vector<std::string>::const_iterator b, std::vector<std::string>::const_iterator e,
                                                       const std::string& filler, unsigned px, unsigned py,
                                                       unsigned lx, unsigned ly) : posX(px), posY(py), LX(lx), LY(ly)
{

    if(filler.size() == 0) throw std::runtime_error("Filler string supplied to Grid constructor is empty.");

    unsigned currentLength = 0; //how much of the current gridSpace has been used up
    std::for_each(b, e, [&currentLength](const std::string& word){currentLength += word.size();}); //count up how much space is taken up by the non-filler words

    /*Each non-filler word will be preceded by a random amount of random filler characters. However we must ensure that we don't use up s characters. Any remaining
    space will be taken up by filler characters too. */

    unsigned s = LX*LY;
    if(currentLength > s) throw std::runtime_error("Length of words is more than the size of the grid.");

    unsigned upperLim = (s - currentLength)/(e-b+1); //the number of filler sequences is e-b+1
    if(upperLim < 4) throw std::runtime_error("The average filler characters to print between words is " + std::to_string(upperLim) +
                                              "\n which is far too low. Either decrease the number of words, increase"
                                              "\n the number of grids or increase the grid size.");

    std::string words;

    for(auto it = b; it != e; ++it){
        unsigned fillerAmount = randomNormalUns(upperLim, 1);
        for(unsigned i = 0; i < fillerAmount; ++i){
            words.push_back(filler[randomUns(0, filler.size()-1)]); //generate the filler characters preceding the word
        }
        currentLength += fillerAmount;
        words += *it;
    }

    if(currentLength > s) throw std::logic_error("Too much filler allocation. Could be random unluckiness from normal distribution function."); //I can remove this once I'm confident I've done everything right

    for(unsigned i = 0; i < (s - currentLength); ++i){ //generate the remaining filler characters if necessary
        words.push_back(filler[randomUns(0, filler.size()-1)]);
    }

    for(unsigned y = 0; y < LY; ++y){ //now add the lines of text to our vector "lines"
        lines.push_back(words.substr(y*LX, LX));
    }

}

void createGrids(std::vector<Grid>& gridVec, const std::vector<std::string>& selectWords, const std::string& filler,
                 unsigned No, unsigned perRow, unsigned posX, unsigned posY, unsigned padding, unsigned LX, unsigned LY){
    /*create #No grids from words in selectWords in the gridVec vector, with filler between the words,
    first grid starting at (posX, posY) and padding between each grid, all with lengthX, lengthY of LX/LY, and change
    the y-position of the grids every perRow grid.*/

    if(No == 0) return; //nothing to do

    unsigned wordNo = selectWords.size()/No; //words per grid, the number from this division is truncated
    unsigned remainder = selectWords.size()%No; //remainders, distributed randomly between grids.

    std::set<unsigned> receivers; //the grids that will receive an extra word
    while(receivers.size() < remainder) receivers.insert(randomUns(0, No-1));

    unsigned extra = (receivers.find(0) != receivers.end()) ? 1 : 0;
    unsigned start_index = 0;
    unsigned end_index = wordNo + extra;

    for(unsigned i = 0; i < No; ++i){

        auto b = selectWords.begin()+start_index;
        auto e = selectWords.begin()+end_index;

        gridVec.emplace_back(b, e, filler, posX+(LX+padding)*(i%perRow), posY + (i/perRow)*(LY+1), LX, LY);

        extra = (receivers.find(i+1) != receivers.end()) ? 1 : 0;
        start_index = end_index;
        end_index += wordNo+extra;

    }

}

void printGrid(WINDOW* w, const Grid& g){

    unsigned x = 0, y = 0;
    getyx(w, y, x); //obtain current cursor position
    wmove(w, g.positionY(), g.positionX());

    for(unsigned y = 0; y < g.lengthY(); ++y){
        waddstr(w, g.lines[y].c_str());
        wmove(w, g.positionY()+(y+1), g.positionX());
    }

    wmove(w, y, x); //position at the top left of the grid.

}

int printGridsLineTimed(WINDOW* w, std::vector<Grid>::const_iterator b, std::vector<Grid>::const_iterator e, unsigned period){  //prints first line of all grids, then second line of all grids, etc etc... Timed and with address in padding.

    //no need to save current position and return back to it, since TimedPrint does that for us.
    std::vector<unsigned> lineAmounts;
    for(auto it = b; it != e; ++it)
        lineAmounts.push_back(std::accumulate(b, it, 0, [](unsigned l, const Grid& r){ return l += r.lengthY(); })); //used for the hexadecimal address since not all grids are the same height

    int keyPress = 0;

    for(auto it = b; it != e; ++it){
        for(unsigned y = 0; y < it->lengthY(); ++y){
            std::string address = int_to_hex(33856+(y*12) + 12*lineAmounts[it-b]);
            if((keyPress = TimedPrint(w, address, it->positionX() - address.size() - 1, it->positionY() + y, period)) != ERR) period = 0;
            if((keyPress = TimedPrint(w, it->lines[y].c_str(), it->positionX(), it->positionY() + y, period)) != ERR) period = 0;
        }

    }

    return keyPress;

}

std::string boldGridWord(WINDOW* w, const std::vector<Grid>::const_iterator& curr){ //bolds the word in the grid the cursor is on and returns the entire bolded word

    unsigned x = 0, y = 0;
    getyx(w, y, x); //obtain current cursor position
    std::string newWord;

    wattron(w, A_BLINK);
    //Are we on a filler character already?
    if(isalpha(winch(w))){ //We are on a word
        char c = winch(w);
        waddch(w, c);
        wmove(w, y, x);
        newWord.push_back(c);
    }
    else{ //we must be on a filler char. Bold the filler character and leave
        char c = winch(w);
        waddch(w, c);
        wmove(w, y, x);
        wattroff(w, A_BLINK);
        newWord.push_back(c);
        return newWord;
    }


    unsigned newX = x, newY = y; //moving leftwards

    while(true){
        if(newX == curr->borderTop().first){
            if(newY == curr->borderTop().second) break; //we are at the top left and there is nothing to do
            newX += (curr->lengthX() - 1);
            --newY;
        }
        else --newX;

        char c = mvwinch(w, newY, newX);

        if(!isalpha(c)) break;//we hit a filler character, time to end
        else{//presumably we've hit a character
            waddch(w, c);
            wmove(w, newY, newX);
            newWord.insert(newWord.begin(), c);
        }
    }

    newX = x, newY = y; //moving rightwards

    while(true){
        if(newX == curr->borderBot().first){
            if(newY == curr->borderBot().second) break; //we are at the bottom right and there is nothing to do
            newX -= (curr->lengthX() - 1);
            ++newY;
        }
        else ++newX;

        char c = mvwinch(w, newY, newX);

        if(!isalpha(c)){ //we hit a filler character, time to end
            break;
        } else{//presumably we've hit a character
            waddch(w, c);
            wmove(w, newY, newX);
            newWord.push_back(c);
        }
    }

    wattroff(w, A_BLINK);
    wmove(w, y, x);
    return newWord;


}

std::string unboldGridWord(WINDOW* w, const std::vector<Grid>::const_iterator& curr){ //unbolds the word the cursor is on and returns the entire unbolded word

    unsigned x = 0, y = 0;
    getyx(w, y, x); //obtain current cursor position
    std::string oldWord;

    wattroff(w, A_BLINK);
    //Are we on a filler character already?
    if(isalpha(winch(w))){ //We are on a word
        char c = winch(w);
        waddch(w, c);
        wmove(w, y, x);
        oldWord.push_back(c);
    }
    else{ //we are on a filler, bold the filler character and leave
        char c = winch(w);
        waddch(w, c);
        wmove(w, y, x);
        oldWord.push_back(c);
        return oldWord;
    }


    unsigned newX = x, newY = y; //moving leftwards

    while(true){
        if(newX == curr->borderTop().first){
            if(newY == curr->borderTop().second) break; //we are at the top left and there is nothing to do
            newX += (curr->lengthX() - 1);
            --newY;
        }
        else --newX;

        char c = mvwinch(w, newY, newX);

        if(!isalpha(c)) break;//we hit a filler character, time to end
        else{//presumably we've hit a character
            waddch(w, c);
            wmove(w, newY, newX);
            oldWord.insert(oldWord.begin(), c);
        }
    }

    newX = x, newY = y; //moving rightwards

    while(true){
        if(newX == curr->borderBot().first){
            if(newY == curr->borderBot().second) break; //we are at the bottom right and there is nothing to do
            newX -= (curr->lengthX() - 1);
            ++newY;
        }
        else ++newX;

        char c = mvwinch(w, newY, newX);

        if(!isalpha(c)){ //we hit a filler character, time to end
            break;
        } else{//presumably we've hit a character
            waddch(w, c);
            wmove(w, newY, newX);
            oldWord.push_back(c);
        }
    }

    wmove(w, y, x);//return cursor to original position
    return oldWord;

}

void moveLeftGrid(WINDOW* w, const std::vector<Grid>& gridVec, std::vector<Grid>::const_iterator& curr, unsigned perRow){

    unsigned x = 0, y = 0;
    getyx(w, y, x); //obtain current cursor position

    unsigned newX = x, newY = y;

    if(isalpha(winch(w))){ //our cursor is already on a word
        /*Two cases here:
        1) The word section is at the beginning of the line. In which case we skip over the rest of the word and then decide whether to move to the next grid.
        2) The word is not at the end of the line. In which case we move to the previous filler character. */
        while(isalpha(winch(w)) && newX > curr->borderTop().first){
            wmove(w, y, --newX);
        } //ends when it doesn't encounter an alpha numeric character or hits the left of the grid
        if(newX == curr->borderTop().first){ //checking whether we hit the left side of the grid
            if((curr - gridVec.begin())%perRow == 0){ //then there is no previous grid and we need to return
                wmove(w, y, x);
                return;
            } else{
                --curr; //our current grid has changed
                if(y > curr->borderBot().second) newY = curr->borderBot().second;
                else if(y < curr->borderTop().second) newY = curr->borderTop().second;
                wmove(w, newY, curr->borderBot().first);
            }
        }
    }
    else{
        /*our cursor is on a filler character. Two cases here:
        1) The filler character is at the beginning of the line, in which case we decide whether to move to a previous grid.
        2) The filler character is not at the beginning of the line and we just move to the previous character. */
        if(newX == curr->borderTop().first){ //checking whether we are at the left side of the grid
            if((curr - gridVec.begin())%perRow == 0){ //then there is no previous grid and we need to return
                wmove(w, y, x);
                return;
            } else{
                --curr; //our current grid has changed
                if(y > curr->borderBot().second) newY = curr->borderBot().second;
                else if(y < curr->borderTop().second) newY = curr->borderTop().second;
                wmove(w, newY, curr->borderBot().first);
            }
        }
        else wmove(w, y, --newX);
    }

}

void moveRightGrid(WINDOW* w, const std::vector<Grid>& gridVec, std::vector<Grid>::const_iterator& curr, unsigned perRow){

    unsigned x = 0, y = 0;
    getyx(w, y, x); //obtain current cursor position

    unsigned newX = x, newY = y;

    if(isalpha(winch(w))){ //our cursor is already on a word
        /*Two cases here:
        1) The word section is at the end of the line. In which case we skip over the rest of the word and then decide whether to move to the next grid.
        2) The word is not at the end of the line. In which case we move to the next filler character. */
        while(isalpha(winch(w)) && newX < curr->borderBot().first){
            wmove(w, y, ++newX);
        } //ends when it doesn't encounter an alpha numeric character or hits the right side of the grid
        if(newX == curr->borderBot().first){ //checking whether we hit the right side of the grid
            if((curr + 1 - gridVec.begin())%perRow == 0 || curr == (gridVec.end() - 1)){ //then there is no next grid and we need to return
                wmove(w, y, x);
                return;
            } else{
                ++curr;
                if(y > curr->borderBot().second) newY = curr->borderBot().second;
                else if(y < curr->borderTop().second) newY = curr->borderTop().second;
                wmove(w, newY, curr->borderTop().first);
            }
        }
    }
    else{
        /*our cursor is on a filler character. Two cases here:
        1) The filler character is at the end of the line, in which case we decide whether to move to the next grid.
        2) The filler character is not at the end of the line and we just move to the next character. */
        if(newX == curr->borderBot().first){ //checking whether we are at the right side of the grid
            if((curr + 1 - gridVec.begin())%perRow == 0 || curr == (gridVec.end() - 1)){ //then there is no next grid and we need to return
                wmove(w, y, x);
                return;
            } else{
                ++curr;
                if(y > curr->borderBot().second) newY = curr->borderBot().second;
                else if(y < curr->borderTop().second) newY = curr->borderTop().second;
                wmove(w, newY, curr->borderTop().first);
            }
        }
        else wmove(w, y, ++newX);
    }

}

void moveUpGrid(WINDOW* w, const std::vector<Grid>& gridVec, std::vector<Grid>::const_iterator& curr, unsigned perRow){

    unsigned x = 0, y = 0;
    getyx(w, y, x); //obtain current cursor position

    unsigned newX = x;

    if(y == curr->borderTop().second){ //we are at the top of the grid
        if((curr - gridVec.begin()) < perRow) return; //we are in the first row, no grid to move to
        else curr -= perRow;

        if(x < curr->borderTop().first) newX = curr->borderTop().first;
        else if(x > curr->borderBot().first) newX = curr->borderBot().first;
        wmove(w, curr->borderBot().second, newX);
    }
    else wmove(w, y-1, x);

    return;

}

void moveDownGrid(WINDOW* w, const std::vector<Grid>& gridVec, std::vector<Grid>::const_iterator& curr, unsigned perRow){

    unsigned x = 0, y = 0;
    getyx(w, y, x); //obtain current cursor position

    unsigned newX = x;

    if(y == curr->borderBot().second){ //we are at the bottom of the grid
        if((gridVec.end() - 1 - curr) < perRow){//there is no grid DIRECTLY below
            //Either the last grid is on the same row, or on the next row but before the x-position of the current one
            if((gridVec.end()-1-gridVec.begin())/perRow == (curr-gridVec.begin())/perRow) return; //last grid is on the same row
            else curr = gridVec.end() - 1; //last grid is on the next row
        }
        else curr += perRow; //there is a grid directly below

        if(x < curr->borderTop().first) newX = curr->borderTop().first;
        else if(x > curr->borderBot().first) newX = curr->borderBot().first;
        wmove(w, curr->borderTop().second, newX);
    }
    else wmove(w, y+1, x);
    return;

}
