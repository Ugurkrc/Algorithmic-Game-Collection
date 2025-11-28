#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#define SIZE 10

char board[SIZE][SIZE];
char display[SIZE][SIZE];
int ship_sizes[4] = {2, 3, 3, 4};
int ship_hits[4] = {0};
int ship_start_row[4];
int ship_start_col[4];
int ship_is_vertical[4];
int ships_remaining = 4;
int total_shots = 0;

void boardSetup() {
    int i, j;
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            board[i][j] = '-';
            display[i][j] = '-';
        }
    }
}

void saveShips() {
    FILE *file = fopen("ship_locations.txt", "w");
    int i;
    if (file == NULL) {
        printf("Error opening ship locations file!\n");
        return;
    }
    fprintf(file, "Ship Locations:\n");
    for (i = 0; i < 4; i++) {
        fprintf(file, "Ship %d: Size %d at (%d,%d) %s\n", 
                i+1, ship_sizes[i], 
                ship_start_row[i], ship_start_col[i],
                ship_is_vertical[i] ? "Vertical" : "Horizontal");
    }
    fclose(file);
}

void keepLog(int row, int col, int hit, int sunk) {
    FILE *file = fopen("battleship.log.txt", "a");
    if (file == NULL) {
        printf("Error opening log file!\n");
        return;
    }
    fprintf(file, "Shot %d: (%d,%d) - %s", total_shots, row, col, hit ? "HIT" : "MISS");
    if (sunk) {
        fprintf(file, " - SUNK A SHIP!");
    }
    fprintf(file, "\n");
    fclose(file);
}

int checkForShip(int row, int col, int size, int is_vertical) {
    int i, j;
    if (is_vertical) {
        if (row + size > SIZE) return 0;
        for (i = row; i < row + size; i++) {
            if (board[i][col] != '-') 
            return 0;
        }
    } else {
        if (col + size > SIZE) return 0;
        for (j = col; j < col + size; j++) {
            if (board[row][j] != '-') 
            return 0;
        }
    }
    return 1;
}

void placeShip(int row, int col, int size, int is_vertical, int ship_index) {
    int i, j;
    ship_sizes[ship_index] = size;
    ship_hits[ship_index] = 0;
    ship_start_row[ship_index] = row;
    ship_start_col[ship_index] = col;
    ship_is_vertical[ship_index] = is_vertical;
    if (is_vertical) {
        for (i = row; i < row + size; i++) {
            board[i][col] = 'S';
        }
    } else {
        for (j = col; j < col + size; j++) {
            board[row][j] = 'S';
        }
    }
}

void randomShip() {
    int i, row, col, is_vertical, placed;
    srand(time(NULL));
    for (i = 0; i < 4; i++) {
        placed = 0;
        while (!placed) {
            row = rand() % SIZE;
            col = rand() % SIZE;
            is_vertical = rand() % 2;
            if (checkForShip(row, col, ship_sizes[i], is_vertical)) {
                placeShip(row, col, ship_sizes[i], is_vertical, i);
                placed = 1;
            }
        }
    }
}

void printBoard() {
    int i, j;
    printf("\n  ");
    for (i = 0; i < SIZE; i++) {
        printf("%d ", i);
    }
    printf("\n");
    for (i = 0; i < SIZE; i++) {
        printf("%d ", i);
        for (j = 0; j < SIZE; j++) {
            printf("%c ", display[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

int checkSunkShip(int ship_index) {
    if (ship_hits[ship_index] == ship_sizes[ship_index]) {
        return 1;
    }
    return 0;
}

void markSunkShip(int ship_index) {
    int row = ship_start_row[ship_index];
    int col = ship_start_col[ship_index];
    int size = ship_sizes[ship_index];
    int is_vertical = ship_is_vertical[ship_index];
    int i, j;
    if (is_vertical) {
        for (i = row; i < row + size; i++) {
            display[i][col] = 'X';
        }
    } else {
        for (j = col; j < col + size; j++) {
            display[row][j] = 'X';
        }
    }
}

int FIRE(int row, int col) {
    int i;
    total_shots++;
    if (board[row][col] == 'S') {
        display[row][col] = 'X';
        for (i = 0; i < 4; i++) {
            if (ship_is_vertical[i]) {
                if (col == ship_start_col[i] && 
                    row >= ship_start_row[i] && 
                    row < ship_start_row[i] + ship_sizes[i]) {
                    ship_hits[i]++;
                    if (checkSunkShip(i)) {
                        markSunkShip(i);
                        ships_remaining--;
                        printf("Congratulations! You have sunk a size %d ship!\n", ship_sizes[i]);
                        keepLog(row, col, 1, 1);
                        return 2;
                    }
                }
            } else {
                if (row == ship_start_row[i] && 
                    col >= ship_start_col[i] && 
                    col < ship_start_col[i] + ship_sizes[i]) {
                    ship_hits[i]++;
                    if (checkSunkShip(i)) {
                        markSunkShip(i);
                        ships_remaining--;
                        printf("Congratulations! You have sunk a size %d ship!\n", ship_sizes[i]);
                        keepLog(row, col, 1, 1);
                        return 2;
                    }
                }
            }
        }
        printf("Hit!\n");
        keepLog(row, col, 1, 0);
        return 1;
    } else {
        display[row][col] = 'O';
        printf("Miss!\n");
        keepLog(row, col, 0, 0);
        return 0;
    }
}

void newGame() {
    int i;
    boardSetup();
    randomShip();
    saveShips();
    for (i = 0; i < SIZE; i++) {
        int j;
        for (j = 0; j < SIZE; j++) {
            display[i][j] = '-';
        }
    }
    ships_remaining = 4;
    total_shots = 0;
    for (i = 0; i < 4; i++) {
        ship_hits[i] = 0;
    }
    FILE *file = fopen("battleship.log.txt", "w");
    if (file != NULL) {
        fclose(file);
    }
}

int main() {
    char input[10];
    int i, row, col;
    printf("Welcome to Battleship!\n");
    printf("Enter coordinates as row,col (e.g., 3,5)\n");
    printf("Enter X to exit the game\n\n");
    
    newGame();
    
    while (1) {
        printBoard();
        
        if (ships_remaining == 0) {
            printf("Congratulations! You've sunk all ships in %d shots!\n", total_shots);
            printf("Press 'N' to start a new game or any other key to exit: ");
            
            fgets(input, sizeof(input), stdin);
            for (i = 0; input[i]; i++) {
                input[i] = toupper(input[i]);
            }
            
            if (strcmp(input, "N\n") == 0) {
                newGame();
                continue;
            } else {
                printf("Game ended. Total shots fired: %d\n", total_shots);
                break;
            }
        }
        
        printf("Enter target: ");
        fgets(input, sizeof(input), stdin);
        for (i = 0; input[i]; i++) {
            input[i] = toupper(input[i]);
        }
        
        if (strcmp(input, "X\n") == 0) {
            printf("Game ended. Total shots fired: %d\n", total_shots);
            break;
        }
        
        if (sscanf(input, "%d,%d", &row, &col) == 2) {
            if (row >= 0 && row < SIZE && col >= 0 && col < SIZE) {
                if (display[row][col] == '-') {
                    FIRE(row, col);
                } else {
                    printf("You've already targeted this location.\n");
                }
            } else {
                printf("Invalid coordinates. Please enter values between 0 and 9.\n");
            }
        } else {
            printf("Invalid input. Please enter coordinates as row,col (e.g., 3,5) or X to exit\n");
        }
    }
    return 0;
}