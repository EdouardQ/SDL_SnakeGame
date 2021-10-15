#include <iostream>
#include "game.hpp"

using namespace std;

int main()
{
    Game game = Game();
    game.Run();
    cout << "Game has terminated successfully, score: " << game.GetScore() << endl; //", size: " << game.GetSize() << endl;
    return 0;
}