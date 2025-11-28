#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/* I used enumaration keep the user and computer info of the board, we can see why this is useful later on.*/
typedef enum {
    EMPTY = 0,
    COMPUTER = 1,  /* X for computer */
    USER = 2       /* O for user */
} CellState;

/* Every move has a row and column (basically x and y) coordinates so I kept them in another struct.*/
typedef struct {
    int row;
    int col;
} Position;

/* This struct only keeps track of the computers move it has the same coordinates as the user but also  a score tab.*/
typedef struct {
    Position pos;
    int score;
} Move;

/* I had to keep track of my function prototypes in order to not lose my mind.*/
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


/* Since we can move 8 different sides and checking them one by one was such a pain, I just used a 2D array to check them in a for loop. */
const int directions[8][2] = {{-1,-1}, {-1,0}, {-1,1}, {0,-1}, {0,1}, {1,-1}, {1,0}, {1,1}};

/* I will be explaining the general idea of my code. */
int main() {
    int size;
    CellState** board;
    CellState currentPlayer;
    bool gameRunning;
    int computerCount, userCount;
    
    printf("Welcome to Reversi!\n\n");
    
    /* The program gets the board size with the proper rules.*/
    size = getBoardSize();
    
    /* Program creates the board pops a warning if it was unable to do so.*/
    board = allocateBoard(size);
    if (!board) {
        printf("Memory allocation failed!\n");
        return 1;
    }
    
    /* The game reversi always starts with XO  layup in the beginng (it's more like black white white black) so we initialize the board with that.
                                           OX  */
    initializeBoard(board, size);
    
    /* The variable currentPlayer keeps track of the player who moves at the moment. I wanted to give the first move to the player everytime to observe the code better, 
    but I could also give it to the computer or even make it random */
    currentPlayer = USER; /* User starts */
    gameRunning = true;
    
    /* Where the game actually runs. */
    while (gameRunning) {
        Position userPos, computerPos;
        int flipped;
        
        /* Displays the current board by basically printing the array in the dynamic memory.*/
        displayBoard(board, size);
        
        if (currentPlayer == USER) {
            /* If the player is making the move the program checks if the move is valid for example, it checks if the move is out of bounds etc. */
            if (hasValidMoves(board, size, USER)) {
                printf("\nYour turn (O):\n");
                userPos = getUserMove(board, size);
                flipped = makeMove(board, size, userPos, USER); /* This part is really important the program calculates the amount of cells captured. 
                I will give extra info while explaining the actual which does this. */
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
            /* Computer's turn */ /* Same algorithm as I mentioned above */
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
        
        /* A menu type of thing.*/
        printf("\nPress Enter to continue...");
        while (getchar() != '\n');
    }
    
    /* The part where the final board is shown.*/
    displayBoard(board, size);
    
    /* The part where the scores are counted.*/
    countCells(board, size, &computerCount, &userCount);
    printGameResult(computerCount, userCount);
    
    /* Freeing the memory which was alloacted earlier.*/
    freeBoard(board, size);
    
    return 0;
}

/* The function that I created the dynamic board. */
CellState** allocateBoard(int size) { /* Since I return a cellstate pointer the return type is also a cellstate pointer.*/
    CellState** board;
    int i, j;
    
    /* This is the main board.*/
    board = (CellState**)malloc(size * sizeof(CellState*));
    if (!board) {
        return NULL;
    }
    
    /* This is the part where I allocate memory for each line by using calloc function. */
    for (i = 0; i < size; i++) {
        board[i] = (CellState*)calloc(size, sizeof(CellState)); /* I mentioned the calloc usage earlier and the fact that it is important now I will explain why.*/
        /* By using calloc we fill each line with 0's by doing that we keep them free. So no need to clear them manually.*/
        if (!board[i]) {
            /* if something crazy happens we the lines before that gets cleared.*/
            for (j = 0; j < i; j++) {
                free(board[j]);
            }
            /* Also the board gets cleaned so we can start from scratch.*/
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
    
    /* We free the lines one by one by using a for loop. */
    for (i = 0; i < size; i++) {
        free(board[i]);
    }
    /* Then the pointer which points to that char array.*/
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
    
    /* The program finds the center by dividing the size by two because the recersi board is a square. 
    We need to find the center because we start the game after we make the special start position of the game. */
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
    
    /* We print the line of letters (the top part) */
    for (j = 0; j < size; j++) {
        printf("%c", 'a' + j); /* Printing them by ASCII numbers was faster so... */
    }
    printf("\n");
    
    /* The actual loop to print the lines (morelike cells) one by one. */
    for (i = 0; i < size; i++) {
        printf("%d ", i + 1); /* To keep track of the rows. */
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

/* This is pretty self-explanatory. */
int getBoardSize(void) {
    int size;
    
    while (1) { /* We get the board info from the user and check whether it fits the rules or not. */
        printf("Enter board size (4-20, must be an even number!!!!!): "); /* If the number is too big or too small the program pops a warning,
         clears the buffer and gets a new info until it's fit.(CHECK A COUPLE OF LINES BELOW.)*/
        if (scanf("%d", &size) != 1) { /* If the scanf does not return 1 which means there was something wrong, probably some other character or string was entered we force the player to enter an actual integer.*/
            printf("Invalid input. Please enter a number.\n");
            /* Buffer clearing. */
            while (getchar() != '\n');
            continue;
        }
        
        /* As mentioned  the board must be between 4-20 and must be even. First we check if it is between 4 and 20 and chech whether it's even or not.*/
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
    
    /* I faced a couple of issues while getting info so we clear the input buffer once again right after getting the input. */
    while (getchar() != '\n');
    return size;
}

/* The user input gets turned into an actual position on the board. */
Position parseMove(const char* input) {
    Position pos;
    char col;
    int row;
    
    /* This is the default position in case something is wrong. */
    pos.row = -1;
    pos.col = -1;
    
    /* We check if the input length is appropriate, returns the default position if not.*/
    if (strlen(input) < 2) {
        return pos;
    }
    
    /* We get the column info with letters and rows by numbers. */
    col = tolower(input[0]); /* So we lower the column info. Because it is case sensetive. */
    if (col < 'a' || col > 't') { /* Since the board is 20x20 at max we can't have any column further than 't'. */
        return pos;/* Returns default if something is wrong. */
    }
    
    /* Satır numarasını al ve kontrol et */
    row = atoi(&input[1]) - 1; /* I used atoi to get the integer of the row because even though it is a number logically it is in string format. So we change that and put that in to our input array. */
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
        return true; /* If the given position is between the actual borders we return true (1)*/
    } else {
        return false; /* Else (0). */
    }
}

/* We get the user move. */
Position getUserMove(CellState** board, int size) {
    char input[10];
    Position pos;
    
    while (1) {
        printf("Enter your move (e.g., d3): ");
        if (scanf("%9s", input) != 1) { /* This is what I mentioned earlier, we get the user input in string format in order to use this in integer format we have to use atoi.*/
            printf("Invalid input.\n");
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
        
        /* Checks if the move is reversi appropriate or not. */
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

/* This is how we check every direction. */
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

/* This is one of the most important functions of the program because this function is literally the computers decide-maker. But since I do not know much about the game reversi or how the actual AI's function
 in a game I made the computer as greedy as possible (ALWAYS CHOOSES THE MOVE WITH THE BIGGEST NUMBER OF FLIPS.)  */
Position getComputerMove(CellState** board, int size) {
    Move bestMove;
    int i, j;
    Position pos;
    int score;
    
    /* We assign a default move. */
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
    
    /* We try everysingle move once again. */
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
    
    /* Do not forget to reset the counters before starting. */
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