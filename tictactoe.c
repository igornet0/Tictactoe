#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define CELL_SIZE 50
#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 700
#define WINNING_LENGTH 5

typedef enum { EMPTY, PLAYER_X, PLAYER_O } Cell;

// Используем статическое поле большого размера, так как "бесконечное" поле физически невозможно.
// Однако мы будем использовать технику прокрутки с отображением только видимой части.
#define MAX_SIZE 1000
Cell board[MAX_SIZE][MAX_SIZE];

int cameraX = 0; // Положение камеры по X
int cameraY = 0; // Положение камеры по Y


// Функция инициализации игрового поля, заполняет все клетки значением EMPTY
void initBoard() {
    for (int i = 0; i < MAX_SIZE; i++)
        for (int j = 0; j < MAX_SIZE; j++)
            board[i][j] = EMPTY;
}


// Функция отображения текста на экране с использованием SDL_ttf
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


// Функция для отображения сообщения в диалоговом окне с кнопками "Закрыть" и "Снова"
void drawMessageBox(SDL_Renderer* renderer, const char* message) {
    SDL_Color bgColor = {255, 255, 255, 255}; // Белый фон окна
    SDL_Color textColor = {0, 0, 0, 255}; // Черный цвет текста

    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_Rect messageRect = {WINDOW_WIDTH / 4, WINDOW_HEIGHT / 3, WINDOW_WIDTH / 2, 150};
    SDL_RenderFillRect(renderer, &messageRect);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &messageRect);

    renderText(renderer, message, messageRect.x + 10, messageRect.y + 20, textColor);

    SDL_Rect closeButton = {messageRect.x + 10, messageRect.y + 70, 100, 40};
    SDL_Rect retryButton = {messageRect.x + messageRect.w - 110, messageRect.y + 70, 100, 40};

    SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255); // Красный цвет для кнопки "Закрыть"
    SDL_RenderFillRect(renderer, &closeButton);
    renderText(renderer, "Close", closeButton.x + 10, closeButton.y + 10, textColor);

    SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255); // Зеленый цвет для кнопки "Снова"
    SDL_RenderFillRect(renderer, &retryButton);
    renderText(renderer, "Retry", retryButton.x + 10, retryButton.y + 10, textColor);
}


// Проверка, был ли клик внутри прямоугольника
int isClickInsideRect(SDL_Rect rect, int x, int y) {
    return (x >= rect.x && x <= rect.x + rect.w && y >= rect.y && y <= rect.y + rect.h);
}


// Функция проверки ничьей - проверяет, заполнены ли все клетки
int checkDraw() {
    for (int i = 0; i < MAX_SIZE; i++) {
        for (int j = 0; j < MAX_SIZE; j++) {
            if (board[i][j] == EMPTY) {
                return 0; // Есть пустая клетка, продолжаем игру.
            }
        }
    }
    return 1; // Все клетки заполнены, ничья.
}


// Функция отрисовки видимой части игрового поля с учетом камеры
void drawBoard(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    int startX = cameraX / CELL_SIZE;
    int startY = cameraY / CELL_SIZE;
    int endX = (cameraX + WINDOW_WIDTH) / CELL_SIZE + 1;
    int endY = (cameraY + WINDOW_HEIGHT) / CELL_SIZE + 1;

    for (int i = startY; i <= endY; i++) {
        for (int j = startX; j <= endX; j++) {
            int x = j * CELL_SIZE - cameraX;
            int y = i * CELL_SIZE - cameraY;

            SDL_Rect cellRect = { x, y, CELL_SIZE, CELL_SIZE };
            SDL_RenderDrawRect(renderer, &cellRect);

            if (board[i][j] == PLAYER_X) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_RenderDrawLine(renderer, x + 10, y + 10, x + CELL_SIZE - 10, y + CELL_SIZE - 10);
                SDL_RenderDrawLine(renderer, x + CELL_SIZE - 10, y + 10, x + 10, y + CELL_SIZE - 10);
            } else if (board[i][j] == PLAYER_O) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
                for (int angle = 0; angle < 360; angle++) {
                    int drawX = 20 * cos(angle * M_PI / 180) + x + CELL_SIZE / 2;
                    int drawY = 20 * sin(angle * M_PI / 180) + y + CELL_SIZE / 2;
                    SDL_RenderDrawPoint(renderer, drawX, drawY);
                }
            }
        }
    }
}


// Функция проверки победы - проверяет горизонтальные, вертикальные и диагональные линии
int checkWin(Cell player) {
    // Проверка всех выигрышных комбинаций
    for (int i = 0; i < MAX_SIZE; i++) {
        for (int j = 0; j <= MAX_SIZE - WINNING_LENGTH; j++) {
            // Горизонтальная проверка
            if (board[i][j] == player) {
                int count = 0;
                for (int k = 0; k < WINNING_LENGTH; k++) {
                    if (board[i][j + k] == player) count++;
                }
                if (count == WINNING_LENGTH) return 1;
            }
        }
    }

    for (int i = 0; i <= MAX_SIZE - WINNING_LENGTH; i++) {
        for (int j = 0; j < MAX_SIZE; j++) {
            // Вертикальная проверка
            if (board[i][j] == player) {
                int count = 0;
                for (int k = 0; k < WINNING_LENGTH; k++) {
                    if (board[i + k][j] == player) count++;
                }
                if (count == WINNING_LENGTH) return 1;
            }
        }
    }

    for (int i = 0; i <= MAX_SIZE - WINNING_LENGTH; i++) {
        for (int j = 0; j <= MAX_SIZE - WINNING_LENGTH; j++) {
            // Диагональная проверка
            if (board[i][j] == player) {
                int count1 = 0, count2 = 0;
                for (int k = 0; k < WINNING_LENGTH; k++) {
                    if (board[i + k][j + k] == player) count1++;
                    if (board[i + k][j + WINNING_LENGTH - 1 - k] == player) count2++;
                }
                if (count1 == WINNING_LENGTH || count2 == WINNING_LENGTH) return 1;
            }
        }
    }

    return 0;
}

int minimax(int depth, int isMaximizing) {
    if (checkWin(PLAYER_O)) return 10 - depth;
    if (checkWin(PLAYER_X)) return depth - 10;

    int isDraw = 1;
    for (int i = 0; i < MAX_SIZE; i++) {
        for (int j = 0; j < MAX_SIZE; j++) {
            if (board[i][j] == EMPTY) {
                isDraw = 0;
                break;
            }
        }
    }
    if (isDraw) return 0;

    if (isMaximizing) {
        int bestScore = -1000;
        for (int i = 0; i < MAX_SIZE; i++) {
            for (int j = 0; j < MAX_SIZE; j++) {
                if (board[i][j] == EMPTY) {
                    board[i][j] = PLAYER_O;
                    int score = minimax(depth + 1, 0);
                    board[i][j] = EMPTY;
                    bestScore = fmax(score, bestScore);
                }
            }
        }
        return bestScore;
    } else {
        int bestScore = 1000;
        for (int i = 0; i < MAX_SIZE; i++) {
            for (int j = 0; j < MAX_SIZE; j++) {
                if (board[i][j] == EMPTY) {
                    board[i][j] = PLAYER_X;
                    int score = minimax(depth + 1, 1);
                    board[i][j] = EMPTY;
                    bestScore = fmin(score, bestScore);
                }
            }
        }
        return bestScore;
    }
}

// Функция хода компьютера
void aiMove() {
    int bestX = -1, bestY = -1;
    int bestScore = -1000;

    for (int i = 0; i < MAX_SIZE; i++) {
        for (int j = 0; j < MAX_SIZE; j++) {
            if (board[i][j] == EMPTY) {
                board[i][j] = PLAYER_O;
                int score = minimax(0, 0);
                board[i][j] = EMPTY;
                if (score > bestScore) {
                    bestScore = score;
                    bestX = j;
                    bestY = i;
                }
            }
        }
    }

    if (bestX != -1 && bestY != -1) {
        board[bestY][bestX] = PLAYER_O;
    }
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    SDL_Window* window = SDL_CreateWindow("Infinite Tic Tac Toe", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    initBoard();
    
    int running = 1;
    int gameOver = 0;
    Cell currentPlayer = PLAYER_X;
    char message[50] = "";

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }

            // Обработка стрелок для перемещения камеры
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP:
                    case SDLK_w: cameraY -= CELL_SIZE; break;
                    case SDLK_DOWN:
                    case SDLK_s: cameraY += CELL_SIZE; break;
                    case SDLK_LEFT:
                    case SDLK_a: cameraX -= CELL_SIZE; break;
                    case SDLK_RIGHT:
                    case SDLK_d: cameraX += CELL_SIZE; break;
                }
            }

            // Обработка кликов мыши для размещения X или O
            if (event.type == SDL_MOUSEBUTTONDOWN && !gameOver) {
                int x = (event.button.x + cameraX) / CELL_SIZE;
                int y = (event.button.y + cameraY) / CELL_SIZE;

                if (board[y][x] == EMPTY) {
                    board[y][x] = currentPlayer;

                    // Проверка на победу и переключение игрока
                    if (checkWin(currentPlayer)) {
                        snprintf(message, sizeof(message), "Player %c wins!", currentPlayer == PLAYER_X ? 'X' : 'O');
                        gameOver = 1;
                    } else if (checkDraw()) {
                        snprintf(message, sizeof(message), "It's a draw!");
                        gameOver = 1;
                    } else {
                        currentPlayer = (currentPlayer == PLAYER_X) ? PLAYER_O : PLAYER_X;

                        if (currentPlayer == PLAYER_O) {
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
            }

            // Обработка нажатий на кнопки "Закрыть" и "Снова" в диалоговом окне
            if (gameOver && event.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX = event.button.x;
                int mouseY = event.button.y;

                SDL_Rect closeButton = {WINDOW_WIDTH / 4 + 10, WINDOW_HEIGHT / 3 + 70, 100, 40};
                SDL_Rect retryButton = {WINDOW_WIDTH / 4 + WINDOW_WIDTH / 2 - 110, WINDOW_HEIGHT / 3 + 70, 100, 40};

                if (isClickInsideRect(closeButton, mouseX, mouseY)) {
                    running = 0; // Закрываем игру
                } else if (isClickInsideRect(retryButton, mouseX, mouseY)) {
                    initBoard(); // Сбросить поле
                    gameOver = 0; // Сбросить состояние игры
                    snprintf(message, sizeof(message), "");
                }
            }
        }

        // Отрисовка игрового поля и сообщений
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        
        drawBoard(renderer);

        if (gameOver) {
            drawMessageBox(renderer, message);
        }

        SDL_RenderPresent(renderer);
    }

    // Очистка ресурсов и завершение работы SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}