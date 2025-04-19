#include <ncursesw/ncurses.h>
#include <unistd.h>
#include <cmath>
#include <cstring>
#include <vector>
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
    
public:
    GameObject(float x, float y, float width, float height) 
        : position(x, y), size(width, height), active(true) {}
    
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
        float angle = (rand() % 60 + 30) * M_PI / 180.0f;  // Random angle
        velocity = Vector2D(cos(angle), -sin(angle)) * speed;
    }
    
    void update(float deltaTime) override {
        position.x += velocity.x * deltaTime;
        position.y += velocity.y * deltaTime;
    }
    
    void draw() override {
        int currentX = static_cast<int>(round(position.x));
        int currentY = static_cast<int>(round(position.y));
        mvaddch(currentY, currentX, symbol);
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
    Paddle(float x, float y, float width, float height, float speed)
        : GameObject(x, y, width, height), speed(speed) {}
    
    void update(float deltaTime) override {}
    
    void draw() override {
        for (int x = 0; x < static_cast<int>(size.x); x++) {
            mvaddch(static_cast<int>(position.y), static_cast<int>(position.x) + x, ACS_BLOCK);
        }
    }
    
    void moveLeft(float deltaTime, float minX) {
        position.x -= speed * deltaTime;
        if (position.x < minX) position.x = minX;
    }
    
    void moveRight(float deltaTime, float maxX) {
        position.x += speed * deltaTime;
        if (position.x + size.x > maxX) position.x = maxX - size.x;
    }
};

// Block class
class Block : public GameObject {
public:
    Block(float x, float y, float width, float height)
        : GameObject(x, y, width, height) {}
    
    void update(float deltaTime) override {}
    
    void draw() override {
        for (int y = 0; y < static_cast<int>(size.y); y++) {
            for (int x = 0; x < static_cast<int>(size.x); x++) {
                mvaddch(static_cast<int>(position.y) + y, static_cast<int>(position.x) + x, ACS_CKBOARD);
            }
        }
    }
};

// Game class to manage the game
class BreakoutGame {
private:
    Paddle* paddle;
    Ball* ball;
    std::vector<Block*> blocks;
    int score;
    
public:
    BreakoutGame(float paddleX, float paddleY, float paddleWidth, float ballSpeed)
        : score(0) {
        paddle = new Paddle(paddleX, paddleY, paddleWidth, 1, 30.0f);
        ball = new Ball(paddleX + paddleWidth / 2, paddleY - 1, ballSpeed);
        setupBlocks();
    }
    
    ~BreakoutGame() {
        delete paddle;
        delete ball;
        for (auto block : blocks) delete block;
    }
    
    void setupBlocks() {
        for (int i = 0; i < 5; i++) {
            blocks.push_back(new Block(10, 3 + i * 2, 5, 1));
        }
    }
    
    void handleInput(int key, float deltaTime) {
        if (key == KEY_LEFT) {
            paddle->moveLeft(deltaTime, 0);
        } else if (key == KEY_RIGHT) {
            paddle->moveRight(deltaTime, COLS);
        }
    }
    
    void update(float deltaTime) {
        ball->update(deltaTime);
        
        // Check collision with paddle
        if (ball->collidesWith(*paddle)) {
            ball->bounceY();
            ball->setVelocity(Vector2D(ball->getVelocity().x, -ball->getVelocity().y));
        }
        
        // Check collision with blocks
        for (auto block : blocks) {
            if (block->isActive() && ball->collidesWith(*block)) {
                ball->bounceY();
                block->setActive(false);
                score += 100; // Increase score for hitting a block
            }
        }
    }
    
    void render() {
        paddle->draw();
        ball->draw();
        for (auto block : blocks) {
            if (block->isActive()) {
                block->draw();
            }
        }
        mvprintw(0, 0, "Score: %d", score);
    }
};

int main() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    
    BreakoutGame game(COLS / 2 - 5, LINES - 3, 10, 0.5f);
    
    while (true) {
        int ch = getch();
        game.handleInput(ch, 1.0f / 60.0f); // Assume a constant delta time for simplicity
        game.update(1.0f / 60.0f);
        clear();
        game.render();
        refresh();
        usleep(16667); // ~60 FPS
    }
    
    endwin();
    return 0;
}
