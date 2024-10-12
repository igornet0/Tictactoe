#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define CELL_SIZE 50
#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 700

typedef enum { EMPTY, PLAYER_X, PLAYER_O } Cell;

// Используем статическое поле большого размера, так как "бесконечное" поле физически невозможно.
// Однако мы будем использовать технику прокрутки с отображением только видимой части.
#define MAX_SIZE 1000
Cell board[MAX_SIZE][MAX_SIZE];

int cameraX = 0; // Положение камеры по X
int cameraY = 0; // Положение камеры по Y

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

void drawMessageBox(SDL_Renderer* renderer, const char* message) {
    SDL_Color bgColor = {255, 255, 255, 255}; // Белый фон окна
    SDL_Color textColor = {0, 0, 0, 255}; // Черный цвет текста

    // Устанавливаем цвет и рисуем прямоугольник в центре экрана
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_Rect messageRect = {WINDOW_WIDTH * 50 / 4, WINDOW_HEIGHT * 50 / 3, WINDOW_WIDTH * 50 / 2, 150};
    SDL_RenderFillRect(renderer, &messageRect);

    // Рисуем рамку вокруг прямоугольника
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &messageRect);

    // Отрисовываем текст в центре прямоугольника
    renderText(renderer, message, messageRect.x + 10, messageRect.y + 20, textColor);

    // Рисуем кнопки "Закрыть" и "Снова"
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


void drawBoard(SDL_Renderer* renderer) {
    // Устанавливаем цвет линий сетки
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    // Вычисляем видимые границы на экране
    int startX = cameraX / CELL_SIZE;
    int startY = cameraY / CELL_SIZE;
    int endX = (cameraX + WINDOW_WIDTH) / CELL_SIZE + 1;
    int endY = (cameraY + WINDOW_HEIGHT) / CELL_SIZE + 1;

    // Рисуем видимые клетки поля
    for (int i = startY; i <= endY; i++) {
        for (int j = startX; j <= endX; j++) {
            int x = j * CELL_SIZE - cameraX;
            int y = i * CELL_SIZE - cameraY;

            // Рисуем границы клетки
            SDL_Rect cellRect = { x, y, CELL_SIZE, CELL_SIZE };
            SDL_RenderDrawRect(renderer, &cellRect);

            // Рисуем X и O в клетках
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


int checkWin(Cell player) {
    // Check horizontal, vertical and diagonal for winning condition
    for (int i = 0; i < WINDOW_HEIGHT; i++) {
        for (int j = 0; j < WINDOW_WIDTH; j++) {
            if (board[i][j] == player) {
                // Check horizontal
                if (j <= WINDOW_WIDTH - WINNING_LENGTH && 
                    board[i][j + 1] == player && 
                    board[i][j + 2] == player && 
                    board[i][j + 3] == player && 
                    board[i][j + 4] == player) return 1;
                
                // Check vertical
                if (i <= WINDOW_HEIGHT - WINNING_LENGTH && 
                    board[i + 1][j] == player && 
                    board[i + 2][j] == player && 
                    board[i + 3][j] == player && 
                    board[i + 4][j] == player) return 1;

                // Check diagonal 
                if (i <= WINDOW_HEIGHT - WINNING_LENGTH && j <= WINDOW_WIDTH - WINNING_LENGTH &&
                    board[i + 1][j + 1] == player && 
                    board[i + 2][j + 2] == player && 
                    board[i + 3][j + 3] == player && 
                    board[i + 4][j + 4] == player) return 1;

                // Check diagonal /
                if (i >= WINNING_LENGTH - 1 && j <= WINDOW_WIDTH - WINNING_LENGTH &&
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
        x = rand() % WINDOW_HEIGHT;
        y = rand() % width;
    } while (board[x][y] != EMPTY);
    
    board[x][y] = PLAYER_O;
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init(); // Инициализация TTF для отображения текста
    SDL_Window* window = SDL_CreateWindow("Infinite Tic Tac Toe", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    initBoard();
    
    int running = 1;
    Cell currentPlayer = PLAYER_X;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;

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
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                int x = (event.button.x + cameraX) / CELL_SIZE;
                int y = (event.button.y + cameraY) / CELL_SIZE;

                if (board[y][x] == EMPTY) {
                    board[y][x] = currentPlayer;
                    currentPlayer = (currentPlayer == PLAYER_X) ? PLAYER_O : PLAYER_X;
                }
            }
        }

        // Отрисовка
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        
        drawBoard(renderer);

        SDL_RenderPresent(renderer);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}