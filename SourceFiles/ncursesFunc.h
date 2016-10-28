#ifndef ncursesFunc
#define ncursesFunc

#include <string>
#include <curses.h>

int TimedPrint(WINDOW*, const std::string&, unsigned, unsigned, unsigned);
void clearLine(WINDOW*, unsigned, unsigned);
void replaceLine(WINDOW*, const std::string&, unsigned, unsigned);
std::string grabAndClearLine(WINDOW*, unsigned, unsigned);
void moveLineUp(WINDOW*, unsigned, unsigned, unsigned);

#endif // ncursesFunc
