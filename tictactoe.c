#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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


void renderText(SDL_Renderer* renderer, const char* message, int x, int y, SDL_Color color) {
    TTF_Font* font = TTF_OpenFont("arial.ttf", 24);
    if (!font) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return;
    }

    SDL_Surface* surface = TTF_RenderText_Solid(font, message, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect rect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    TTF_CloseFont(font);
}


int checkDraw() {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (board[i][j] == EMPTY) {
                return 0; // Есть пустая клетка, продолжаем игру.
            }
        }
    }
    return 1; // Все клетки заполнены, ничья.
}


void drawBoard(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    for (int i = 0; i <= width; i++) {
        SDL_RenderDrawLine(renderer, i * 50, 0, i * 50, height * 50);
    }
    for (int i = 0; i <= height; i++) {
        SDL_RenderDrawLine(renderer, 0, i * 50, width * 50, i * 50);
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (board[i][j] == PLAYER_X) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_RenderDrawLine(renderer, j * 50 + 10, i * 50 + 10, j * 50 + 40, i * 50 + 40);
                SDL_RenderDrawLine(renderer, j * 50 + 40, i * 50 + 10, j * 50 + 10, i * 50 + 40);
            } else if (board[i][j] == PLAYER_O) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                for (int angle = 0; angle < 360; angle++) {
                    int x = 20 * cos(angle * M_PI / 180) + j * 50 + 25;
                    int y = 20 * sin(angle * M_PI / 180) + i * 50 + 25;
                    SDL_RenderDrawPoint(renderer, x, y);
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
        printf("Invalid size!\n");
        return -1;
    }

    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init(); // Инициализация TTF для отображения текста
    SDL_Window* window = SDL_CreateWindow("Tic Tac Toe", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width * 50, height * 50, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    initBoard();
    
    int running = 1;
    Cell currentPlayer = PLAYER_X;
    int gameOver = 0;
    char message[50] = "";

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;

            if (event.type == SDL_MOUSEBUTTONDOWN && !gameOver) {
                int x = event.button.x / 50;
                int y = event.button.y / 50;

                if (x < width && y < height && board[y][x] == EMPTY) {
                    board[y][x] = PLAYER_X;
                    if (checkWin(PLAYER_X)) {
                        snprintf(message, sizeof(message), "Player X wins!");
                        gameOver = 1;
                    } else if (checkDraw()) {
                        snprintf(message, sizeof(message), "It's a draw!");
                        gameOver = 1;
                    } else {
                        currentPlayer = PLAYER_O;
                        aiMove();
                        if (checkWin(PLAYER_O)) {
                            snprintf(message, sizeof(message), "Player O wins!");
                            gameOver = 1;
                        } else if (checkDraw()) {
                            snprintf(message, sizeof(message), "It's a draw!");
                            gameOver = 1;
                        }
                        currentPlayer = PLAYER_X;
                    }
                }
            }

            if (gameOver && (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_KEYDOWN)) {
                running = 0; // Закрываем игру при нажатии клавиши или клике
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        
        drawBoard(renderer);

        if (gameOver) {
            SDL_Color color = {0, 0, 0, 255}; // Черный цвет текста
            renderText(renderer, message, 10, height * 50 / 2 - 20, color);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}