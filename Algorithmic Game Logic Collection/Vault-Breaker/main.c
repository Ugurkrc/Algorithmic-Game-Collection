#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int* generate_code();
int* get_guess();

int main() {
    char mode;
    int i, j;

    srand(time(NULL));

    printf("Vault Codebreaker Game\n");
    printf("Enter 'A' for Admin Mode or 'P' for Player Mode: ");
    scanf(" %c", &mode);

    if (mode == 'A' || mode == 'a') {
        int code_length, digit_min, digit_max, max_attempts;
        int allow_duplicates, points_correct, points_misplaced, penalty_wrong;

        printf("Enter code length: ");
        scanf("%d", &code_length);
        printf("Enter digit min: ");
        scanf("%d", &digit_min);
        printf("Enter digit max: ");
        scanf("%d", &digit_max);
        printf("Allow duplicates? (1 = Yes, 0 = No): ");
        scanf("%d", &allow_duplicates);
        printf("Enter max attempts: ");
        scanf("%d", &max_attempts);
        printf("Points for correct digit in correct place: ");
        scanf("%d", &points_correct);
        printf("Points for correct digit in wrong place: ");
        scanf("%d", &points_misplaced);
        printf("Penalty for wrong digit: ");
        scanf("%d", &penalty_wrong);

        FILE* config = fopen("vault_config.txt", "w");
        if (config == NULL) {
            printf("Failed to write config file.\n");
            return 1;
        }

        fprintf(config, "CODE_LENGTH=%d\n", code_length);
        fprintf(config, "DIGIT_MIN=%d\n", digit_min);
        fprintf(config, "DIGIT_MAX=%d\n", digit_max);
        fprintf(config, "MAX_ATTEMPTS=%d\n", max_attempts);
        fprintf(config, "ALLOW_DUPLICATES=%d\n", allow_duplicates);
        fprintf(config, "POINTS_CORRECT=%d\n", points_correct);
        fprintf(config, "POINTS_MISPLACED=%d\n", points_misplaced);
        fprintf(config, "PENALTY_WRONG=%d\n", penalty_wrong);

        fclose(config);
        printf("Configuration saved.\n");

    } else if (mode == 'P' || mode == 'p') {
        int code_length, digit_min, digit_max, max_attempts;
        int allow_duplicates, points_correct, points_misplaced, penalty_wrong;
        int* code;
        int* guess;
        int* matched;
        int* used;
        char input[100];
        char feedback[100];
        int attempt, score, temp_score, win;
        int current_time;
        FILE* config = fopen("vault_config.txt", "r");

        if (config == NULL) {
            printf("Configuration file not found.\n");
            return 1;
        }

        fscanf(config, "CODE_LENGTH=%d\n", &code_length);
        fscanf(config, "DIGIT_MIN=%d\n", &digit_min);
        fscanf(config, "DIGIT_MAX=%d\n", &digit_max);
        fscanf(config, "MAX_ATTEMPTS=%d\n", &max_attempts);
        fscanf(config, "ALLOW_DUPLICATES=%d\n", &allow_duplicates);
        fscanf(config, "POINTS_CORRECT=%d\n", &points_correct);
        fscanf(config, "POINTS_MISPLACED=%d\n", &points_misplaced);
        fscanf(config, "PENALTY_WRONG=%d\n", &penalty_wrong);
        fclose(config);

        FILE* log = fopen("game_log.txt", "w");
        if (log == NULL) {
            printf("Log file could not be created.\n");
            return 1;
        }

        code = generate_code();
        FILE* code_file = fopen("vault_code.txt", "w");
        for (i = 0; i < code_length; i++) {
            fprintf(code_file, "%d", code[i]);
        }
        fprintf(code_file, "\n");
        fclose(code_file);

        time(&current_time);
        fprintf(log, "--- Vault Codebreaker Game Log ---\n");
        fprintf(log, "Game Date: %s", ctime(&current_time));
        fprintf(log, "Secret Code: ");
        for (i = 0; i < code_length; i++) {
            fprintf(log, "%d", code[i]);
        }
        fprintf(log, "\nCode Length: %d\n", code_length);
        fprintf(log, "Digit Range: %d-%d\n", digit_min, digit_max);
        fprintf(log, "Duplicates Allowed: %d\n", allow_duplicates);
        fprintf(log, "Max Attempts: %d\n\n", max_attempts);

        score = 0;

        for (attempt = 1; attempt <= max_attempts; attempt++) {
            printf("Attempt %d: ", attempt);
            guess = get_guess();

            matched = (int*)malloc(sizeof(int) * code_length);
            used = (int*)malloc(sizeof(int) * code_length);
            for (i = 0; i < code_length; i++) {
                matched[i] = 0;
                used[i] = 0;
            }

            temp_score = 0;
            strcpy(feedback, "");

            for (i = 0; i < code_length; i++) {
                if (guess[i] == code[i]) {
                    strcat(feedback, "C ");
                    matched[i] = 1;
                    used[i] = 1;
                    temp_score += points_correct;
                } else {
                    strcat(feedback, "_ ");
                }
            }

            for (i = 0; i < code_length; i++) {
                if (feedback[2 * i] == 'C') continue;
                int found = 0;
                for (j = 0; j < code_length; j++) {
                    if (!matched[j] && guess[i] == code[j] && !used[j]) {
                        found = 1;
                        used[j] = 1;
                        matched[j] = 1;
                        break;
                    }
                }
                if (found) {
                    feedback[2 * i] = 'M';
                    temp_score += points_misplaced;
                } else {
                    feedback[2 * i] = 'W';
                    temp_score -= penalty_wrong;
                }
            }

            score += temp_score;

            fprintf(log, "Attempt %d: ", attempt);
            for (i = 0; i < code_length; i++) {
                fprintf(log, "%d", guess[i]);
            }
            fprintf(log, " => Feedback: %s| Score: %d\n", feedback, score);

            printf("Feedback: %s| Score: %d\n", feedback, score);

            win = 1;
            for (i = 0; i < code_length; i++) {
                if (guess[i] != code[i]) {
                    win = 0;
                    break;
                }
            }

            free(guess);
            free(matched);
            free(used);

            if (win) break;
        }

        fprintf(log, "\nFinal Score: %d\n", score);
        if (score >= 90) {
            fprintf(log, "Rank: Code Master 🧠\n");
            printf("Rank: Code Master 🧠\n");
        } else if (score >= 70) {
            fprintf(log, "Rank: Cipher Hunter 🕵️\n");
            printf("Rank: Cipher Hunter 🕵️\n");
        } else if (score >= 50) {
            fprintf(log, "Rank: Number Sleuth 🔍\n");
            printf("Rank: Number Sleuth 🔍\n");
        } else if (score >= 30) {
            fprintf(log, "Rank: Safe Kicker 🦵\n");
            printf("Rank: Safe Kicker 🦵\n");
        } else if (score >= 10) {
            fprintf(log, "Rank: Lucky Breaker 🍀\n");
            printf("Rank: Lucky Breaker 🍀\n");
        } else {
            fprintf(log, "Rank: Code Potato 🥔\n");
            printf("Rank: Code Potato 🥔\n");
        }
        fprintf(log, "-----------------------------------\n");

        fclose(log);
        free(code);
    }

    return 0;
}

int* generate_code() {
    int i, digit;
    FILE* config = fopen("vault_config.txt", "r");

    int code_length, digit_min, digit_max, max_attempts;
    int allow_duplicates, points_correct, points_misplaced, penalty_wrong;

    fscanf(config, "CODE_LENGTH=%d\n", &code_length);
    fscanf(config, "DIGIT_MIN=%d\n", &digit_min);
    fscanf(config, "DIGIT_MAX=%d\n", &digit_max);
    fscanf(config, "MAX_ATTEMPTS=%d\n", &max_attempts);
    fscanf(config, "ALLOW_DUPLICATES=%d\n", &allow_duplicates);
    fscanf(config, "POINTS_CORRECT=%d\n", &points_correct);
    fscanf(config, "POINTS_MISPLACED=%d\n", &points_misplaced);
    fscanf(config, "PENALTY_WRONG=%d\n", &penalty_wrong);
    fclose(config);

    int* code = (int*)malloc(sizeof(int) * code_length);
    int used[10] = {0};

    do {
        for (i = 0; i < code_length; ) {
            digit = digit_min + rand() % (digit_max - digit_min + 1);
            if (!allow_duplicates && used[digit]) continue;
            code[i++] = digit;
            if (!allow_duplicates) used[digit] = 1;
        }
    } while (code[0] == 0);

    return code;
}

int* get_guess() {
    FILE* config = fopen("vault_config.txt", "r");

    int code_length, digit_min, digit_max, max_attempts;
    int allow_duplicates, points_correct, points_misplaced, penalty_wrong;
    char input[100];
    int i;

    fscanf(config, "CODE_LENGTH=%d\n", &code_length);
    fscanf(config, "DIGIT_MIN=%d\n", &digit_min);
    fscanf(config, "DIGIT_MAX=%d\n", &digit_max);
    fscanf(config, "MAX_ATTEMPTS=%d\n", &max_attempts);
    fscanf(config, "ALLOW_DUPLICATES=%d\n", &allow_duplicates);
    fscanf(config, "POINTS_CORRECT=%d\n", &points_correct);
    fscanf(config, "POINTS_MISPLACED=%d\n", &points_misplaced);
    fscanf(config, "PENALTY_WRONG=%d\n", &penalty_wrong);
    fclose(config);

    int* guess = (int*)malloc(sizeof(int) * code_length);
    
    while (1) {
        printf("Enter your guess (%d digits): ", code_length);
        scanf("%s", input);
        
        if (strlen(input) != code_length) {
            printf("Invalid input length. Please enter exactly %d digits.\n", code_length);
            continue;
        }
        
        int valid = 1;
        for (i = 0; i < code_length; i++) {
            if (input[i] < '0' || input[i] > '9') {
                valid = 0;
                break;
            }
        }
        
        if (!valid) {
            printf("Invalid input. Please enter digits only.\n");
            continue;
        }
        
        break;
    }

    for (i = 0; i < code_length; i++) {
        guess[i] = input[i] - '0';
    }

    return guess;
}