#include <iostream>
#include "game.hpp"

using namespace std;

int main(int argc, char* argv[])
{
    Game game = Game();
    game.Run();
    cout << "Game has terminated successfully, score: " << game.GetScore()
        << ", size: " << game.GetSize() << endl;
    return 0;
}