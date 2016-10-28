A short C++ program of the terminal word game from the the video game Fallout.

NOTES:

-Probably doesn't work on anything but Windows.
-Make sure all the WordFiles and ConfigFiles folders are kept in the same directory as FalloutWordGame.exe
-Inside config.txt I would probably only mess about with wordAmount, gridNo, maxSpeed, perRow or rolloutSpeed. The values for the other variables need to be with specific ranges for correct
program behaviour.
-The game randomly selects 13 words and 1 answer, so it could be that in some rounds it is impossible to win. I have rarely found this to be the case. I'll probably sort this out one day. Probably.
-The potential words were copied from the internet and are numerous (in the 100000s), so it could be that some contained words are offensive. Sorry about that.
-If you want to compile this yourself, make sure you include the pdcurses and necessary boost libraries library when compiling. You will need to find these files yourself (from the PDcurses download 
on sourceforge and boost homepage) and compile the binaries for your own system. 