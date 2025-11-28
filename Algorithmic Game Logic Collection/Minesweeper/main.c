#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAX_SIZE 10
#define MAX_MOVES 1000

int board_size;
int mine_board[MAX_SIZE][MAX_SIZE];
int revealed[MAX_SIZE][MAX_SIZE];
int game_over = 0;
int move_rows[MAX_MOVES];
int move_cols[MAX_MOVES];
int move_top = 0;
int push_count[MAX_MOVES];
int move_history_top = 0;

void push(int row, int col) {
    if (move_top < MAX_MOVES) {
        move_rows[move_top] = row;
        move_cols[move_top] = col;
        move_top++;
    }
}

void pop_group() {
    int i;
    if (move_history_top <= 0) {
        printf("No moves to undo!\n");
        return;
    }

    int count = push_count[--move_history_top];
    for (i = 0; i < count; i++) {
        if (move_top > 0) {
            move_top--;
            int r = move_rows[move_top];
            int c = move_cols[move_top];
            revealed[r][c] = 0;
        }
    }
    printf("Last move undone (%d cell%s).\n", count, count > 1 ? "s" : "");
}

int count_mines(int r, int c) {
    int count = 0;
    int dr, dc;
    for (dr = -1; dr <= 1; dr++) {
        for (dc = -1; dc <= 1; dc++) {
            int nr = r + dr;
            int nc = c + dc;
            if (nr >= 0 && nr < board_size && nc >= 0 && nc < board_size) {
                if (mine_board[nr][nc]) count++;
            }
        }
    }
    return count;
}

int flood_fill(int r, int c) {
    if (r < 0 || r >= board_size || c < 0 || c >= board_size || revealed[r][c]) return 0;

    revealed[r][c] = 1;
    push(r, c);
    int opened = 1;

    if (count_mines(r, c) == 0) {
        int dr, dc;
        for (dr = -1; dr <= 1; dr++) {
            for (dc = -1; dc <= 1; dc++) {
                if (dr != 0 || dc != 0)
                    opened += flood_fill(r + dr, c + dc);
            }
        }
    }

    return opened;
}

void print_board(int show_all) {
    int i, j;

    printf("\n    ");
    for (i = 0; i < board_size; i++) {
        printf("%2d ", i);
    }
    printf("\n");

    printf("   ");
    for (i = 0; i < board_size; i++) {
        printf("---");
    }
    printf("\n");

    for (i = 0; i < board_size; i++) {
        printf("%2d|", i);
        for (j = 0; j < board_size; j++) {
            if (revealed[i][j] || show_all) {
                if (mine_board[i][j]) {
                    printf(" * ");
                } else {
                    int mines = count_mines(i, j);
                    printf(" %d ", mines);
                }
            } else {
                printf(" # ");
            }
        }
        printf("\n");
    }
}

void save_map() {
    FILE *fp = fopen("map.txt", "w");
    int i, j;
    for (i = 0; i < board_size; i++) {
        for (j = 0; j < board_size; j++) {
            if (mine_board[i][j]) {
                fprintf(fp, "* ");
            } else {
                fprintf(fp, ". ");
            }
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void save_moves() {
    FILE *fp = fopen("moves.txt", "w");
    fprintf(fp, "--- Game Moves ---\n");
    int move_number = 1;
    int index = 0;
    int i, j;
    for (i = 0; i < move_history_top; i++) {
        fprintf(fp, "Move %d:\n", move_number++);
        for (j = 0; j < push_count[i]; j++) {
            fprintf(fp, "    (Row %d, Col %d)\n", move_rows[index], move_cols[index]);
            index++;
        }
    }
    fprintf(fp, "Total Moves: %d\n", move_history_top);
    fclose(fp);
}

void generate_board() {
    srand(time(NULL));
    board_size = 2 + rand() % 9;
    int num_mines = board_size;
    int placed = 0;
    while (placed < num_mines) {
        int r = rand() % board_size;
        int c = rand() % board_size;
        if (!mine_board[r][c]) {
            mine_board[r][c] = 1;
            placed++;
        }
    }
    save_map();
}

void gameOver(int row, int col) {
    game_over = 1;
    revealed[row][col] = 1;
    printf("\nBOOM! You hit a mine. Game Over.\n");
    print_board(0);
}

int main() {
    char input[20];
    int row, col;

    generate_board();

    while (!game_over) {
        print_board(0);
        printf("\nEnter move (row col) or 'undo': ");
        scanf("%s", input);

        if (strcmp(input, "undo") == 0) {
            pop_group();
        } else {
            row = atoi(input);
            scanf("%d", &col);

            if (row < 0 || row >= board_size || col < 0 || col >= board_size) {
                printf("Invalid move. Try again.\n");
                continue;
            }
            if (revealed[row][col]) {
                printf("Already revealed. Try again.\n");
                continue;
            }

            if (mine_board[row][col]) {
                push(row, col); 
                push_count[move_history_top++] = 1;
                gameOver(row, col);
            } else {
                int opened = flood_fill(row, col);
                push_count[move_history_top++] = opened;
            }
        }
    }

    save_moves();
    return 0;
}
