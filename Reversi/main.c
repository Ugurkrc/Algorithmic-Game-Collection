#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

typedef enum {
    EMPTY = 0,
    COMPUTER = 1,  /* X for computer */
    USER = 2       /* O for user */
} CellState;
//Player
typedef struct {
    int row;
    int col;
} Position;
//Computer
typedef struct {
    Position pos;
    int score;
} Move;

CellState** allocateBoard(int size);
void freeBoard(CellState** board, int size);
void initializeBoard(CellState** board, int size);
void displayBoard(CellState** board, int size);
bool isValidMove(CellState** board, int size, Position pos, CellState player);
int makeMove(CellState** board, int size, Position pos, CellState player);
Position getUserMove(CellState** board, int size);
Position getComputerMove(CellState** board, int size);
bool hasValidMoves(CellState** board, int size, CellState player);
void countCells(CellState** board, int size, int* computerCount, int* userCount);
void printGameResult(int computerCount, int userCount);
int getBoardSize(void);
Position parseMove(const char* input);
bool isValidPosition(int size, Position pos);
int countFlips(CellState** board, int size, Position pos, CellState player);
void flipCells(CellState** board, int size, Position pos, CellState player);
int checkDirection(CellState** board, int size, Position pos, CellState player, int dRow, int dCol, bool flip);


//All possible moves in an array.
const int directions[8][2] = {{-1,-1}, {-1,0}, {-1,1}, {0,-1}, {0,1}, {1,-1}, {1,0}, {1,1}};

int main() {
    int size;
    CellState** board;
    CellState currentPlayer;
    bool gameRunning;
    int computerCount, userCount;
    
    printf("Welcome to Reversi!\n\n");
    
    /* Get the board size.*/
    size = getBoardSize();
    
    /* Create board warn if cannot.*/
    board = allocateBoard(size);
    if (!board) {
        printf("Memory allocation failed!\n");
        return 1;
    }
    
    /* The game reversi always starts with XO  layup in the beginng (it's more like black white white black) so the board is initialized with that.
                                           OX  */
    initializeBoard(board, size);
    
    /* The variable currentPlayer keeps track of the player who moves at the moment. Game always start with the user */
    currentPlayer = USER; /* User starts */
    gameRunning = true;
    
    /* Where the game actually runs. */
    while (gameRunning) {
        Position userPos, computerPos;
        int flipped;
        
        /* Displays the current board by printing the array in the dynamic memory.*/
        displayBoard(board, size);
        
        if (currentPlayer == USER) {
            /* If the player is making the move the program checks if the move is valid for example, it checks if the move is out of bounds etc. */
            if (hasValidMoves(board, size, USER)) {
                printf("\nYour turn (O):\n");
                userPos = getUserMove(board, size);
                flipped = makeMove(board, size, userPos, USER); /* The program calculates the amount of cells captured.*/
                printf("You captured %d cell(s).\n", flipped);
                currentPlayer = COMPUTER; /* Computer gets the opportunity.*/
            } else {
                printf("You have no valid moves. Skipping your turn.\n"); /* To not get stuck in an unending board the program checks whether the player has a valid move or not and moves on automatically if not.*/
                if (!hasValidMoves(board, size, COMPUTER)) {
                    gameRunning = false; /* If both players have no valid moves, the game ends. */
                    printf("Both players have no valid moves. Game over.\n");
                } else {
                    currentPlayer = COMPUTER;
                }
            }
        } else {
            /* Computer's turn */ /* Same algorithm as mentioned above */
            if (hasValidMoves(board, size, COMPUTER)) {
                printf("\nComputer's turn (X):\n");
                computerPos = getComputerMove(board, size);
                flipped = makeMove(board, size, computerPos, COMPUTER);
                printf("Computer plays %c%d and captures %d cell(s).\n", 
                       'a' + computerPos.col, computerPos.row + 1, flipped);
                       displayBoard(board, size); /* Displays the board after the computer's move. */
                currentPlayer = USER;
            } else {
                printf("Computer has no valid moves. Skipping computer's turn.\n");
                if (!hasValidMoves(board, size, USER)) {
                    gameRunning = false;
                } else {
                    currentPlayer = USER;
                }
            }
        }
        
        /* Game menu.*/
        printf("\nPress Enter to continue...");
        while (getchar() != '\n');
    }
    
    /* The final board is shown.*/
    displayBoard(board, size);
    
    /* Scores get counted.*/
    countCells(board, size, &computerCount, &userCount);
    printGameResult(computerCount, userCount);
    
    /* Release the allocated memory.*/
    freeBoard(board, size);
    
    return 0;
}

/* Creates the board dynamically. */
CellState** allocateBoard(int size) {
    CellState** board;
    int i, j;
    
    /* This is the main board.*/
    board = (CellState**)malloc(size * sizeof(CellState*));
    if (!board) {
        return NULL;
    }
    
    for (i = 0; i < size; i++) {
        board[i] = (CellState*)calloc(size, sizeof(CellState)); 
        if (!board[i]) {
            /* If not functioning properly the program frees the memory and ends the game..*/
            for (j = 0; j < i; j++) {
                free(board[j]);
            }
            /* The board gets cleaned so the game can start from scratch.*/
            free(board);
            return NULL;
        }
    }
    return board;
}

/* The function to free the memory allocated for the board. */
void freeBoard(CellState** board, int size) {
    int i;
    
    if (!board) {
        return;
    }
    
    /* lines get freed one by one. */
    for (i = 0; i < size; i++) {
        free(board[i]);
    }
    free(board);
}

/* As I mentioned before we have to start the board in a spesific way because of the rules of the game reversi. We do that here. */
void initializeBoard(CellState** board, int size) {
    int i, j;
    int center;
    
    /* Empty all the cells*/
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            board[i][j] = EMPTY;
        }
    }
    
    center = size / 2;
    
    /* We place the necessary cells to start the game. */
    board[center - 1][center - 1] = COMPUTER; /* An X for the left top corner of the center. */
    board[center - 1][center] = USER;         /* An O for the right top corner of the center. */
    board[center][center - 1] = USER;         /* An X for the left bottom corner of the center. */
    board[center][center] = COMPUTER;         /* An O for the right bottom corner of the center.X */
}

/* The function to print the board. */
void displayBoard(CellState** board, int size) {
    int i, j;
    char cell;
    
    printf("\n  ");
    
    for (j = 0; j < size; j++) {
        printf("%c", 'a' + j); 
    }
    printf("\n");
    
    /* The actual loop to print the cells one by one. */
    for (i = 0; i < size; i++) {
        printf("%d ", i + 1); /* Keep track of the rows. */
        for (j = 0; j < size; j++) {
            /* Cell info is checked and placed accordingly. */
            if (board[i][j] == EMPTY) {
                cell = '.';
            } else if (board[i][j] == COMPUTER) {
                cell = 'X';
            } else {
                cell = 'O';
            }
            printf("%c", cell);
        }
        printf("\n");
    }
}

int getBoardSize(void) {
    int size;
    
    while (1) { /* Get the board info from the user and check whether it fits the rules or not. */
        printf("Enter board size (4-20, must be an even number!!!!!): "); /* If the number is too big or too small the program throws a warning,
         clears the buffer and gets a new info until it's fit.*/
        if (scanf("%d", &size) != 1) { 
            printf("Invalid input. Please enter a number.\n");
            /* Buffer clearing. */
            while (getchar() != '\n');
            continue;
        }
        
        /* The board must be between 4-20 and must be even. First check if it is between 4 and 20 and chech whether it's even or not.*/
        if (size < 4 || size > 20) {
            printf("Board size must be between 4 and 20.\n");
            continue;
        }
        
        /* Even control. */
        if (size % 2 != 0) {
            printf("Board size must be even.\n");
            continue;
        }
        
        break; /* Breaks the loop if the input is fine. */
    }
    
    while (getchar() != '\n');
    return size;
}

/* The user input gets turned into an actual position on the board. */
Position parseMove(const char* input) {
    Position pos;
    char col;
    int row;
    
    /* Default position in case something is wrong. */
    pos.row = -1;
    pos.col = -1;
    
    /* We check if the input length is appropriate, returns the default position if not.*/
    if (strlen(input) < 2) {
        return pos;
    }
    
    col = tolower(input[0]); /* lower the column info. Because it is case sensetive. */
    if (col < 'a' || col > 't') { /* Since the board is 20x20 at max, can't have any column further than 't'. */
        return pos;/* Returns default if something is wrong. */
    }
    
    row = atoi(&input[1]) - 1; 
    if (row < 0) {
        return pos; /* Returns default if something is wrong.*/
    }
    
    /* Actually creates a valid location. */
    pos.col = col - 'a'; /* a becomes 0 b becomes 1.....*/
    pos.row = row;
    
    return pos; /* Returns a healthy position value.*/
}

/* Border control. */
bool isValidPosition(int size, Position pos) {
    if (pos.row >= 0 && pos.row < size && pos.col >= 0 && pos.col < size) {
        return true; /* If the given position is between the actual borders return true (1)*/
    } else {
        return false; /* Else (0). */
    }
}

/* get the user move. */
Position getUserMove(CellState** board, int size) {
    char input[10];
    Position pos;
    
    while (1) {
        printf("Enter your move (e.g., d3): ");
        if (scanf("%9s", input) != 1) { 
            continue;
        }
        
        /* Parse it into an actual move. */
        pos = parseMove(input);
        
        /* Checks if the parsed move is valid or not. */
        if (!isValidPosition(size, pos)) {
            printf("Invalid position. Use format like 'd3'.\n");
            continue;
        }
        
        /* If the move is valid then checks if the desired cell is empty or not. */
        if (board[pos.row][pos.col] != EMPTY) {
            printf("Position already occupied.\n");
            continue;
        }
        
        /* Checks if the move is appropriate or not. */
        if (!isValidMove(board, size, pos, USER)) {
            printf("Invalid move. You must capture at least one opponent piece.\n");
            continue;
        }
        
        break; /* Breaks if everything is fine. */
    }
    
    return pos;
}

/* The function that checks whether the move is valid or not, just mentioned before. */
bool isValidMove(CellState** board, int size, Position pos, CellState player) {
    /* Is the position available? */
    if (!isValidPosition(size, pos)) {
        return false;
    }/* Is the cell empty? */
    if (board[pos.row][pos.col] != EMPTY) {
        return false;
    }
    
    /* Can you flip at least one stone? */
    if (countFlips(board, size, pos, player) > 0) {
        return true;
    } else {
        return false;
    }
}

/* Where we actually keep count of the cells flipped by a move. */
int countFlips(CellState** board, int size, Position pos, CellState player) {
    int totalFlips;
    int i;
    
    totalFlips = 0;
    
    /* Checks every single direction by a for loops, and increases the flip count if it fits. */
    for (i = 0; i < 8; i++) {
        totalFlips += checkDirection(board, size, pos, player, 
                                   directions[i][0], directions[i][1], false);
    }
    
    return totalFlips;
}

int checkDirection(CellState** board, int size, Position pos, CellState player, 
                  int dRow, int dCol, bool flip) {
    CellState opponent;
    int flips;
    int row, col;
    int i;
    
    /* Decide who is the opponent. */
    if (player == USER) {
        opponent = COMPUTER;
    } else {
        opponent = USER;
    }
    
    flips = 0;
    row = pos.row + dRow;
    col = pos.col + dCol;
    
    /* Checks the direction (line by line) and increases the flip count if there is a enemy cell. Also incrases the position so we do not get stuck in a infinite loop. */
    while (row >= 0 && row < size && col >= 0 && col < size && 
           board[row][col] == opponent) {
        flips++;
        row += dRow;
        col += dCol;
    }
    
    /* If the move is out of bounds it is not accepted. */
    if (row < 0 || row >= size || col < 0 || col >= size || 
        board[row][col] != player) {
        return 0;
    }
    
    /* But if the flip flag is 1 and you can actually flip some cells then the program actually does the considered changes on the board. */
    if (flip && flips > 0) {
        row = pos.row + dRow;
        col = pos.col + dCol;
        for (i = 0; i < flips; i++) {
            board[row][col] = player;
            row += dRow;
            col += dCol;
        }
    }
    
    return flips;
}

/* Makes the moves happen. */
int makeMove(CellState** board, int size, Position pos, CellState player) {
    int totalFlipped;
    int i;
    
    totalFlipped = 0;
    
    /* This function actually places the player cell. */
    board[pos.row][pos.col] = player;
    
    /* We use the same function before to reach all 8 directions and flip the necessary ones. */
    for (i = 0; i < 8; i++) {
        totalFlipped += checkDirection(board, size, pos, player, directions[i][0], directions[i][1], true);
                                     
    }
    
    return totalFlipped;
}

/*(ALWAYS CHOOSES THE MOVE WITH THE BIGGEST NUMBER OF FLIPS.)  */
Position getComputerMove(CellState** board, int size) {
    Move bestMove;
    int i, j;
    Position pos;
    int score;
    
    /* assign a default move. */
    bestMove.pos.row = -1;
    bestMove.pos.col = -1;
    bestMove.score = -1;
    
    /* Tries everysingle move with a loop. */
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            pos.row = i;
            pos.col = j;
            
            /* If it's valid. */
            if (isValidMove(board, size, pos, COMPUTER)) {
                /* Compute the amount of flips. */
                score = countFlips(board, size, pos, COMPUTER);
                
                /* If it's better than the previous than this is the best move. */
                if (score > bestMove.score) {
                    bestMove.pos = pos;
                    bestMove.score = score;
                }
            }
        }
    }
    
    return bestMove.pos;
}

/* Checks if the computer has a valid move or not. */
bool hasValidMoves(CellState** board, int size, CellState player) {
    int i, j;
    Position pos;
    
    /* try everysingle move once again. */
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            pos.row = i;
            pos.col = j;
            
            /* If there is even one single valid move, return 1;  */
            if (isValidMove(board, size, pos, player)) {
                return true;
            }
        }
    }
    
    /* 0 if not. */
    return false;
}

/* Counts the amount of occupied cells. */
void countCells(CellState** board, int size, int* computerCount, int* userCount) {
    int i, j;
    
    /* reset the counters before starting. */
    *computerCount = 0;
    *userCount = 0;
    
    /* Checks every cell. */
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            if (board[i][j] == COMPUTER) {
                (*computerCount)++; /* Increases computer if it's a 1.*/
            } else if (board[i][j] == USER) {
                (*userCount)++; /* Increases player if it's a 0. */
            }
        }
    }
}

/* Prints the result info. */
void printGameResult(int computerCount, int userCount) {
    printf("\n=== GAME OVER ===\n");
    printf("Final Score:\n");
    printf("Computer (X): %d\n", computerCount);
    printf("User (O): %d\n", userCount);
    /* Prints the scores. (flip founts.)*/
    
    /* The bigger one wins. */
    if (computerCount > userCount) {
        printf("\nComputer wins!\n");
    } else if (userCount > computerCount) {
        printf("\nYou win! Congratulations!\n");
    } else {
        printf("\nIt's a tie!\n"); /* Tıe if they are equal.*/
    }
}
