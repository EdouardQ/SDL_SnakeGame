#include <iostream>
#include "game.hpp"

using namespace std;

int main()
{
    bool nextgame = true;
    Game game;

    while (nextgame)
    {
        game = Game();
        nextgame = game.Run();
        game.Close();
    }
    
    cout << "Game has terminated successfully, score: " << game.GetScore() << endl; //", size: " << game.GetSize() << endl;
    return 0;
}