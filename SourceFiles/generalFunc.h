#ifndef generalFunc
#define generalFunc

#include <string>

unsigned randomUns(unsigned, unsigned);
unsigned randomNormalUns(unsigned, unsigned);
unsigned countMatch(const std::string&, const std::string&);
void sleepMilli(unsigned);
std::string& capitalise(std::string&);
std::string capitalise_copy(const std::string&);
std::string int_to_hex(unsigned);

#endif // generalFunc
