#include <ncursesw/ncurses.h>
#include <unistd.h>
#include <cmath>
#include <cstring>
#include <vector>

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
    Paddle(int startX, int startY, int paddleWidth = 7) : 
        x(static_cast<float>(startX)), y(static_cast<float>(startY)), 
        lastDrawnX(startX), lastDrawnY(startY),
        directionX(0.0f), speed(0.5f), width(paddleWidth), moving(false) {}

    void update() {
        if (moving) {
            x += directionX * speed;
        }
    }

    void setDirection(float dx) {
        directionX = dx;
        moving = dx != 0.0f;
    }

    void stop() {
        moving = false;
    }

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
        }
        
        attron(COLOR_PAIR(1));
        for (int i = 0; i < width; i++) {
            mvaddch(currentY, currentX + i, '=');
        }
        attroff(COLOR_PAIR(1));
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
    Ball(int startX, int startY) : 
        x(static_cast<float>(startX)), y(static_cast<float>(startY)), 
        lastDrawnX(startX), lastDrawnY(startY),
        directionX(0.7f), directionY(-0.7f), 
        speed(0.3f), active(true) {}

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
        }
        
        attron(COLOR_PAIR(2));
        mvaddch(currentY, currentX, 'O');
        attroff(COLOR_PAIR(2));
    }

    float getX() const { return x; }
    float getY() const { return y; }
};

class Block {
private:
    int x, y;             
    int width, height;    
    bool active;          
    int colorPair;        

public:
    Block(int startX, int startY, int w = 4, int h = 1, int color = 3) : 
        x(startX), y(startY), width(w), height(h), active(true), colorPair(color) {}

    void draw() {
        if (!active) return;
        attron(COLOR_PAIR(colorPair));
        for (int row = 0; row < height; row++) {
            for (int col = 0; col < width; col++) {
                mvaddch(y + row, x + col, ACS_CKBOARD); // Use '#' to represent blocks
            }
        }
        attroff(COLOR_PAIR(colorPair));
    }

    void clear() {
        for (int row = 0; row < height; row++) {
            for (int col = 0; col < width; col++) {
                mvaddch(y + row, x + col, ' ');
            }
        }
    }

    bool collidesWith(const Ball& ball) {
        if (!active) return false;
        float ballX = ball.getX();
        float ballY = ball.getY();
        return (ballX >= x && ballX < x + width && ballY >= y && ballY < y + height);
    }

    void setActive(bool isActive) {
        if (active && !isActive) clear();
        active = isActive;
    }

    bool isActive() const { return active; }
};

class BattleBox {
private:
    int x, y;         
    int width, height; 
    bool needsRedraw;  

public:
    BattleBox(int startX, int startY, int w, int h) :
        x(startX), y(startY), width(w), height(h), needsRedraw(true) {}

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
    GameManager(int screenWidth, int screenHeight) : 
        battleBox(screenWidth/2 - 20, screenHeight/2 - 15, 40, 30),
        paddle(screenWidth/2 - 3, screenHeight/2 + 10),
        ball(screenWidth/2, screenHeight/2 + 9),
        blockCount(0), gameOver(false), gameWon(false) {
        initializeBlocks();
    }

    void initializeBlocks() {
        blocks.clear();
        int blockWidth = 4, blockHeight = 1, padding = 1;
        int boxWidth = battleBox.getWidth() - 2;
        int boxX = battleBox.getX() + 1;
        int boxY = battleBox.getY() + 2;
        int blocksPerRow = (boxWidth + padding) / (blockWidth + padding);
        int maxRows = 5;

        for (int row = 0; row < maxRows; row++) {
            for (int col = 0; col < blocksPerRow; col++) {
                int blockX = boxX + col * (blockWidth + padding);
                int blockY = boxY + row * (blockHeight + padding);
                int blockColor = 3 + (row % 5);
                blocks.emplace_back(blockX, blockY, blockWidth, blockHeight, blockColor);
                blockCount++;
            }
        }
    }

    void update() {
        if (gameOver || gameWon) return;
        paddle.update();

        // Constrain paddle position
        float paddleX = paddle.getX();
        if (paddleX < battleBox.getX() + 1) {
            paddle.setPosition(static_cast<float>(battleBox.getX() + 1), paddle.getY());
        } else if (paddleX + paddle.getWidth() > battleBox.getX() + battleBox.getWidth()) {
            paddle.setPosition(static_cast<float>(battleBox.getX() + battleBox.getWidth() - paddle.getWidth()), paddle.getY());
        }

        ball.update();
        // Ball collision with walls
        float ballX = ball.getX();
        float ballY = ball.getY();
        if (ballX <= battleBox.getX() + 1 || ballX >= battleBox.getX() + battleBox.getWidth() - 1) {
            ball.reverseX();
        }
        if (ballY <= battleBox.getY() + 1) {
            ball.reverseY();
        }
        if (ballY >= battleBox.getY() + battleBox.getHeight() - 1) {
            gameOver = true;
            return;
        }

        // Ball collision with paddle
        if (ballY >= paddle.getY() - 1 && ballY <= paddle.getY() &&
            ballX >= paddle.getX() && ballX < paddle.getX() + paddle.getWidth()) {
            ball.reverseY();
            float hitPosition = (ballX - paddle.getX()) / paddle.getWidth();
            float newDirX = 2.0f * (hitPosition - 0.5f);
            ball.setDirection(newDirX, -abs(ball.getDirectionY()));
        }

        // Ball collision with blocks
        for (auto& block : blocks) {
            if (block.isActive() && block.collidesWith(ball)) {
                block.setActive(false);
                blockCount--;
                if (blockCount <= 0) {
                    gameWon = true;
                }
                break;
            }
        }
    }

    void draw() {
        battleBox.draw();
        for (auto& block : blocks) {
            block.draw();
        }
        paddle.draw();
        ball.draw();
        int maxY, maxX;
        getmaxyx(stdscr, maxY, maxX);
        mvprintw(maxY - 3, 2, "Left/Right arrows to move paddle. Blocks remaining: %d", blockCount);
        mvprintw(maxY - 2, 2, "Space to stop/restart. Q to quit.");
        if (gameOver) {
            attron(COLOR_PAIR(1));
            mvprintw(maxY / 2, maxX / 2 - 5, "GAME OVER");
            attroff(COLOR_PAIR(1));
        } else if (gameWon) {
            attron(COLOR_PAIR(3));
            mvprintw(maxY / 2, maxX / 2 - 9, "YOU WIN! ALL BLOCKS CLEARED");
            attroff(COLOR_PAIR(3));
        }
    }

    void handleInput(int key) {
        if (gameOver || gameWon) {
            if (key == ' ') reset();
            return;
        }
        if (key == KEY_LEFT) {
            paddle.setDirection(-1.0f);
            paddle.start();
        } else if (key == KEY_RIGHT) {
            paddle.setDirection(1.0f);
            paddle.start();
        } else if (key == ' ') {
            paddle.stop();
        }
    }

    void reset() {
        gameOver = false;
        gameWon = false;
        int maxY, maxX;
        getmaxyx(stdscr, maxY, maxX);
        paddle.setPosition(maxX / 2 - 3, maxY / 2 + 10);
        ball.setPosition(maxX / 2, maxY / 2 + 9);
        ball.setDirection(0.7f, -0.7f);
        initializeBlocks();
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
        usleep(16667);  // ~60 FPS
    }

    endwin();
    return 0;
}
