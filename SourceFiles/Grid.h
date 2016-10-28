#ifndef GRID
#define GRID

#include <vector>
#include <string>
#include <curses.h>

class Grid{
    friend void printGrid(WINDOW*, const Grid&);
    friend int printGridsLineTimed(WINDOW*, std::vector<Grid>::const_iterator, std::vector<Grid>::const_iterator, unsigned);
public:
    Grid(std::vector<std::string>::const_iterator b, std::vector<std::string>::const_iterator e, const std::string&, unsigned, unsigned, unsigned, unsigned);
    std::pair<unsigned, unsigned> borderTop() const {return std::make_pair(posX, posY);}
    std::pair<unsigned, unsigned> borderBot() const {return std::make_pair(posX+LX-1, posY+LY-1);}
    unsigned positionX() const {return posX;}
    unsigned positionY() const {return posY;}
    unsigned lengthX() const {return LX;}
    unsigned lengthY() const {return LY;}
private:
    std::vector<std::string> lines; //the lines of the grid, fully padded.
    unsigned posX, posY; //the position of the top left part of the grid
    unsigned LX, LY; //the length and height of the grids
};

void createGrids(std::vector<Grid>&, const std::vector<std::string>&, const std::string&, unsigned, unsigned, unsigned, unsigned, unsigned, unsigned, unsigned);
void printGrid(WINDOW*, const Grid&);
int printGridsLineTimed(WINDOW*, std::vector<Grid>::const_iterator, std::vector<Grid>::const_iterator, unsigned);
std::string boldGridWord(WINDOW*, const std::vector<Grid>::const_iterator&);
std::string unboldGridWord(WINDOW*, const std::vector<Grid>::const_iterator&);
void moveLeftGrid(WINDOW*, const std::vector<Grid>&, std::vector<Grid>::const_iterator&, unsigned);
void moveRightGrid(WINDOW*, const std::vector<Grid>&, std::vector<Grid>::const_iterator&, unsigned);
void moveUpGrid(WINDOW*, const std::vector<Grid>&, std::vector<Grid>::const_iterator&, unsigned);
void moveDownGrid(WINDOW*, const std::vector<Grid>&, std::vector<Grid>::const_iterator&, unsigned);

#endif // GRID
