#include "ConnectFour.h"
#include <iostream>
#include <string>
#include <limits>

using namespace std;

int main() {
    const int GAME_COUNT = 5;

    // Create an array of five independent ConnectFour objects.
    // Each element has its own board, heights and state.
    ConnectFour* games = new ConnectFour[GAME_COUNT];

    cout << "=== CONNECT FOUR MULTI-GAME MANAGER ===\n";

    while (true) {
        cout << "\nChoose a game (1-" << GAME_COUNT << "), or 0 to exit: ";
        int sel;
        if (!(cin >> sel)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        if (sel == 0) break;
        if (sel < 1 || sel > GAME_COUNT) {
            cout << "Invalid selection. Please choose 1-" << GAME_COUNT << " or 0 to quit.\n";
            continue;
        }

        int idx = sel - 1;

        // Ask for optional shape file for the chosen game.
        cout << "Enter shape filename for game " << sel << " (leave empty to skip): ";
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // flush newline
        string fname;
        getline(cin, fname);
        if (!fname.empty()) {
            games[idx].loadFromFile(fname);
        }

        // Choose mode and set it on the specific game object.
        cout << "Choose mode for game " << sel << " (1 = Player vs Computer, 2 = Player vs Player): ";
        int mode;
        cin >> mode;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            mode = 1;
        }
        games[idx].setVsComputer(mode == 1);

        // Start the game. playGame will not ask for shape or mode again.
        cout << "\nStarting game " << sel << "...\n";
        games[idx].playGame();
        cout << "\nFinished game " << sel << ". Returning to manager.\n";
    }

    delete[] games;
    cout << "Goodbye!\n";
    return 0;
}
