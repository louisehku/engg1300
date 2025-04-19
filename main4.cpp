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
    Ball(float x, float y, float radius, float speed)
        : GameObject(x, y, 1, 1), speed(speed), symbol(ACS_BULLET) {
        float angle = (rand() % 60 + 30) * 3.14159f / 180.0f;
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
            attron(COLOR_PAIR(1));
            mvaddch(currentY, currentX, symbol);
            attroff(COLOR_PAIR(1));
            lastDrawnX = currentX;
            lastDrawnY = currentY;
        } else {
            attron(COLOR_PAIR(1));
            mvaddch(currentY, currentX, symbol);
            attroff(COLOR_PAIR(1));
        }
    }

    void bounceX() { velocity.x = -velocity.x; }
    void bounceY() { velocity.y = -velocity.y; }
    Vector2D getVelocity() const { return velocity; }
    void setVelocity(Vector2D newVel) { velocity = newVel; }
};

// Paddle class
class Paddle : public GameObject {
private:
    float speed;

public:
    Paddle(float x, float y, float width, float height, float speed)
        : GameObject(x, y, width, height), speed(speed) {}

    void update(float deltaTime) override {}

    void draw() override {
        int currentX = static_cast<int>(round(position.x));
        int currentY = static_cast<int>(round(position.y));

        if (currentX != lastDrawnX || currentY != lastDrawnY) {
            clearPrevious();
            lastDrawnX = currentX;
            lastDrawnY = currentY;
        }

        attron(COLOR_PAIR(2));
        for (int x = 0; x < static_cast<int>(size.x); x++) {
            mvaddch(currentY, currentX + x, ACS_BLOCK);
        }
        attroff(COLOR_PAIR(2));
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

// Bullet class
class Bullet {
public:
    int x, y;
    Bullet(int startX, int startY) : x(startX), y(startY) {}
    void move() { y--; } // Move bullet upwards
};

// Enemy class
class Enemy {
public:
    int x, y;
    Enemy(int startX, int startY) : x(startX), y(startY) {}
};

// Game class to manage the game
class BreakoutGame {
private:
    Paddle* paddle;
    Ball* ball;
    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    int score;
    bool gameOver;

public:
    BreakoutGame(int startX, int startY, int width, int height) 
        : score(0), gameOver(false) {
        paddle = new Paddle(startX + (width / 2) - 5, startY + height - 2, 10, 1, 30);
        ball = new Ball(startX + (width / 2), startY + height / 2, 1.0f, 20.0f);
        setupEnemies();
    }

    ~BreakoutGame() {
        delete paddle;
        delete ball;
    }

    void setupEnemies() {
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 10; j++) {
                enemies.emplace_back(j * 6 + 5, i + 1); // Simple grid formation
            }
        }
    }

    void handleInput(int key, float deltaTime) {
        if (gameOver) return;

        if (key == KEY_LEFT) {
            paddle->moveLeft(deltaTime, 1);
        } else if (key == KEY_RIGHT) {
            paddle->moveRight(deltaTime, COLS - 1);
        } else if (key == ' ') {
            bullets.emplace_back(paddle->getPosition().x + 5, paddle->getPosition().y - 1); // Shoot bullet
        }
    }

    void update(float deltaTime) {
        if (gameOver) return;

        ball->update(deltaTime);

        // Check collisions with paddle
        if (ball->collidesWith(*paddle)) {
            ball->bounceY();
        }

        // Move bullets
        for (int i = 0; i < bullets.size(); i++) {
            bullets[i].move();
            // Check for collisions with enemies
            for (int j = 0; j < enemies.size(); j++) {
                if (bullets[i].x == enemies[j].x && bullets[i].y == enemies[j].y) {
                    bullets.erase(bullets.begin() + i);
                    enemies.erase(enemies.begin() + j);
                    score++;
                    i--; // Adjust index after removal
                    break; // Exit inner loop
                }
            }
            // Remove bullets that go off-screen
            if (bullets[i].y < 0) {
                bullets.erase(bullets.begin() + i);
                i--;
            }
        }

        // Update ball position and check for game over
        if (ball->getPosition().y >= LINES - 1) {
            gameOver = true; // Ball fell below the screen
        }
    }

    void draw() {
        clear();
        paddle->draw();
        ball->draw();

        // Draw bullets
        for (auto& bullet : bullets) {
            mvaddch(bullet.y, bullet.x, '|');
        }

        // Draw enemies
        for (auto& enemy : enemies) {
            mvaddch(enemy.y, enemy.x, '#');
        }

        mvprintw(0, 0, "Score: %d", score);
        if (gameOver) {
            mvprintw(LINES / 2, COLS / 2 - 5, "GAME OVER!");
        }
        refresh();
    }

    bool isGameOver() const { return gameOver; }
};

int main() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);

    BreakoutGame game(0, 0, COLS, LINES - 1);
    float lastTime = static_cast<float>(clock()) / CLOCKS_PER_SEC;

    while (!game.isGameOver()) {
        float currentTime = static_cast<float>(clock()) / CLOCKS_PER_SEC;
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        int ch;
        while ((ch = getch()) != ERR) {
            game.handleInput(ch, deltaTime);
        }

        game.update(deltaTime);
        game.draw();
        usleep(16667); // ~60 FPS
    }

    endwin();
    return 0;
}
