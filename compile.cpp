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

// Paddle class
class Paddle {
private:
    float x, y;           // Position with floating-point precision for smooth movement
    int lastDrawnX, lastDrawnY; // Last position where the paddle was drawn
    float directionX;     // Direction vector (only horizontal movement)
    float speed;          // Movement speed
    int width;            // Paddle width
    bool moving;          // Whether the paddle is moving

public:
    Paddle(int startX, int startY, int paddleWidth = 7) : 
        x(static_cast<float>(startX)), y(static_cast<float>(startY)), 
        lastDrawnX(startX), lastDrawnY(startY),
        directionX(0.0f), speed(0.5f), width(paddleWidth), moving(false) {}

    void update() {
        if (moving) {
            // Move in the current direction (horizontal only)
            x += directionX * speed;
        }
    }

    void setDirection(float dx) {
        directionX = dx;
        moving = (dx != 0.0f);  // Start moving when a direction is set
    }
    
    void stop() {
        moving = false;
    }

    void setPosition(float newX, float newY) {
        x = newX;
        y = newY;
    }

    void clearPrevious() {
        // Clear the previous position
        for (int i = 0; i < width; i++) {
            mvaddch(lastDrawnY, lastDrawnX + i, ' ');
        }
    }

    void draw() {
        int currentX = static_cast<int>(round(x));
        int currentY = static_cast<int>(round(y));
        
        // Only redraw if position has changed
        if (currentX != lastDrawnX || currentY != lastDrawnY) {
            clearPrevious();
            lastDrawnX = currentX;
            lastDrawnY = currentY;
        }
        
        // Draw paddle
        attron(COLOR_PAIR(1)); // Paddle color
        for (int i = 0; i < width; i++) {
            mvaddch(currentY, currentX + i, '=');
        }
        attroff(COLOR_PAIR(1));
    }

    float getX() const { return x; }
    float getY() const { return y; }
    int getWidth() const { return width; }
};

// Ball class
class Ball {
private:
    float x, y;           // Position with floating-point precision
    int lastDrawnX, lastDrawnY; // Last position where the ball was drawn
    float directionX, directionY; // Direction vector
    float speed;          // Movement speed
    bool active;          // Whether the ball is active (moving)

public:
    Ball(int startX, int startY) : 
        x(static_cast<float>(startX)), y(static_cast<float>(startY)), 
        lastDrawnX(startX), lastDrawnY(startY),
        directionX(0.7f), directionY(-0.7f),
        speed(0.4f), active(true) {}

    void update() {
        if (active) {
            x += directionX * speed;
            y += directionY * speed;
        }
    }

    void setDirection(float dx, float dy) {
        directionX = dx;
        directionY = dy;
        
        // Normalize the direction vector
        float length = sqrt(dx * dx + dy * dy);
        if (length > 0) {
            directionX /= length;
            directionY /= length;
        }
    }
    
    void reverseX() {
        directionX = -directionX;
    }
    
    void reverseY() {
        directionY = -directionY + ((rand() % 20) / 100.0f - 0.1f);
    }
    
    void setPosition(float newX, float newY) {
        x = newX;
        y = newY;
    }

    void clearPrevious() {
        mvaddch(lastDrawnY, lastDrawnX, ' ');
    }

    void draw() {
        int currentX = static_cast<int>(round(x));
        int currentY = static_cast<int>(round(y));
        
        if (currentX != lastDrawnX || currentY != lastDrawnY) {
            clearPrevious();
            lastDrawnX = currentX;
            lastDrawnY = currentY;
        }
        
        // Draw ball
        attron(COLOR_PAIR(2)); // Ball color
        mvaddch(currentY, currentX, 'O');
        attroff(COLOR_PAIR(2));
    }

    float getX() const { return x; }
    float getY() const { return y; }
};

// Block class
class Block {
private:
    int x, y;             // Position
    int width, height;    // Size
    bool active;          // Whether the block is active (not destroyed)
    int colorPair;        // Color pair to use for the block

public:
    Block(int startX, int startY, int w = 4, int h = 1, int color = 3) 
        : x(startX), y(startY), width(w), height(h), active(true), colorPair(color) {}

    void draw() {
        if (!active) return;

        attron(COLOR_PAIR(colorPair));
        for (int row = 0; row < height; ++row) {
            for (int col = 0; col < width; ++col) {
                mvaddch(y + row, x + col, ACS_CKBOARD);
            }
        }
        attroff(COLOR_PAIR(colorPair));
    }

    void clear() {
        for (int row = 0; row < height; ++row) {
            for (int col = 0; col < width; ++col) {
                mvaddch(y + row, x + col, ' ');
            }
        }
    }

    bool collidesWith(const Ball& ball) const {
        if (!active) return false;

        float ballX = ball.getX();
        float ballY = ball.getY();

        return (ballX >= x && ballX < x + width &&
                ballY >= y && ballY < y + height);
    }

    void setActive(bool isActive) {
        if (active && !isActive) {
            clear();
        }
        active = isActive;
    }

    bool isActive() const {
        return active;
    }

    int getX() const { return x; }
    int getY() const { return y; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
};

// BattleBox class
class BattleBox {
private:
    int x, y;         // Top-left corner position
    int width, height; // Box dimensions
    bool needsRedraw;  // Flag to determine if the box needs redrawing

public:
    BattleBox(int startX, int startY, int w, int h) :
        x(startX), y(startY), width(w), height(h), needsRedraw(true) {}

    void draw() {
        if (!needsRedraw) return;
        
        // Enable reverse highlighting
        attron(A_REVERSE);
    
        // Draw the borders of the battle box
        for (int i = -1; i <= width + 1; i++) {
            mvaddch(y, x + i, ' ');              // Top border
            mvaddch(y + height, x + i, ' ');     // Bottom border
        }
    
        for (int i = 0; i <= height; i++) {
            mvaddch(y + i, x, ' ');              // Left border
            mvaddch(y + i, x + width, ' ');      // Right border
        }
    
        // Disable reverse highlighting
        attroff(A_REVERSE);
        
        needsRedraw = false;
    }

    void setNeedsRedraw() {
        needsRedraw = true;
    }

    int getX() const { return x; }
    int getY() const { return y; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
};

// GameManager class
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
        battleBox(screenWidth / 2 - 20, screenHeight / 2 - 8, 40, 16),
        paddle(battleBox.getX() + (battleBox.getWidth() - 7) / 2, 
               battleBox.getY() + battleBox.getHeight() - 1),
        ball(battleBox.getX() + (battleBox.getWidth() / 2), 
             battleBox.getY() + (battleBox.getHeight() - 3)),
        blockCount(0),
        gameOver(false),
        gameWon(false) {
        
        // Initialize blocks
        initializeBlocks();
    }
    
    void initializeBlocks() {
        blocks.clear();
        blockCount = 0;
        
        int blockWidth = 5;
        int blockHeight = 1;
        int padding = 1;
        
        int boxWidth = battleBox.getWidth() - 2; // Accounting for border
        int boxX = battleBox.getX() + 2;
        int boxY = battleBox.getY() + 2;
        
        int blocksPerRow = (boxWidth + padding) / (blockWidth + padding);
        int maxRows = 5;
        
        for (int row = 0; row < maxRows; row++) {
            for (int col = 0; col < blocksPerRow; col++) {
                int blockX = boxX + col * (blockWidth + padding);
                int blockY = boxY + row * (blockHeight + padding);
                int blockColor = 3 + (row % 5);
                
                blocks.push_back(Block(blockX, blockY, blockWidth, blockHeight, blockColor));
                blockCount++;
            }
        }
    }
    
    void update() {
        // Place update logic here
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
    
    void handleInput(int key) {
        // Input handling logic
    }
    
    void reset() {
        // Reset game state logic
    }
    
    bool isGameOver() const {
        return gameOver;
    }
    
    bool isGameWon() const {
        return gameWon;
    }
};

int main() {
    // Initialize ncurses
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
