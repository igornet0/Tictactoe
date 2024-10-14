#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define CELL_SIZE 50
#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 500
#define WINNING_LENGTH 5

typedef enum { EMPTY, PLAYER_X, PLAYER_O } Cell;

// Устанавливаем максимально возможный размер игрового поля
#define MAX_SIZE 100000000000000
Cell board[MAX_SIZE][MAX_SIZE];

int cameraX = 0; // Координаты камеры по X
int cameraY = 0; // Координаты камеры по Y

// Инициализация игрового поля
void initBoard(int* emptyCells) {
    *emptyCells = MAX_SIZE * MAX_SIZE; // Инициализируем количество пустых клеток
    for (int i = 0; i < MAX_SIZE; i++)
        for (int j = 0; j < MAX_SIZE; j++)
            board[i][j] = EMPTY;
}

// Функция отображения текста
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

// Отрисовка окна с результатом игры и кнопками "Close" и "Retry"
void drawMessageBox(SDL_Renderer* renderer, const char* message) {
    SDL_Color bgColor = {255, 255, 255, 255};
    SDL_Color textColor = {0, 0, 0, 255};

    // Рисуем фон и рамку окна
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_Rect messageRect = {WINDOW_WIDTH / 4, WINDOW_HEIGHT / 3, WINDOW_WIDTH / 2, 150};
    SDL_RenderFillRect(renderer, &messageRect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &messageRect);

    renderText(renderer, message, messageRect.x + 10, messageRect.y + 20, textColor);

    // Рисуем кнопки
    SDL_Rect closeButton = {messageRect.x + 10, messageRect.y + 70, 100, 40};
    SDL_Rect retryButton = {messageRect.x + messageRect.w - 110, messageRect.y + 70, 100, 40};

    SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
    SDL_RenderFillRect(renderer, &closeButton);
    renderText(renderer, "Close", closeButton.x + 10, closeButton.y + 10, textColor);

    SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
    SDL_RenderFillRect(renderer, &retryButton);
    renderText(renderer, "Retry", retryButton.x + 10, retryButton.y + 10, textColor);
}

// Проверка, находится ли клик внутри заданного прямоугольника
int isClickInsideRect(SDL_Rect rect, int x, int y) {
    return (x >= rect.x && x <= rect.x + rect.w && y >= rect.y && y <= rect.y + rect.h);
}

// Отрисовка игрового поля
void drawBoard(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    // Рассчитываем границы видимой области
    int startX = cameraX / CELL_SIZE;
    int startY = cameraY / CELL_SIZE;
    int endX = (cameraX + WINDOW_WIDTH) / CELL_SIZE + 1;
    int endY = (cameraY + WINDOW_HEIGHT) / CELL_SIZE + 1;

    // Отрисовка клеток на видимой части поля
    for (int i = startY; i <= endY; i++) {
        for (int j = startX; j <= endX; j++) {
            // Оборачивание координат клеток
            int wrappedX = j % MAX_SIZE;
            int wrappedY = i % MAX_SIZE;
            if (wrappedX < 0) wrappedX += MAX_SIZE;
            if (wrappedY < 0) wrappedY += MAX_SIZE;

            int x = j * CELL_SIZE - cameraX;
            int y = i * CELL_SIZE - cameraY;

            SDL_Rect cellRect = { x, y, CELL_SIZE, CELL_SIZE };
            SDL_RenderDrawRect(renderer, &cellRect);

            // Отрисовка X или O в клетке
            if (board[wrappedY][wrappedX] == PLAYER_X) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_RenderDrawLine(renderer, x + 10, y + 10, x + CELL_SIZE - 10, y + CELL_SIZE - 10);
                SDL_RenderDrawLine(renderer, x + CELL_SIZE - 10, y + 10, x + 10, y + CELL_SIZE - 10);
            } else if (board[wrappedY][wrappedX] == PLAYER_O) {
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

// Оптимизированная проверка победы по последнему ходу игрока
int checkWin(Cell player, int lastX, int lastY) {
    int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}}; // Проверка горизонтали, вертикали и диагоналей

    // Проходим по каждому направлению
    for (int d = 0; d < 4; d++) {
        int count = 1; // Считаем последовательно идущие символы игрока
        int dx = directions[d][0];
        int dy = directions[d][1];

        // Проверка в одном направлении
        for (int step = 1; step < WINNING_LENGTH; step++) {
            int x = lastX + step * dx;
            int y = lastY + step * dy;
            if (x >= 0 && x < MAX_SIZE && y >= 0 && y < MAX_SIZE && board[y][x] == player) {
                count++;
            } else {
                break;
            }
        }

        // Проверка в противоположном направлении
        for (int step = 1; step < WINNING_LENGTH; step++) {
            int x = lastX - step * dx;
            int y = lastY - step * dy;
            if (x >= 0 && x < MAX_SIZE && y >= 0 && y < MAX_SIZE && board[y][x] == player) {
                count++;
            } else {
                break;
            }
        }

        // Проверка на победу
        if (count >= WINNING_LENGTH) {
            return 1;
        }
    }
    return 0;
}

// Функция для хода компьютера, блокирующая игрока и реагирующая на последний ход
void aiMove(int lastPlayerX, int lastPlayerY) {
    int blockX = -1, blockY = -1;
    int searchRadius = 2; // Радиус поиска вокруг последнего хода игрока

    // Поиск угроз от игрока в ограниченной области вокруг последнего хода
    for (int i = lastPlayerY - searchRadius; i <= lastPlayerY + searchRadius; i++) {
        for (int j = lastPlayerX - searchRadius; j <= lastPlayerX + searchRadius; j++) {
            // Проверяем, что координаты находятся в пределах поля и клетка пуста
            if (i >= 0 && i < MAX_SIZE && j >= 0 && j < MAX_SIZE && board[i][j] == EMPTY) {
                // Проверяем, если бы игрок поставил "X" в эту клетку, это привело бы к победе?
                board[i][j] = PLAYER_X;
                if (checkWin(PLAYER_X, j, i)) {
                    blockX = j;
                    blockY = i;
                }
                board[i][j] = EMPTY;

                // Если нашли угрозу, блокируем её
                if (blockX != -1 && blockY != -1) {
                    board[blockY][blockX] = PLAYER_O;
                    return;
                }
            }
        }
    }

    // Если угрозы нет, ходим рядом с последним ходом игрока
    int directions[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};
    for (int d = 0; d < 8; d++) {
        int newX = lastPlayerX + directions[d][0];
        int newY = lastPlayerY + directions[d][1];
        if (newX >= 0 && newX < MAX_SIZE && newY >= 0 && newY < MAX_SIZE && board[newY][newX] == EMPTY) {
            board[newY][newX] = PLAYER_O;
            return;
        }
    }

    // В крайнем случае делаем случайный ход
    int x, y;
    do {
        x = rand() % MAX_SIZE;
        y = rand() % MAX_SIZE;
    } while (board[y][x] != EMPTY);

    board[y][x] = PLAYER_O;
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    SDL_Window* window = SDL_CreateWindow("Infinite Tic Tac Toe", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    int emptyCells;
    initBoard(&emptyCells);

    int running = 1;
    int gameOver = 0;
    Cell currentPlayer = PLAYER_X;
    char message[50] = "";

    // Функция для проверки состояния игры после хода
    void checkGameState(int x, int y) {
        if (checkWin(currentPlayer, x, y)) {
            snprintf(message, sizeof(message), "Player %c wins!", currentPlayer == PLAYER_X ? 'X' : 'O');
            gameOver = 1;
        } else if (emptyCells == 0) {
            snprintf(message, sizeof(message), "It's a draw!");
            gameOver = 1;
        } else {
            currentPlayer = (currentPlayer == PLAYER_X) ? PLAYER_O : PLAYER_X;
        }
    }

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }

            // Управление камерой с зацикливанием
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_w: cameraY = (cameraY - CELL_SIZE + MAX_SIZE * CELL_SIZE) % (MAX_SIZE * CELL_SIZE); break;
                    case SDLK_s: cameraY = (cameraY + CELL_SIZE) % (MAX_SIZE * CELL_SIZE); break;
                    case SDLK_a: cameraX = (cameraX - CELL_SIZE + MAX_SIZE * CELL_SIZE) % (MAX_SIZE * CELL_SIZE); break;
                    case SDLK_d: cameraX = (cameraX + CELL_SIZE) % (MAX_SIZE * CELL_SIZE); break;
                }
            }

            // Обработка хода игрока
            if (event.type == SDL_MOUSEBUTTONDOWN && !gameOver) {
                int x = (event.button.x + cameraX) / CELL_SIZE;
                int y = (event.button.y + cameraY) / CELL_SIZE;

                // Проверяем, что клетка пуста, и делаем ход
                if (board[y][x] == EMPTY) {
                    board[y][x] = currentPlayer;
                    emptyCells--;
                    checkGameState(x, y);

                    // Если игра продолжается и ход у компьютера
                    if (!gameOver && currentPlayer == PLAYER_O) {
                        aiMove(x, y);  // Передаём последний ход игрока
                        emptyCells--;
                        checkGameState(x, y); // Проверяем состояние игры после
                    }
                }
            }

            // Обработка кликов на кнопки в диалоговом окне
            if (gameOver && event.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX = event.button.x;
                int mouseY = event.button.y;

                SDL_Rect closeButton = {WINDOW_WIDTH / 4 + 10, WINDOW_HEIGHT / 3 + 70, 100, 40};
                SDL_Rect retryButton = {WINDOW_WIDTH / 4 + WINDOW_WIDTH / 2 - 110, WINDOW_HEIGHT / 3 + 70, 100, 40};

                if (isClickInsideRect(closeButton, mouseX, mouseY)) {
                    running = 0; // Закрыть игру
                } else if (isClickInsideRect(retryButton, mouseX, mouseY)) {
                    initBoard(&emptyCells); // Начать новую игру
                    gameOver = 0;
                    currentPlayer = PLAYER_X; // Сброс хода на игрока X
                    snprintf(message, sizeof(message), "");
                }
            }
        }

        // Отрисовка игрового поля
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        drawBoard(renderer);

        // Если игра завершена, отображаем сообщение
        if (gameOver) {
            drawMessageBox(renderer, message);
        }

        SDL_RenderPresent(renderer);
    }

    // Очистка ресурсов SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}