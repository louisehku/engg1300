#include <ncursesw/ncurses.h>
#include <unistd.h>
#include <cmath>
#include <cstring>
#include <vector>
#include <algorithm>

// Forward declarations
class Ball;
class Paddle;
class Block;
class GameManager;

class Paddle {
private:
    float x, y;
    int lastDrawnX, lastDrawnY;
    float directionX;
    float speed;
    int width;
    bool moving;

public:
    Paddle(int startX, int startY, int paddleWidth = 7)
        : x(static_cast<float>(startX)), y(static_cast<float>(startY)),
          lastDrawnX(startX), lastDrawnY(startY),
          directionX(0.0f), speed(0.5f), width(paddleWidth), moving(false) {}

    void update() {
        if (moving) {
            x += directionX * speed;
        }
    }

    void setDirection(float dx) {
        directionX = dx;
        moving = (dx != 0.0f);
    }

    void stop() { moving = false; }
    void start() { moving = true; }

    void clearPrevious() {
        for (int i = 0; i < width; i++) {
            mvaddch(lastDrawnY, lastDrawnX + i, ' ');
        }
    }

    void draw() {
        int currentX = static_cast<int>(round(x));
        int currentY = static_cast<int>(round(y));

        if (currentX != lastDrawnX || currentY != lastDrawnY) {
            clearPrevious();
            lastDrawnX = currentX;
            lastDrawnY = currentY;

            attron(COLOR_PAIR(1));
            for (int i = 0; i < width; i++) {
                mvaddch(currentY, currentX + i, '=');
            }
            attroff(COLOR_PAIR(1));
        }
    }
    
    void setPosition(float newX, float newY) {
        x = newX;
        y = newY;
    }

    float getX() const { return x; }
    float getY() const { return y; }
    int getWidth() const { return width; }
};

class Ball {
private:
    float x, y;
    int lastDrawnX, lastDrawnY;
    float directionX, directionY;
    float speed;
    bool active;

public:
    Ball(int startX, int startY)
        : x(static_cast<float>(startX)), y(static_cast<float>(startY)),
          lastDrawnX(startX), lastDrawnY(startY),
          directionX(0.7f), directionY(-0.7f), speed(0.4f), active(true) {}

    void update() {
        if (active) {
            x += directionX * speed;
            y += directionY * speed;
        }
    }

    void setDirection(float dx, float dy) {
        directionX = dx;
        directionY = dy;

        float length = sqrt(dx * dx + dy * dy);
        if (length > 0) {
            directionX /= length;
            directionY /= length;
        }
    }

    void reverseX() { directionX = -directionX; }
    void reverseY() { directionY = -directionY; }

    void draw() {
        int currentX = static_cast<int>(round(x));
        int currentY = static_cast<int>(round(y));

        if (currentX != lastDrawnX || currentY != lastDrawnY) {
            mvaddch(lastDrawnY, lastDrawnX, ' ');

            lastDrawnX = currentX;
            lastDrawnY = currentY;

            attron(COLOR_PAIR(2));
            mvaddch(currentY, currentX, 'O');
            attroff(COLOR_PAIR(2));
        }
    }

    float getX() const { return x; }
    float getY() const { return y; }
};

class Block {
private:
    int x, y;
    int width, height;
    bool active;

public:
    Block(int startX, int startY, int w = 4, int h = 1)
        : x(startX), y(startY), width(w), height(h), active(true) {}

    void draw() {
        if (!active) return;

        attron(A_REVERSE);
        for (int i = 0; i <= width; i++) {
            mvaddch(y, x + i, ' ');
            mvaddch(y + height, x + i, ' ');
        }
        for (int i = 0; i < height; i++) {
            mvaddch(y + i + 1, x, ' ');
            mvaddch(y + i + 1, x + width, ' ');
        }
        attroff(A_REVERSE);
    }

    bool collidesWith(const Ball& ball) const {
        if (!active) return false;
        float ballX = ball.getX();
        float ballY = ball.getY();
        return (ballX >= x && ballX < x + width &&
                ballY >= y && ballY < y + height);
    }

    void setActive(bool isActive) { active = isActive; }
    bool isActive() const { return active; }
};

class BattleBox {
private:
    int x, y;
    int width, height;
    bool needsRedraw;

public:
    BattleBox(int startX, int startY, int w, int h)
        : x(startX), y(startY), width(w), height(h), needsRedraw(true) {}

    void draw() {
        if (!needsRedraw) return;

        attron(A_REVERSE);
        for (int i = -1; i <= width + 1; i++) {
            mvaddch(y, x + i, ' ');
            mvaddch(y + height, x + i, ' ');
        }
        for (int i = 0; i <= height; i++) {
            mvaddch(y + i, x, ' ');
            mvaddch(y + i, x + width, ' ');
        }
        attroff(A_REVERSE);
        needsRedraw = false;
    }

    void setNeedsRedraw() { needsRedraw = true; }
    int getX() const { return x; }
    int getY() const { return y; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
};

class GameManager {
private:
    BattleBox battleBox;
    Paddle paddle;
    Ball ball;
    std::vector<Block> blocks;
    int blockCount;
    bool gameOver;
    bool gameWon;

public:
    GameManager(int screenWidth, int screenHeight) 
        : battleBox(screenWidth / 2 - 20, screenHeight / 2 - 15, 40, 30),
          paddle(screenWidth / 2 - 3, screenHeight / 2 + 14),
          ball(screenWidth / 2, screenHeight / 2 + 13),
          blockCount(0), gameOver(false), gameWon(false) {
        initializeBlocks();
    }

    void initializeBlocks() {
        blocks.clear();
        blockCount = 0;

        int blockWidth = 4;
        int blockHeight = 1;
        int padding = 1;
        
        int boxWidth = battleBox.getWidth() - 2;
        int boxX = battleBox.getX() + 2;
        int boxY = battleBox.getY() + 2;
        
        int blocksPerRow = (boxWidth + padding) / (blockWidth + padding);
        int maxRows = 5;

        for (int row = 0; row < maxRows; row++) {
            for (int col = 0; col < blocksPerRow; col++) {
                blocks.emplace_back(boxX + col * (blockWidth + padding), 
                                    boxY + row * (blockHeight + padding), 
                                    blockWidth, blockHeight);
                blockCount++;
            }
        }
    }

    void update() {
        if (gameOver || gameWon) return;

        paddle.update();
        constrainPaddle();

        ball.update();
        handleBallCollisions();
    }

    void draw() {
        battleBox.draw();
        for (auto& block : blocks) {
            block.draw();
        }
        paddle.draw();
        ball.draw();
        drawGameState();
    }

    void handleInput(int key) {
        if (gameOver || gameWon) {
            if (key == '\n') reset();
            return;
        }

        if (key == KEY_LEFT) {
            paddle.setDirection(-1.0f);
            paddle.start();
        } else if (key == KEY_RIGHT) {
            paddle.setDirection(1.0f);
            paddle.start();
        } else if (key == '\n') {
            paddle.stop();
        }
    }

    void reset() {
        gameOver = gameWon = false;
        int maxY, maxX;
        getmaxyx(stdscr, maxY, maxX);
        paddle.setPosition(maxX / 2 - 3, maxY / 2 + 14);
        ball.setPosition(maxX / 2, maxY / 2 + 13);
        ball.setDirection(0.7f, -0.7f);
        initializeBlocks();
    }

private:
    void constrainPaddle() {
        float paddleX = paddle.getX();
        float paddleY = paddle.getY();
        int boxX = battleBox.getX();
        int boxWidth = battleBox.getWidth();

        if (paddleX < boxX + 1) {
            paddle.setPosition(static_cast<float>(boxX + 1), paddleY);
        } else if (paddleX + paddle.getWidth() > boxX + boxWidth) {
            paddle.setPosition(static_cast<float>(boxX + boxWidth - paddle.getWidth()), paddleY);
        }
    }

    void handleBallCollisions() {
        float ballX = ball.getX();
        float ballY = ball.getY();
        int boxX = battleBox.getX();
        int boxY = battleBox.getY();
        int boxWidth = battleBox.getWidth();
        int boxHeight = battleBox.getHeight();

        if (ballX <= boxX + 1 || ballX >= boxX + boxWidth - 1) {
            ball.reverseX();
        }
        if (ballY <= boxY + 1) {
            ball.reverseY();
        }
        if (ballY >= boxY + boxHeight - 1) {
            gameOver = true;
            return;
        }
        handlePaddleCollision();
        handleBlockCollisions();
    }

    void handlePaddleCollision() {
        if (ball.getY() > paddle.getY() - 1 && ball.getY() < paddle.getY() &&
            ball.getX() >= paddle.getX() && ball.getX() < paddle.getX() + paddle.getWidth() - 1) {
            ball.reverseY();
            float hitPosition = (ball.getX() - paddle.getX()) / paddle.getWidth();
            float newDirX = 2.0f * (hitPosition - 0.5f);
            newDirX = std::clamp(newDirX, -0.8f, 0.8f);
            ball.setDirection(newDirX, -std::abs(ball.getDirectionY()));
        }
    }

    void handleBlockCollisions() {
        for (auto& block : blocks) {
            if (block.isActive() && block.collidesWith(ball)) {
                block.setActive(false);
                blockCount--;

                if (blockCount <= 0) {
                    gameWon = true;
                }

                ball.reverseY(); // Simple response for demonstration
                break; // Only handle one collision per update
            }
        }
    }

    void drawGameState() {
        int maxY, maxX;
        getmaxyx(stdscr, maxY, maxX);
        mvprintw(maxY - 3, 2, "Left/Right arrows to move paddle    Blocks remaining: %d", blockCount);
        mvprintw(maxY - 2, 2, "Space to stop/restart    Q to quit");

        if (gameOver) {
            attron(COLOR_PAIR(1));
            mvprintw(maxY / 2, maxX / 2 - 5, "GAME OVER");
            mvprintw(maxY / 2 + 1, maxX / 2 - 11, "Press ENTER to restart");
            attroff(COLOR_PAIR(1));
        } else if (gameWon) {
            attron(COLOR_PAIR(3));
            mvprintw(maxY / 2, maxX / 2 - 9, "YOU WIN! ALL BLOCKS CLEARED");
            mvprintw(maxY / 2 + 1, maxX / 2 - 11, "Press ENTER to restart");
            attroff(COLOR_PAIR(3));
        }
    }
};

int main() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    nodelay(stdscr, TRUE);

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_RED, COLOR_BLACK);
        init_pair(2, COLOR_CYAN, COLOR_BLACK);
        init_pair(3, COLOR_GREEN, COLOR_BLACK);
    }

    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    GameManager game(maxX, maxY);

    bool running = true;
    while (running) {
        int ch;
        while ((ch = getch()) != ERR) {
            if (ch == 'q' || ch == 'Q') {
                running = false;
                break;
            } else {
                game.handleInput(ch);
            }
        }

        game.update();
        game.draw();
        refresh();
        usleep(16667); // ~60 FPS
    }

    endwin();
    return 0;
}
