#include "ConnectFour.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cctype>
#include <limits>

using namespace std;

// ---------------------------- Cell implementation ----------------------------
// The Cell class is a tiny wrapper for a board square.
// It stores its column letter, row number and a state.
// The stream operators let us print a single-cell symbol easily.

ConnectFour::Cell::Cell() : column('a'), row(1), state(CellState::EMPTY) {}
ConnectFour::Cell::Cell(char c, int r, CellState s) : column(c), row(r), state(s) {}

char ConnectFour::Cell::getColumn() const { return column; }
int ConnectFour::Cell::getRow() const { return row; }
ConnectFour::CellState ConnectFour::Cell::getState() const { return state; }
void ConnectFour::Cell::setState(ConnectFour::CellState s) { state = s; }

bool ConnectFour::Cell::operator==(const Cell& other) const {
    // For game-state comparisons we only care about the piece state.
    return state == other.state;
}

// These ++/-- operators provide a cyclic change through the enum values.
ConnectFour::Cell& ConnectFour::Cell::operator++() {
    int x = static_cast<int>(state);
    x = (x + 1) % 4;
    state = static_cast<CellState>(x);
    return *this;
}
ConnectFour::Cell ConnectFour::Cell::operator++(int) {
    Cell tmp = *this;
    ++(*this);
    return tmp;
}
ConnectFour::Cell& ConnectFour::Cell::operator--() {
    int x = static_cast<int>(state);
    x = (x + 3) % 4;
    state = static_cast<CellState>(x);
    return *this;
}
ConnectFour::Cell ConnectFour::Cell::operator--(int) {
    Cell tmp = *this;
    --(*this);
    return tmp;
}

ostream& operator<<(ostream& os, const ConnectFour::Cell& c) {
    // We map the internal enum to a single display character.
    char ch = '.';
    switch (c.state) {
        case ConnectFour::CellState::EMPTY:    ch = '.'; break;
        case ConnectFour::CellState::USER1:    ch = 'X'; break;
        case ConnectFour::CellState::USER2:    ch = 'O'; break;
        case ConnectFour::CellState::COMPUTER: ch = 'C'; break;
    }
    os << ch;
    return os;
}

istream& operator>>(istream& is, ConnectFour::Cell& c) {
    // Read tokens like "a3" (column letter + row number).
    string tok;
    if (!(is >> tok)) return is;
    if (tok.size() < 2 || !isalpha(tok[0])) {
        is.setstate(ios::failbit);
        return is;
    }
    c.column = tolower(tok[0]);
    c.row = stoi(tok.substr(1));
    return is;
}

// ---------------------------- Board helpers ----------------------------
// The board is stored as a dynamic 2D array: Cell** gameBoard.
// colHeights[c] says how many playable cells the column has (from bottom).
// rows is the maximum playable height across all columns.

void ConnectFour::initializeBoard() {
    // Allocate rows x cols and set each cell to EMPTY with coordinates.
    gameBoard = new Cell*[rows];
    for (int r = 0; r < rows; ++r) {
        gameBoard[r] = new Cell[cols];
        for (int c = 0; c < cols; ++c) {
            gameBoard[r][c] = Cell(static_cast<char>('a' + c), r + 1, CellState::EMPTY);
        }
    }
}

void ConnectFour::deallocateBoard() {
    // Free the allocated 2D array.
    if (gameBoard != nullptr) {
        for (int r = 0; r < rows; ++r) delete[] gameBoard[r];
        delete[] gameBoard;
        gameBoard = nullptr;
    }
}

// ------------------------ Constructors / Destructor ------------------------
// Constructors ensure colHeights is allocated and board initialized.
// isVsComputer defaults to false; main will set it as needed.

ConnectFour::ConnectFour()
    : rows(5), cols(5), gameBoard(nullptr), colHeights(nullptr),
      gameEnded(false), winner(CellState::EMPTY), isVsComputer(false)
{
    colHeights = new int[cols];
    for (int i = 0; i < cols; ++i) colHeights[i] = rows;
    initializeBoard();
}

ConnectFour::ConnectFour(int r, int c)
    : rows(r), cols(c), gameBoard(nullptr), colHeights(nullptr),
      gameEnded(false), winner(CellState::EMPTY), isVsComputer(false)
{
    if (rows < 4) rows = 4;
    if (cols < 4) cols = 4;
    colHeights = new int[cols];
    for (int i = 0; i < cols; ++i) colHeights[i] = rows;
    initializeBoard();
}

ConnectFour::ConnectFour(const string& filename)
    : gameBoard(nullptr), colHeights(nullptr),
      gameEnded(false), winner(CellState::EMPTY), isVsComputer(false)
{
    loadFromFile(filename);
}

ConnectFour::ConnectFour(const ConnectFour& o)
    : rows(o.rows), cols(o.cols), gameBoard(nullptr), colHeights(nullptr),
      gameEnded(o.gameEnded), winner(o.winner), isVsComputer(o.isVsComputer)
{
    // Deep copy colHeights and gameBoard
    colHeights = new int[cols];
    for (int c = 0; c < cols; ++c) colHeights[c] = o.colHeights[c];

    gameBoard = new Cell*[rows];
    for (int r = 0; r < rows; ++r) {
        gameBoard[r] = new Cell[cols];
        for (int c = 0; c < cols; ++c) gameBoard[r][c] = o.gameBoard[r][c];
    }
}

ConnectFour& ConnectFour::operator=(const ConnectFour& o) {
    if (this == &o) return *this;

    deallocateBoard();
    delete[] colHeights;

    rows = o.rows;
    cols = o.cols;
    gameEnded = o.gameEnded;
    winner = o.winner;
    isVsComputer = o.isVsComputer;

    colHeights = new int[cols];
    for (int c = 0; c < cols; ++c) colHeights[c] = o.colHeights[c];

    gameBoard = new Cell*[rows];
    for (int r = 0; r < rows; ++r) {
        gameBoard[r] = new Cell[cols];
        for (int c = 0; c < cols; ++c) gameBoard[r][c] = o.gameBoard[r][c];
    }
    return *this;
}

ConnectFour::~ConnectFour() {
    deallocateBoard();
    delete[] colHeights;
    colHeights = nullptr;
}

// ---------------------------- Game core ----------------------------
// find the lowest available cell in a column. (This is basically gravity)
// Returns row index or -1 if column is not playable or full.

int ConnectFour::findLowestEmpty(int col) const {
    if (col < 0 || col >= cols) return -1;
    int playable = colHeights[col];
    if (playable <= 0) return -1;
    int minRow = rows - playable;
    for (int r = rows - 1; r >= minRow; --r)
        if (gameBoard[r][col].getState() == CellState::EMPTY) return r;
    return -1;
}

bool ConnectFour::isBoardFull() const {
    // If any playable cell is empty, board is not full.
    for (int c = 0; c < cols; ++c) {
        int playable = colHeights[c];
        int minRow = rows - playable;
        for (int r = minRow; r < rows; ++r)
            if (gameBoard[r][c].getState() == CellState::EMPTY) return false;
    }
    return true;
}

// Check up to 7 positions along a direction to find 4 in a row.
// This respects the playable region of each column.
bool ConnectFour::checkDirection(int r, int c, int dr, int dc, CellState p) const {
    int count = 0;
    for (int step = -3; step <= 3; ++step) {
        int rr = r + dr * step;
        int cc = c + dc * step;
        if (rr < 0 || rr >= rows || cc < 0 || cc >= cols) {
            count = 0;
            continue;
        }
        int playable = colHeights[cc];
        int minRow = rows - playable;
        if (playable <= 0 || rr < minRow) {
            count = 0;
            continue;
        }
        if (gameBoard[rr][cc].getState() == p) {
            ++count;
            if (count >= 4) return true;
        } else {
            count = 0;
        }
    }
    return false;
}

bool ConnectFour::checkWin(int r, int c, CellState p) const {
    // Horizontal, vertical, diag \ and diag /
    return checkDirection(r, c, 0, 1, p) ||
           checkDirection(r, c, 1, 0, p) ||
           checkDirection(r, c, 1, 1, p) ||
           checkDirection(r, c, 1, -1, p);
}

// Simple AI:
// 1) Try immediate winning move.
// 2) Block user's immediate winning move.
// 3) Random valid column.
// 4) First valid fallback.
int ConnectFour::computerMove() {
    static bool seeded = false;
    if (!seeded) {
        srand(static_cast<unsigned>(time(nullptr)));
        seeded = true;
    }

    // Try to win
    for (int c = 0; c < cols; ++c) {
        int r = findLowestEmpty(c);
        if (r == -1) continue;
        gameBoard[r][c].setState(CellState::COMPUTER);
        bool win = checkWin(r, c, CellState::COMPUTER);
        gameBoard[r][c].setState(CellState::EMPTY);
        if (win) return c;
    }

    // Block user (USER1)
    for (int c = 0; c < cols; ++c) {
        int r = findLowestEmpty(c);
        if (r == -1) continue;
        gameBoard[r][c].setState(CellState::USER1);
        bool win = checkWin(r, c, CellState::USER1);
        gameBoard[r][c].setState(CellState::EMPTY);
        if (win) return c;
    }

    // Random attempts
    for (int i = 0; i < 50; ++i) {
        int c = rand() % cols;
        if (findLowestEmpty(c) != -1) return c;
    }

    // Fallback
    for (int c = 0; c < cols; ++c)
        if (findLowestEmpty(c) != -1) return c;

    return -1;
}

// Place a piece for player p into a column letter (like 'a').
// Does basic validation and updates gameEnded/winner when appropriate.
void ConnectFour::makeMove(char column, CellState p) {
    if (isalpha(column)) column = tolower(column);
    int col = column - 'a';
    if (col < 0 || col >= cols) {
        cout << "Invalid column: " << column << ". Choose a between 'a' and '" << char('a' + cols - 1) << "'.\n";
        return;
    }
    int row = findLowestEmpty(col);
    if (row == -1) {
        cout << "Column " << column << " is full or not playable.\n";
        return;
    }

    gameBoard[row][col].setState(p);
    if (checkWin(row, col, p)) {
        gameEnded = true;
        winner = p;
    } else if (isBoardFull()) {
        gameEnded = true;
        winner = CellState::EMPTY; // draw
    }
}

// Public wrappers for moves
void ConnectFour::play() {
    if (gameEnded) return;
    int c = computerMove();
    if (c == -1) {
        cout << "Computer has no valid moves. It's a draw.\n";
        gameEnded = true;
        winner = CellState::EMPTY;
        return;
    }
    cout << "Computer plays column " << char('a' + c) << "\n";
    makeMove(static_cast<char>('a' + c), CellState::COMPUTER);
}

void ConnectFour::play(char column) {
    makeMove(column, CellState::USER1);
}

bool ConnectFour::isGameEnded() const { return gameEnded; }

// ---------------------------- Printing ----------------------------
// We always print a full rectangle rows x cols.
// Non-playable positions are shown as '.' so the board looks regular.

void ConnectFour::printBoard() const {
    // Column header
    cout << " ";
    for (int c = 0; c < cols; ++c) cout << static_cast<char>('a' + c);
    cout << "\n";

    // Rows (top to bottom)
    for (int r = 0; r < rows; ++r) {
        cout << " ";
        for (int c = 0; c < cols; ++c) {
            int playableStart = rows - colHeights[c];
            if (playableStart < 0) playableStart = 0;
            if (r < playableStart) {
                // Non-playable area — display dot to keep rectangular shape.
                cout << ".";
            } else {
                CellState s = gameBoard[r][c].getState();
                switch (s) {
                    case CellState::EMPTY:    cout << '.'; break;
                    case CellState::USER1:    cout << 'X'; break;
                    case CellState::USER2:    cout << 'O'; break;
                    case CellState::COMPUTER: cout << 'C'; break;
                }
            }
        }
        cout << "\n";
    }
}

ostream& operator<<(ostream& os, const ConnectFour& g) {
    g.printBoard();
    return os;
}

// ---------------------------- Comparison ----------------------------
bool ConnectFour::operator==(const ConnectFour& o) const {
    if (rows != o.rows || cols != o.cols) return false;
    for (int c = 0; c < cols; ++c) if (colHeights[c] != o.colHeights[c]) return false;
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            if (!(gameBoard[r][c] == o.gameBoard[r][c])) return false;
    return true;
}
bool ConnectFour::operator!=(const ConnectFour& o) const { return !(*this == o); }

// ---------------------------- Persistence (no vector) ----------------------------
// Save all crucial state so we can restore it exactly later on

void ConnectFour::saveToFile(const string& fn) const {
    ofstream f(fn);
    if (!f.is_open()) {
        cout << "Cannot save to file: " << fn << "\n";
        return;
    }
    f << rows << " " << cols << "\n";
    for (int c = 0; c < cols; ++c) f << colHeights[c] << " ";
    f << "\n";
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) f << static_cast<int>(gameBoard[r][c].getState()) << " ";
        f << "\n";
    }
    f << (gameEnded ? 1 : 0) << "\n";
    f << static_cast<int>(winner) << "\n";
    cout << "Saved to " << fn << "\n";
}

// Load a saved play state (or shape file if created specially).
void ConnectFour::loadFromFile(const string& filename) {
    ifstream file(filename.c_str());
    if (!file.is_open()) {
        cout << "Error opening file: " << filename << ". Using default 5x5.\n";
        *this = ConnectFour();
        return;
    }

    // Count non-empty lines of the shape file.
    string temp;
    int lineCount = 0;
    while (getline(file, temp)) {
        if (!temp.empty()) ++lineCount;
    }

    if (lineCount == 0) {
        cout << "Empty shape file. Using default 5x5.\n";
        *this = ConnectFour();
        file.close();
        return;
    }

    // Rewind to read lines into dynamic storage.
    file.clear();
    file.seekg(0, ios::beg);

    char** lines = new char*[lineCount];
    int* lengths = new int[lineCount];
    int idx = 0;
    while (idx < lineCount && getline(file, temp)) {
        if (temp.empty()) continue;
        lengths[idx] = static_cast<int>(temp.length());
        lines[idx] = new char[lengths[idx]];
        for (int i = 0; i < lengths[idx]; ++i) lines[idx][i] = temp[i];
        ++idx;
    }
    file.close();

    // Determine max columns (the width we should use).
    int maxCols = 0;
    for (int i = 0; i < lineCount; ++i) if (lengths[i] > maxCols) maxCols = lengths[i];

    // Count '*' in each column to derive playable heights.
    int* heights = new int[maxCols];
    for (int c = 0; c < maxCols; ++c) heights[c] = 0;
    for (int c = 0; c < maxCols; ++c) {
        for (int r = 0; r < lineCount; ++r) {
            if (c < lengths[r] && lines[r][c] == '*') heights[c]++;
        }
    }

    // Compute the board rows as the maximum column height.
    int maxHeight = 0;
    for (int c = 0; c < maxCols; ++c) if (heights[c] > maxHeight) maxHeight = heights[c];

    // Reinitialize rectangle board
    deallocateBoard();
    delete[] colHeights;

    rows = (maxHeight > 0 ? maxHeight : 1);
    cols = (maxCols > 0 ? maxCols : 1);

    colHeights = new int[cols];
    for (int c = 0; c < cols; ++c) colHeights[c] = heights[c];

    initializeBoard();

    // Cleanup dynamic buffers used while parsing.
    for (int i = 0; i < lineCount; ++i) delete[] lines[i];
    delete[] lines;
    delete[] lengths;
    delete[] heights;

    cout << "Shape loaded: " << rows << "x" << cols << "\n";
}

// ---------------------------- Play loop  ----------------------------
// This function only runs the game loop. It does NOT ask for mode or shape.
// main() must already have set isVsComputer and optionally loaded a shape.
void ConnectFour::playGame() {
    if (gameEnded) {
        printBoard();
        cout << "This game is already finished.\n";
        return;
    }

    printBoard();

    bool vsComputer = isVsComputer;
    CellState current = CellState::USER1;

    while (!gameEnded) {
        if (vsComputer && current == CellState::COMPUTER) {
            cout << "\n--- COMPUTER (C) turn ---\n";
            play(); // PC makes its move
        } else {
            cout << "\n--- " 
                 << (current == CellState::USER1 ? "USER1 (X)" : "USER2 (O)") 
                 << " turn ---\n";
            cout << "Enter column (a-" << static_cast<char>('a' + cols - 1) << "): ";

            char col;
            cin >> col;

            if (cin.fail()) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }

            if (current == CellState::USER1)
                makeMove(col, CellState::USER1);
            else
                makeMove(col, CellState::USER2);
        }

        printBoard();

        if (gameEnded) break;

        // Switch turns
        if (vsComputer) {
            current = (current == CellState::USER1) ? CellState::COMPUTER : CellState::USER1;
        } else {
            current = (current == CellState::USER1) ? CellState::USER2 : CellState::USER1;
        }
    }

    cout << "\n=== GAME OVER ===\n";
    if (winner == CellState::USER1)      cout << "USER1 (X) wins!\n";
    else if (winner == CellState::USER2) cout << "USER2 (O) wins!\n";
    else if (winner == CellState::COMPUTER) cout << "COMPUTER (C) wins!\n";
    else cout << "It's a draw!\n";

    // ------------------------------------------------------------------
    // SAVE MENU — After the game ends, user may want to save the board.
    // ------------------------------------------------------------------
    char choice;
    cout << "\nWould you like to save this game? (y/n): ";
    cin >> choice;

    if (!cin.fail() && (choice == 'y' || choice == 'Y')) {
        cout << "Enter filename to save (for example: mygame.txt): ";
        string fname;
        cin >> fname;

        if (!cin.fail() && !fname.empty()) {
            saveToFile(fname);
        } else {
            cout << "Invalid filename. Save aborted.\n";
        }
    }
}


// ---------------------------- Mode setter/getter ----------------------------
void ConnectFour::setVsComputer(bool v) { isVsComputer = v; }
bool ConnectFour::getVsComputer() const { return isVsComputer; }
