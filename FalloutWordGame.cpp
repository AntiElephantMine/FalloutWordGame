#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <ctime>

using std::string;
using std::cout; using std::endl; using std::cin;

/*As it stands, the game is too hard. Work out how to add an interactive interface from which we can select
potential matches (just like Fallout)*/

int main(){

    std::ifstream input("D:\\HDD Documents\\HDD Users\\Stefan\\Documents\\C++\\Side Projects\\FalloutWordGame\\words.txt");

    if(!input){
        cout << "Failed to open file containing words. Closing program." << endl;
        return -1;
    }

    std::vector<string> words;

    for(string word; input >> word;)
        words.push_back(word);

    input.close();

    std::default_random_engine e(time(0));
    e.discard(10); /*the FIRST value output by e does not change much with time and so uniform_int_distribution will always return
    the same value when the program is loaded, so we discard a few. I think default_random_engine works a bit like chaos theory: the first
    values are always quite similar, but the next numbers output by the engine are completely different even if there is only a slight change
    in that first value. */
    std::uniform_int_distribution<unsigned> u(0, words.size()-1);

    string answer = words[u(e)];
    bool success = false;

    cout << "Work out the " << answer.size() << "-letter word." << endl;
    cout << "Each answer shows how many positionally correct letters were entered. Good luck!" << endl;

    unsigned maxTries = (answer.size() < 5 ? 5 : (answer.size() < 11 ? 6 : 7));

    for(unsigned tries = 0; tries < maxTries; ++tries){

        string input;

        cout << "(ATTEMPT: " << tries+1 << " OF " << maxTries << ") Please enter an answer: ";
        while(cin >> input){
            if(input.size() != answer.size()){
                cout << "Incorrect size. Enter a valid answer: ";
                continue;
            }
            break;
        }

        unsigned correctIndex = 0;
        for(unsigned j = 0; j < answer.size(); ++j)
            if(tolower(input[j]) == tolower(answer[j])) ++correctIndex;

        cout << "Number of correct letters: " << correctIndex << " of " << answer.size() << endl;

        if(correctIndex == answer.size()){
            cout << "\nCorrect answer! Access to mainframe granted." << endl;
            success = true;
            break;
        }


    }

    if(success) cout << "Beep bop beep boop." << char(7) << endl;
    else cout << "\nOut of tries! The answer was " << answer << ".\nBad luck, thanks for playing you worthless animal." << endl;


}





