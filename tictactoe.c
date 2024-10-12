#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 20
#define WINNING_LENGTH 5

typedef enum { EMPTY, PLAYER_X, PLAYER_O } Cell;

Cell board[MAX_SIZE][MAX_SIZE];
int width, height;

void initBoard() {
    for (int i = 0; i < MAX_SIZE; i++)
        for (int j = 0; j < MAX_SIZE; j++)
            board[i][j] = EMPTY;
}

void drawBoard(SDL_Renderer* renderer) {
    for (int i = 0; i <= width; i++) {
        SDL_RenderDrawLine(renderer, i * 50, 0, i * 50, height * 50);
    }
    for (int i = 0; i <= height; i++) {
        SDL_RenderDrawLine(renderer, 0, i * 50, width * 50, i * 50);
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (board[i][j] == PLAYER_X) {
                // Draw X
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_RenderDrawLine(renderer, j * 50 + 10, i * 50 + 10, j * 50 + 40, i * 50 + 40);
                SDL_RenderDrawLine(renderer, j * 50 + 40, i * 50 + 10, j * 50 + 10, i * 50 + 40);
            } else if (board[i][j] == PLAYER_O) {
                // Draw O
                SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
                for (int r = 0; r < 50; r++) {
                    for (int angle = 0; angle < 360; angle++) {
                        int x = r * cos(angle * M_PI / 180) + j * 50 + 25;
                        int y = r * sin(angle * M_PI / 180) + i * 50 + 25;
                        SDL_RenderDrawPoint(renderer, x, y);
                    }
                }
            }
        }
    }
}

int checkWin(Cell player) {
    // Check horizontal, vertical and diagonal for winning condition
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (board[i][j] == player) {
                // Check horizontal
                if (j <= width - WINNING_LENGTH && 
                    board[i][j + 1] == player && 
                    board[i][j + 2] == player && 
                    board[i][j + 3] == player && 
                    board[i][j + 4] == player) return 1;
                
                // Check vertical
                if (i <= height - WINNING_LENGTH && 
                    board[i + 1][j] == player && 
                    board[i + 2][j] == player && 
                    board[i + 3][j] == player && 
                    board[i + 4][j] == player) return 1;

                // Check diagonal 
                if (i <= height - WINNING_LENGTH && j <= width - WINNING_LENGTH &&
                    board[i + 1][j + 1] == player && 
                    board[i + 2][j + 2] == player && 
                    board[i + 3][j + 3] == player && 
                    board[i + 4][j + 4] == player) return 1;

                // Check diagonal /
                if (i >= WINNING_LENGTH - 1 && j <= width - WINNING_LENGTH &&
                    board[i - 1][j + 1] == player && 
                    board[i - 2][j + 2] == player && 
                    board[i - 3][j + 3] == player && 
                    board[i - 4][j + 4] == player) return 1;
            }
        }
    }
    return 0;
}

void aiMove() {
    // Simple AI that makes a random move
    int x, y;
    do {
        x = rand() % height;
        y = rand() % width;
    } while (board[x][y] != EMPTY);
    
    board[x][y] = PLAYER_O;
}

int main(int argc, char* argv[]) {
    printf("Enter width and height of the board (max %d): ", MAX_SIZE);
    scanf("%d %d", &width, &height);
        if (width > MAX_SIZE || height > MAX_SIZE || width < 1 || height < 1) {
        printf("Invalid size!n");
        return -1;
    }

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Tic Tac Toe", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width * 50, height * 50, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    initBoard();
    
    int running = 1;
    Cell currentPlayer = PLAYER_X;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
            if (event.type == SDL_MOUSEBUTTONDOWN && currentPlayer == PLAYER_X) {
                int x = event.button.x / 50;
                int y = event.button.y / 50;

                if (x < width && y < height && board[y][x] == EMPTY) {
                    board[y][x] = PLAYER_X;
                    if (!checkWin(PLAYER_X)) {
                        currentPlayer = PLAYER_O;
                        aiMove();
                        currentPlayer = PLAYER_X;
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        
        drawBoard(renderer);
        
        SDL_RenderPresent(renderer);

        if (checkWin(PLAYER_X)) {
            printf("Player X wins!n");
            running = 0;
        } else if (checkWin(PLAYER_O)) {
            printf("Player O wins!n");
            running = 0;
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
