#include <string>
#include <random>
#include <ctime>
#include <map>
#include <utility> //pair
#include <sstream>
#include <thread>
#include "generalFunc.h"


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

    return round(distributions[range](generator)); //we round otherwise the decimal is truncated, meaning the normal distribution favours lower numbers.
}

unsigned countMatch(const std::string& s1, const std::string& s2){

    if(s1.size() != s2.size()) throw std::runtime_error("Compared strings do not have the same size.");

    unsigned countNo = 0;

    for(unsigned i = 0; i < s1.size(); ++i)
        if(s1[i] == s2[i]) ++countNo;

    return countNo;

}

void sleepMilli(unsigned t){

    std::this_thread::sleep_for(std::chrono::milliseconds(t));

}

std::string& capitalise(std::string& word){

    for(auto& c : word)
        c = toupper(c);
    return word;

}

std::string capitalise_copy(const std::string& word){

    std::string ret = word; //RVO conditions met, copy elision may occur.
    capitalise(ret); //passes to capitalise taking lvalue reference. NOTE: We do not return this directly otherwise we would remove RVO and copy elision.
    return ret;

}

std::string int_to_hex(unsigned i){
    std::stringstream stream;
    stream << "0x" << std::uppercase << std::hex << i;
    return stream.str();
}




