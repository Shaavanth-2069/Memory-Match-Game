/*
 * memory_match_game.c
 * A simple terminal-based Memory Match (Concentration) game in C.
 *
 * Features:
 * - Default 4x4 board (8 pairs). Can change BOARD_ROWS and BOARD_COLS.
 * - Randomly places pairs of symbols.
 * - Player picks two cards each turn; revealed if matching.
 * - Tracks moves and time elapsed.
 * - Simple input validation and friendly messages.
 *
 * Compile: gcc memory_match_game.c -o memory_match_game
 * Run:     ./memory_match_game
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

#define BOARD_ROWS 4
#define BOARD_COLS 4
#define MAX_PAIRS ((BOARD_ROWS * BOARD_COLS) / 2)

typedef struct {
    char symbol;    // symbol shown when revealed
    bool revealed;  // temporarily revealed this turn
    bool matched;   // permanently matched
} Card;

void init_board(Card board[BOARD_ROWS][BOARD_COLS]);
void shuffle_symbols(char *symbols, int count);
void display_board(Card board[BOARD_ROWS][BOARD_COLS]);
void display_board_with_hint(Card board[BOARD_ROWS][BOARD_COLS]);
bool pick_card(Card board[BOARD_ROWS][BOARD_COLS], int *row, int *col);
void clear_input_buffer(void);

int main(void) {
    if ((BOARD_ROWS * BOARD_COLS) % 2 != 0) {
        printf("BOARD_ROWS * BOARD_COLS must be even (pairs).\n");
        return 1;
    }

    Card board[BOARD_ROWS][BOARD_COLS];
    int pairs_found = 0;
    int total_pairs = (BOARD_ROWS * BOARD_COLS) / 2;
    int moves = 0;

    srand((unsigned) time(NULL));

    init_board(board);

    printf("Welcome to Memory Match!\n");
    printf("Match all %d pairs. Enter coordinates as row and column (1-based).\n\n", total_pairs);

    time_t start = time(NULL);

    while (pairs_found < total_pairs) {
        display_board(board);

        int r1, c1, r2, c2;
        printf("Pick first card:\n");
        if (!pick_card(board, &r1, &c1)) continue;
        board[r1][c1].revealed = true;
        display_board(board);

        printf("Pick second card:\n");
        if (!pick_card(board, &r2, &c2)) {
            // If invalid pick for second, hide the first again
            board[r1][c1].revealed = false;
            continue;
        }

        // Prevent picking the same card twice
        if (r1 == r2 && c1 == c2) {
            printf("You picked the same card twice. Try again.\n\n");
            board[r1][c1].revealed = false;
            continue;
        }

        board[r2][c2].revealed = true;
        display_board(board);

        moves++;

        if (board[r1][c1].symbol == board[r2][c2].symbol) {
            printf("Nice! It's a match.\n\n");
            board[r1][c1].matched = true;
            board[r2][c2].matched = true;
            pairs_found++;
        } else {
            printf("Not a match. Cards will be hidden.\n\n");
            // short pause so player can see (not sleeping to keep portability)
            printf("Press Enter to continue...");
            clear_input_buffer();
            getchar();
            board[r1][c1].revealed = false;
            board[r2][c2].revealed = false;
        }
    }

    time_t end = time(NULL);
    int seconds = (int) difftime(end, start);

    printf("CONGRATULATIONS! You matched all pairs.\n");
    printf("Moves: %d\n", moves);
    printf("Time: %d:%02d (minutes:seconds)\n", seconds / 60, seconds % 60);

    return 0;
}

// Initialize board with shuffled pairs of symbols
void init_board(Card board[BOARD_ROWS][BOARD_COLS]) {
    // Prepare a symbol list big enough for pairs.
    // Using uppercase letters then lowercase then digits if needed.
    char pool[128];
    int pool_idx = 0;

    for (char c = 'A'; c <= 'Z' && pool_idx < (int)sizeof(pool)-1; ++c) pool[pool_idx++] = c;
    for (char c = 'a'; c <= 'z' && pool_idx < (int)sizeof(pool)-1; ++c) pool[pool_idx++] = c;
    for (char c = '0'; c <= '9' && pool_idx < (int)sizeof(pool)-1; ++c) pool[pool_idx++] = c;
    pool[pool_idx] = '\0';

    int pair_count = (BOARD_ROWS * BOARD_COLS) / 2;
    if (pair_count > pool_idx) {
        printf("Not enough unique symbols to create pairs. Reduce board size.\n");
        exit(1);
    }

    // Create array of symbols with each symbol appearing twice
    char symbols[BOARD_ROWS * BOARD_COLS + 1];
    int idx = 0;
    for (int i = 0; i < pair_count; ++i) {
        symbols[idx++] = pool[i];
        symbols[idx++] = pool[i];
    }

    // Shuffle symbols
    shuffle_symbols(symbols, BOARD_ROWS * BOARD_COLS);

    // Fill board
    idx = 0;
    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            board[r][c].symbol = symbols[idx++];
            board[r][c].revealed = false;
            board[r][c].matched = false;
        }
    }
}

// Fisher-Yates shuffle
void shuffle_symbols(char *symbols, int count) {
    for (int i = count - 1; i > 0; --i) {
        int j = rand() % (i + 1);
        char tmp = symbols[i];
        symbols[i] = symbols[j];
        symbols[j] = tmp;
    }
}

// Display board to the player. Revealed or matched cards show symbol; others show '*'
void display_board(Card board[BOARD_ROWS][BOARD_COLS]) {
    printf("\n    ");
    for (int c = 0; c < BOARD_COLS; ++c) printf("  %d ", c + 1);
    printf("\n   +");
    for (int c = 0; c < BOARD_COLS; ++c) printf("---+");
    printf("\n");

    for (int r = 0; r < BOARD_ROWS; ++r) {
        printf(" %d |", r + 1);
        for (int c = 0; c < BOARD_COLS; ++c) {
            if (board[r][c].matched || board[r][c].revealed) {
                printf(" %c |", board[r][c].symbol);
            } else {
                printf(" * |");
            }
        }
        printf("\n   +");
        for (int c = 0; c < BOARD_COLS; ++c) printf("---+");
        printf("\n");
    }
    printf("\n");
}

// Ask player to pick a card, validate input, return 1 if valid, 0 otherwise.
bool pick_card(Card board[BOARD_ROWS][BOARD_COLS], int *row, int *col) {
    int r, c;
    printf("Enter row (1-%d) and column (1-%d) separated by space: ", BOARD_ROWS, BOARD_COLS);
    int scanned = scanf("%d %d", &r, &c);
    clear_input_buffer();

    if (scanned != 2) {
        printf("Invalid input. Please enter two numbers.\n\n");
        return false;
    }

    if (r < 1 || r > BOARD_ROWS || c < 1 || c > BOARD_COLS) {
        printf("Coordinates out of range. Try again.\n\n");
        return false;
    }

    r--; c--; // convert to 0-based

    if (board[r][c].matched) {
        printf("That card is already matched. Pick another.\n\n");
        return false;
    }

    if (board[r][c].revealed) {
        printf("That card is already revealed this turn. Pick another.\n\n");
        return false;
    }

    *row = r;
    *col = c;
    return true;
}

// Clear remaining input buffer up to newline
void clear_input_buffer(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) ;
}