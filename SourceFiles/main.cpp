/* Steps for getting curses to work:
1) Download the curses zip from https://sourceforge.net/projects/pdcurses/files/. In the win32 folder do "mingw32-make -f mingwin32.mak" to create pdcurses.lib for static linking.
2) #define PDC_DLL_BUILD needs to be commented out of curses.h otherwise it's expecting a DLL file to link with.

NOTES:

1) wgetch() implicitly calls refresh on the screen.
2) add -static to linker options
3) Download boost obviously and remember to set include directory...

*/

#include <vector>
#include <string>
#include <map>
#include <algorithm> //for max function
#include <iostream>
#include <fstream>
#include <curses.h>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include "generalFunc.h"
#include "ncursesFunc.h"
#include "Grid.h"




int main(){

    //GAME SETTINGS AND FILE READING

    unsigned wordSize = randomUns(4,11); //the selected number of characters in the word for this game

    const std::string fileName = "WordFiles\\word" + std::to_string(wordSize) + ".txt";
    std::ifstream wordFile(fileName);
    std::ifstream cat("WordFiles\\asciiCat.txt");

    if(!wordFile){
        std::cerr << "Could not open file: " << fileName << ". Exiting program." << std::endl;
        sleepMilli(5000);
        return -1;
    }
    if(!cat){
        std::cerr << "Could not open file: " << "asciiCat.txt" << ". Exiting program." << std::endl;
        sleepMilli(5000);
        return -1;
    }

    std::vector<std::string> allWords, catBasket;

    for(std::string s; wordFile >> s; ){
        if(s.size() != wordSize){ //shouldn't happen but just in case
            continue;
        }
        allWords.push_back(capitalise(s));
    }

    for(std::string s; getline(cat, s);)
        catBasket.push_back(s);

    cat.close();
    wordFile.close();

    namespace po = boost::program_options;

    po::options_description options("Configuration Options");
    options.add_options()
        ("gridX", po::value<unsigned>(), "Length of each grid")
        ("gridY", po::value<unsigned>(), "Height of each grid")
        ("wordAmount", po::value<unsigned>(), "Number of words on screen")
        ("gridNo", po::value<unsigned>(), "Number of grids on screen")
        ("perRow", po::value<unsigned>(), "Number of grids per row")
        ("gridPadding", po::value<unsigned>(), "Space between grids")
        ("relX", po::value<unsigned>(), "X position of the leftmost grid")
        ("relY", po::value<unsigned>(), "Y position of the leftmost grid")
        ("maxTries", po::value<unsigned>(), "Attempts the user is allowed")
        ("rolloutSpeed", po::value<unsigned>(), "Controls speed at which text rolls out (Higher is slower)")
        ("screenX", po::value<unsigned>(), "Length of the window screen")
        ("screenY", po::value<unsigned>(), "Height of the window screen");

    po::variables_map vm;
    po::store(po::parse_config_file<char>("ConfigFiles\\config.txt", options), vm);
    po::notify(vm);

    std::map<std::string, unsigned> configuration;

    const unsigned &gridX = configuration["gridX"],
                   &gridY = configuration["gridY"],
                   &wordAmount = configuration["wordAmount"],
                   &gridNo = configuration["gridNo"],
                   &perRow = configuration["perRow"],
                   &gridPadding = configuration["gridPadding"],
                   &relX = configuration["relX"],
                   &relY = configuration["relY"],
                   &maxTries = configuration["maxTries"],
                   &rolloutSpeed = configuration["rolloutSpeed"],
                   &screenX = configuration["screenX"],
                   &screenY = configuration["screenY"];

    if(vm.count("gridX")) configuration["gridX"] = vm["gridX"].as<unsigned>();
    else configuration["gridX"] = 12;

    if(vm.count("gridY")) configuration["gridY"] = vm["gridY"].as<unsigned>();
    else configuration["gridY"] = 16;

    if(vm.count("wordAmount")) configuration["wordAmount"] = vm["wordAmount"].as<unsigned>();
    else configuration["wordAmount"] = 13;

    if(vm.count("gridNo")) configuration["gridNo"] = vm["gridNo"].as<unsigned>();
    else configuration["gridNo"] = 2;

    if(vm.count("perRow")) configuration["perRow"] = vm["perRow"].as<unsigned>();
    else configuration["perRow"] = 5;

    if(vm.count("gridPadding")) configuration["gridPadding"] = vm["gridPadding"].as<unsigned>();
    else configuration["gridPadding"] = 8;

    if(vm.count("relX")) configuration["relX"] = vm["relX"].as<unsigned>();
    else configuration["relX"] = gridPadding;

    if(vm.count("relY")) configuration["relY"] = vm["relY"].as<unsigned>();
    else configuration["relY"] = 6;

    if(vm.count("maxTries")) configuration["maxTries"] = vm["maxTries"].as<unsigned>();
    else configuration["maxTries"] = 4;

    if(vm.count("rolloutSpeed")) configuration["rolloutSpeed"] = vm["rolloutSpeed"].as<unsigned>();
    else configuration["rolloutSpeed"] = 30/gridNo;

    if(vm.count("screenX")) configuration["screenX"] = vm["screenX"].as<unsigned>();
    else{
        unsigned calculatedSize = relX + std::min(perRow, gridNo)*(gridX + gridPadding) - gridPadding + std::max(24U, relX);
        unsigned catSize = catBasket.begin()->size();
        configuration["screenX"] = std::max(calculatedSize, catSize);
    }

    if(vm.count("screenY")) configuration["screenY"] = vm["screenY"].as<unsigned>();
    else{
        unsigned rows = (gridNo - 1)/perRow + 1;
        unsigned calculatedSize = relY + rows*(gridY + 1) - 2 + std::max(3U, relY - 5);
        unsigned catSize = catBasket.size();
        configuration["screenY"] = std::max(calculatedSize, catSize);
    }

    //OBTAIN THE WORDS TO USE IN THE GAME

    std::vector<std::string> selectWords;

    for(unsigned i = 0; i < wordAmount; ++i){
        std::vector<std::string>::const_iterator it = allWords.begin();
        it += randomUns(0,allWords.size()-1);
        selectWords.push_back(*it);
        allWords.erase(it);
    }

    //CREATE THE VECTOR OF GRIDS

    const std::string filler = "@#!:;^_+=<>$%*[]{}?-/\\|.`'"; //the filler characters. Make sure this contains no alpha characters.
    std::vector<Grid> gridVec; //vector containing our list of Grids.
    createGrids(gridVec, selectWords, filler, gridNo, perRow, relX, relY, gridPadding, gridX, gridY); //populates gridVec with the grids of equal size

    //START THE SCREEN

    initscr();
    resize_term(screenY, screenX); //resize screen
    cbreak(); //characters typed by user are immediately available to curses.
    keypad(stdscr, true); //enable keyboard commands on standard window
    noecho(); //don't print keyboard input
    curs_set(0); //no cursor displayed


    //PRINT OUT EVERYTHING

    unsigned tries = 0;
    unsigned currentX = relX + (std::min(perRow, gridNo))*(gridX + gridPadding) - gridPadding + 2; //X and Y position of where the currently selected word will be displayed
    unsigned currentY = relY + ((gridNo-1)/perRow + 1)*(gridY + 1) - 2;

    {

        unsigned period = rolloutSpeed;
        if(TimedPrint(stdscr, "Welcome to ROBCO Industries (TM) Termlink.", relX-gridPadding+1, relY-5, period) != ERR) period = 0;
        if(TimedPrint(stdscr, "Password Required", relX-gridPadding+1, relY-4, period) != ERR) period = 0;
        if(TimedPrint(stdscr, "Attempts remaining: " + std::to_string(maxTries-tries), relX-gridPadding+1, relY-2, period) != ERR) period = 0;
        if(printGridsLineTimed(stdscr, gridVec.cbegin(), gridVec.cend(), period) != ERR) period = 0;
        if(TimedPrint(stdscr, ">", currentX - 1, currentY, period) != ERR) period = 0;

    }



    //MAIN LOOP



    std::vector<Grid>::const_iterator currentGrid = gridVec.cbegin(); //the grid our cursor is on: the first. Does not change the grid, so const.
    wmove(stdscr, currentGrid->borderTop().second, currentGrid->borderTop().first); //move to the top left of the first grid
    std::string currentWord = boldGridWord(stdscr, currentGrid);
    std::string oldWord; //the previous word selected before a key press. Indicates whether we need to use TimedPrint.
    std::string answer = selectWords[randomUns(0,selectWords.size()-1)];

    while(true){

        int result = ERR; //holds any potential key press that happens during TimedPrint.

        if(oldWord != currentWord){
            clearLine(stdscr, currentX, currentY);
            result = TimedPrint(stdscr, currentWord, currentX, currentY, 100);
        }

        switch((result != ERR) ? result : wgetch(stdscr)){ //did a key press happen while printing? If so, use the result. Otherwise wait for a key press.
            case KEY_UP:
            case 'w':
                oldWord = unboldGridWord(stdscr, currentGrid);
                moveUpGrid(stdscr, gridVec, currentGrid, perRow);
                currentWord = boldGridWord(stdscr, currentGrid);
                break;
            case KEY_DOWN:
            case 's':
                oldWord = unboldGridWord(stdscr, currentGrid);
                moveDownGrid(stdscr, gridVec, currentGrid, perRow);
                currentWord = boldGridWord(stdscr, currentGrid);
                break;
            case KEY_LEFT:
            case 'a':
                oldWord = unboldGridWord(stdscr, currentGrid);
                moveLeftGrid(stdscr, gridVec, currentGrid, perRow);
                currentWord = boldGridWord(stdscr, currentGrid);
                break;
            case KEY_RIGHT:
            case 'd':
                oldWord = unboldGridWord(stdscr, currentGrid);
                moveRightGrid(stdscr, gridVec, currentGrid, perRow);
                currentWord = boldGridWord(stdscr, currentGrid);
                break;
            case 0xA: //this is what is returned when we press enter. It is the ASCII value for a newline character (since KEY_ENTER did not seem to provide the right value).
                oldWord = currentWord;
                if(currentWord == answer){
                    ++tries;
                    unsigned lineMoveUp = 2; //how many lines we want to move up
                    for(unsigned i = relY + lineMoveUp; i <= currentY - 1 ; ++i){
                        moveLineUp(stdscr, currentX - 1, i, lineMoveUp);
                    }
                    replaceLine(stdscr, "Attempts remaining: " + std::to_string(maxTries-tries), relX-gridPadding+1, relY-2);
                    replaceLine(stdscr, ">" + currentWord, currentX - 1, currentY - 2);
                    replaceLine(stdscr, ">Password Accepted.", currentX - 1, currentY - 1);
                    replaceLine(stdscr, ">", currentX - 1, currentY);
                    wrefresh(stdscr);
                    sleepMilli(5000);

                    //PRINT THE CAT

                    unsigned centreX = (screenX - (catBasket.end()-1)->size())/2; //position needed to print cat in the centre horizontally
                    unsigned centreY = (screenY - catBasket.size())/2; //position needed to print cat in the centre vertically
                    unsigned speed = 2500/screenY; //faster if screen is bigger

                    for(unsigned i = 0; i <= screenY/2; ++i){ //clear half the screen first
                        for(unsigned y = 0; y < screenY; ++y){
                            moveLineUp(stdscr, 0, y, 1);
                        }
                        wrefresh(stdscr);
                        sleepMilli(speed);
                    }

                    for(auto it = catBasket.begin(); it != catBasket.end(); ++it){ //print the cat
                        for(unsigned y = 0; y < screenY; ++y){
                            moveLineUp(stdscr, 0, y, 1);
                        }
                        replaceLine(stdscr, *it, centreX, screenY-1);
                        wrefresh(stdscr);
                        sleepMilli(speed);
                    } //cat is all printed after this, but it won't be at the centre vertically

                    for(unsigned i = 0; i < ((screenY-centreY) - catBasket.size()); ++i){ //move the cat up the remaining distance to get to the centre
                        for(unsigned y = 0; y < screenY; ++y){
                            moveLineUp(stdscr, 0, y, 1);
                        }
                        wrefresh(stdscr);
                        sleepMilli(speed);
                    }

                    sleepMilli(10000);

                    //EXIT PROGRAM

                    endwin();
                    return 0;
                }
                else{
                    if(!isalpha(currentWord[0])) {//then we are on a filler character.
                        unsigned lineMoveUp = 2; //how many lines we want to move up
                        for(unsigned i = relY + lineMoveUp; i <= currentY -1 ; ++i){
                            moveLineUp(stdscr, currentX - 1, i, lineMoveUp);
                        }
                        replaceLine(stdscr, ">" + currentWord, currentX - 1, currentY - 2);
                        replaceLine(stdscr, ">Error", currentX - 1, currentY - 1);
                    }
                    else{ //we are on a word
                        ++tries;
                        if(tries == maxTries){
                            unsigned lineMoveUp = 3; //how many lines we want to move up
                            for(unsigned i = relY + lineMoveUp; i <= currentY -1 ; ++i){
                                moveLineUp(stdscr, currentX - 1, i, lineMoveUp);
                            }
                            replaceLine(stdscr, "Attempts remaining: " + std::to_string(maxTries-tries), relX-gridPadding+1, relY-2);
                            replaceLine(stdscr, ">" + currentWord, currentX - 1, currentY - 3);
                            replaceLine(stdscr, ">Entry Denied.", currentX - 1, currentY - 2);
                            replaceLine(stdscr, ">Initialising Lockout.", currentX - 1, currentY - 1);
                            replaceLine(stdscr, ">", currentX - 1, currentY);
                            wrefresh(stdscr);
                            sleepMilli(5000);
                            endwin();
                            return 0;
                        }
                        else{
                            unsigned lineMoveUp = 3; //how many lines we want to move up
                            for(unsigned i = relY + lineMoveUp; i <= currentY -1 ; ++i){
                                moveLineUp(stdscr, currentX - 1, i, lineMoveUp);
                            }
                            replaceLine(stdscr, "Attempts remaining: " + std::to_string(4-tries), relX-gridPadding+1, relY-2);
                            replaceLine(stdscr, ">" + currentWord, currentX - 1, currentY - 3);
                            replaceLine(stdscr, ">Entry Denied.", currentX - 1, currentY - 2);
                            replaceLine(stdscr, ">Likeness=" + std::to_string(countMatch(currentWord, answer)), currentX - 1, currentY - 1);
                        }
                    }
                }
                break;
            default:
                oldWord = currentWord;
                break;
        } //switch statement

        wrefresh(stdscr);
        flushinp(); //flush input buffer, doesn't work for repeat keys though (from holding down a key)
        sleepMilli(1000/30); /* controls how fast the cursor can move across the screen when a key is held down. I think there are framerate issues
        which means if there is no delay then the cursor kind of skips across the screen and it looks very messy, so we need to slow it down to make it
        smooth, to about 30 FPS.*/
    }

    endwin();

}
