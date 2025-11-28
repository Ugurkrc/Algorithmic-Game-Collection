#ifndef CONNECTFOUR_H
#define CONNECTFOUR_H

#include <iostream>
#include <string>

class ConnectFour {
public:
    // Use scoped enum for safety and clarity.
    enum class CellState { EMPTY = 0, USER1 = 1, USER2 = 2, COMPUTER = 3 };

    // Inner Cell class: represents one board cell.
    class Cell {
    public:
        Cell();
        Cell(char c, int r, CellState s = CellState::EMPTY);

        char getColumn() const;
        int getRow() const;
        CellState getState() const;
        void setState(CellState s);

        bool operator==(const Cell& other) const;

        // ++ and -- overloads (pre/post)
        Cell& operator++();
        Cell operator++(int);
        Cell& operator--();
        Cell operator--(int);

        friend std::ostream& operator<<(std::ostream& os, const Cell& c);
        friend std::istream& operator>>(std::istream& is, Cell& c);

    private:
        char column;
        int row;
        CellState state;
    };

    // Constructors / destructor / assignment
    ConnectFour();                         // default 5x5
    ConnectFour(int r, int c);             // param size
    ConnectFour(const std::string& fn);    // shape file
    ConnectFour(const ConnectFour& other); // deep copy
    ConnectFour& operator=(const ConnectFour& other);
    ~ConnectFour();

    // Core gameplay
    void play();                // computer move
    void play(char column);     // user1 move
    void playGame();            // only the play loop (does NOT ask mode/shape)

    bool isGameEnded() const;
    void printBoard() const;

    // Persistence
    void saveToFile(const std::string& fn) const;
    void loadFromFile(const std::string& fn); // vektörsüz!
    
    // Mode setter/getter (main will set this)
    void setVsComputer(bool v);
    bool getVsComputer() const;

    // Comparison and stream
    bool operator==(const ConnectFour& other) const;
    bool operator!=(const ConnectFour& other) const;
    friend std::ostream& operator<<(std::ostream& os, const ConnectFour& g);

private:
    int rows;
    int cols;
    Cell** gameBoard;   // dynamic 2D array
    int* colHeights;    // playable height per column

    bool gameEnded;
    CellState winner;

    bool isVsComputer;  // true => player vs computer, false => player vs player

    // Helpers
    void initializeBoard();
    void deallocateBoard();
    int findLowestEmpty(int col) const;
    bool isBoardFull() const;
    bool checkDirection(int r, int c, int dr, int dc, CellState p) const;
    bool checkWin(int r, int c, CellState p) const;
    int computerMove();
    void makeMove(char column, CellState p);
};

#endif // CONNECTFOUR_H
