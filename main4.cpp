#include <iostream>
#include <vector>
#include <ncursesw/ncurses.h>
#include <unistd.h>
#include <cmath>
#include <ctime>
#include <cstdlib>

// Vector2D class for positions and velocities
class Vector2D {
public:
    float x, y;
    
    Vector2D(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}
    
    Vector2D operator+(const Vector2D& other) const {
        return Vector2D(x + other.x, y + other.y);
    }
    
    Vector2D operator*(float scalar) const {
        return Vector2D(x * scalar, y * scalar);
    }
};

// Game object base class
class GameObject {
protected:
    Vector2D position;
    Vector2D size;
    bool active;
    int lastDrawnX, lastDrawnY;
    
public:
    GameObject(float x, float y, float width, float height) 
        : position(x, y), size(width, height), active(true),
          lastDrawnX(static_cast<int>(round(x))), lastDrawnY(static_cast<int>(round(y))) {}
    
    virtual ~GameObject() {}

    bool isActive() const { return active; }
    void setActive(bool state) { active = state; }
    
    Vector2D getPosition() const { return position; }
    Vector2D getSize() const { return size; }
    
    bool collidesWith(const GameObject& other) const {
        return (position.x < other.position.x + other.size.x &&
                position.x + size.x > other.position.x &&
                position.y < other.position.y + other.size.y &&
                position.y + size.y > other.position.y);
    }
    
    void clearPrevious() {
        for (int y = 0; y < static_cast<int>(size.y); y++) {
            for (int x = 0; x < static_cast<int>(size.x); x++) {
                mvaddch(lastDrawnY + y, lastDrawnX + x, ' ');
            }
        }
    }
    
    virtual void update(float deltaTime) = 0;
    virtual void draw() = 0;
};

// Ball class
class Ball : public GameObject {
private:
    Vector2D velocity;
    float speed;
    int symbol;
    
public:
    Ball(float x, float y, float speed)
        : GameObject(x, y, 1, 1), speed(speed), symbol(ACS_BULLET) {
        float angle = (rand() % 60 + 30) * M_PI / 180.0f; // Random angle
        velocity = Vector2D(cos(angle), -sin(angle)) * speed;
    }
    
    void update(float deltaTime) override {
        position.x += velocity.x * deltaTime;
        position.y += velocity.y * deltaTime;
    }
    
    void draw() override {
        int currentX = static_cast<int>(round(position.x));
        int currentY = static_cast<int>(round(position.y));
        
        if (currentX != lastDrawnX || currentY != lastDrawnY) {
            clearPrevious();
            attron(COLOR_PAIR(1)); // Ball color
            mvaddch(currentY, currentX, symbol);
            attroff(COLOR_PAIR(1));
            lastDrawnX = currentX;
            lastDrawnY = currentY;
        }
    }
    
    void bounceX() { velocity.x = -velocity.x; }
    void bounceY() { velocity.y = -velocity.y; }
    
    Vector2D getVelocity() const { return velocity; }
    void setVelocity(Vector2D newVel) { velocity = newVel; }
};

// Paddle class (the player controlled board)
class Paddle : public GameObject {
private:
    float speed;
    
public:
    Paddle(float x, float y, float width, float speed)
        : GameObject(x, y, width, 1), speed(speed) {}
    
    void update(float deltaTime) override {}
    
    void draw() override {
        int currentX = static_cast<int>(round(position.x));
        int currentY = static_cast<int>(round(position.y));
        
        if (currentX != lastDrawnX || currentY != lastDrawnY) {
            clearPrevious();
            attron(COLOR_PAIR(2)); // Paddle color
            for (int x = 0; x < static_cast<int>(size.x); x++) {
                mvaddch(currentY, currentX + x, ACS_BLOCK);
            }
            attroff(COLOR_PAIR(2));
            lastDrawnX = currentX;
            lastDrawnY = currentY;
        }
    }
    
    void moveLeft(float deltaTime, float minX) {
        position.x -= speed * deltaTime;
        if (position.x < minX) {
            position.x = minX;
        }
    }
    
    void moveRight(float deltaTime, float maxX) {
        position.x += speed * deltaTime;
        if (position.x + size.x > maxX) {
            position.x = maxX - size.x;
        }
    }
};

// Block class
class Block : public GameObject {
private:
    int hitPoints;
    int score;
    int colorPair;
    
public:
    Block(float x, float y, float hitPoints, int score, int colorPair)
        : GameObject(x, y, 5, 2), hitPoints(hitPoints), score(score), colorPair(colorPair) {}
    
    void update(float deltaTime) override {}
    
    void draw() override {
        if (!active) return;
        
        int currentX = static_cast<int>(round(position.x));
        int currentY = static_cast<int>(round(position.y));
        
        attron(COLOR_PAIR(colorPair));
        for (int y = 0; y < static_cast<int>(size.y); y++) {
            for (int x = 0; x < static_cast<int>(size.x); x++) {
                mvaddch(currentY + y, currentX + x, ACS_CKBOARD);
            }
        }
        attroff(COLOR_PAIR(colorPair));
    }
    
    bool hit() {
        hitPoints--;
        if (hitPoints <= 0) {
            active = false;
            clearPrevious(); // Clear when destroyed
            return true;
        }
        return false;
    }
    
    int getScore() const { return score; }
};

// BattleBox class (game area)
class BattleBox {
private:
    int x, y;         // Top-left corner position
    int width, height; // Box dimensions
    bool needsRedraw;

public:
    BattleBox(int startX, int startY, int w, int h)
        : x(startX), y(startY), width(w), height(h), needsRedraw(true) {}

    void draw() {
        if (!needsRedraw) return;
        
        attron(A_REVERSE);
        for (int i = -1; i <= width + 1; i++) {
            mvaddch(y, x + i, ' '); // Top border
            mvaddch(y + height, x + i, ' '); // Bottom border
        }
        for (int i = 0; i <= height; i++) {
            mvaddch(y + i, x, ' '); // Left border
            mvaddch(y + i, x + width, ' '); // Right border
        }
        attroff(A_REVERSE);
        
        needsRedraw = false;
    }

    int getX() const { return x; }
    int getY() const { return y; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
};

// Game class to manage the game
class BreakoutGame {
private:
    BattleBox* gameArea;
    Ball* ball;
    Paddle* paddle;
    std::vector<Block*> blocks;
    int score;
    bool gameOver;

public:
    BreakoutGame(int startX, int startY, int width, int height)
        : score(0), gameOver(false) {
        
        // Create game area
        gameArea = new BattleBox(startX, startY, width, height);
        
        // Create ball
        ball = new Ball(startX + width / 2, startY + height / 2, 5.0f);
        
        // Create paddle
        paddle = new Paddle(startX + (width - 10) / 2, startY + height - 1, 10.0f, 20.0f);
        
        // Create blocks
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 10; j++) {
                blocks.push_back(new Block(startX + j * 6 + 2, startY + i * 3 + 1, 1, 100, 3));
            }
        }
    }
    
    ~BreakoutGame() {
        delete gameArea;
        delete ball;
        delete paddle;
        
        for (auto block : blocks) {
            delete block;
        }
    }
    
    void handleInput(int key, float deltaTime) {
        if (gameOver) return;
        
        if (key == KEY_LEFT) {
            paddle->moveLeft(deltaTime, gameArea->getX() + 1);
        } else if (key == KEY_RIGHT) {
            paddle->moveRight(deltaTime, gameArea->getX() + gameArea->getWidth() - 1);
        }
    }
    
    void update(float deltaTime) {
        if (gameOver) return;
        
        ball->update(deltaTime);
        
        // Check wall collisions
        Vector2D ballPos = ball->getPosition();
        Vector2D ballSize = ball->getSize();
        
        if (ballPos.x <= gameArea->getX() + 1 || ballPos.x + ballSize.x >= gameArea->getX() + gameArea->getWidth() - 1) {
            ball->bounceX();
        }
        
        if (ballPos.y <= gameArea->getY() + 1) {
            ball->bounceY();
        }
        
        if (ballPos.y + ballSize.y >= gameArea->getY() + gameArea->getHeight() - 1) {
            gameOver = true;
        }
        
        // Paddle collision
        if (ball->collidesWith(*paddle)) {
            ball->bounceY();
        }
        
        // Block collisions
        for (auto block : blocks) {
            if (block->isActive() && ball->collidesWith(*block)) {
                ball->bounceY();
                if (block->hit()) {
                    score += block->getScore();
                }
                break; // Only handle one collision per update
            }
        }
    }
    
    void render() {
        gameArea->draw();
        for (auto block : blocks) {
            if (block->isActive()) {
                block->draw();
            }
        }
        paddle->draw();
        ball->draw();
        
        mvprintw(gameArea->getY() + gameArea->getHeight() + 1, gameArea->getX(), "Score: %d", score);
        
        if (gameOver) {
            mvprintw(gameArea->getY() + gameArea->getHeight() / 2, gameArea->getX() + gameArea->getWidth() / 2 - 5, "GAME OVER!");
        }
    }
    
    bool isGameOver() const { return gameOver; }
};

int main() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);  // Hide cursor
    nodelay(stdscr, TRUE);
    
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_RED, COLOR_BLACK);    // Ball
        init_pair(2, COLOR_WHITE, COLOR_BLUE);   // Paddle
        init_pair(3, COLOR_BLACK, COLOR_GREEN);   // Block
    }

    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);

    BreakoutGame game(maxX / 2 - 30, maxY / 2 - 10, 60, 20);
    
    while (!game.isGameOver()) {
        int ch = getch();
        game.handleInput(ch, 1.0f); // Simple delta time
        game.update(1.0f); // Update game logic
        game.render(); // Render everything
        usleep(16667); // ~60 FPS
    }
    
    endwin();
    return 0;
}
