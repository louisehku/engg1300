#include <iostream>
#include <vector>
#include <ncursesw/ncurses.h>
#include <unistd.h>

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

    // Check collision between objects
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

// Paddle class
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
};

// Block class
class Block : public GameObject {
private:
    int hitPoints;
    int score;
    int colorPair;

public:
    Block(float x, float y, float width, float height, int hitPoints = 1, int score = 100, int colorPair = 3)
        : GameObject(x, y, width, height), hitPoints(hitPoints), score(score), colorPair(colorPair) {}

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
            clearPrevious();
            return true;
        }
        colorPair = 3 + (3 - hitPoints);
        return false;
    }

    int getScore() const { return score; }
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

// Player class
class Player {
public:
    int x, y;

    Player(int startX, int startY) : x(startX), y(startY) {}
    void move(int dx) { x += dx; }
};

// Game class
class Game {
private:
    Player player;
    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    int score;
    int boxX, boxY;

public:
    Game(int startX, int startY) : player(startX + 20, startY + 14), score(0), boxX(startX), boxY(startY) {
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 10; j++) {
                enemies.emplace_back(startX + j * 6 + 5, startY + i + 1);
            }
        }
    }

    void update() {
        for (int i = 0; i < bullets.size(); i++) {
            bullets[i].move();
            for (int j = 0; j < enemies.size(); j++) {
                if (bullets[i].x == enemies[j].x && bullets[i].y == enemies[j].y) {
                    bullets.erase(bullets.begin() + i);
                    enemies.erase(enemies.begin() + j);
                    score++;
                    i--;
                    break;
                }
            }
        }
    }

    void draw() {
        clear();
        mvaddch(player.y, player.x, ACS_CKBOARD);
        for (const auto& bullet : bullets) {
            mvaddch(bullet.y, bullet.x, '|');
        }
        for (const auto& enemy : enemies) {
            mvaddch(enemy.y, enemy.x, '#');
        }
        mvprintw(0, 0, "Score: %d", score);
        refresh();
    }

    void handleInput(int ch) {
        switch (ch) {
            case KEY_LEFT:
                if (player.x > boxX) player.move(-1);
                break;
            case KEY_RIGHT:
                if (player.x < boxX + 39) player.move(1);
                break;
            case ' ':
                bullets.emplace_back(player.x, player.y - 1);
                break;
            case 'q':
                endwin();
                exit(0);
        }
    }
};

// BattleBox class
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
        for (int i = -1; i <= width; i++) {
            mvaddch(y, x + i, ' ');              // Top border
            mvaddch(y + height, x + i, ' ');     // Bottom border
        }
        for (int i = 0; i <= height; i++) {
            mvaddch(y + i, x, ' ');              // Left border
            mvaddch(y + i, x + width, ' ');      // Right border
        }
        attroff(A_REVERSE);
        needsRedraw = false;
    }

    int getX() const { return x; }
    int getY() const { return y; }
};

int main() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);

    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);

    BattleBox battleBox(maxX / 2 - 20, maxY / 2 - 8, 40, 16);
    battleBox.draw();

    Game game(battleBox.getX(), battleBox.getY());

    // Set up colors if terminal supports them
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_RED, COLOR_BLACK);    // Paddle color
        init_pair(2, COLOR_WHITE, COLOR_BLUE);   // Paddle
        init_pair(3, COLOR_BLACK, COLOR_RED);    // Strong blocks
        init_pair(4, COLOR_BLACK, COLOR_YELLOW); // Medium blocks
        init_pair(5, COLOR_BLACK, COLOR_GREEN);  // Weak blocks
        init_pair(6, COLOR_BLACK, COLOR_CYAN);   // One-hit blocks
    }

    while (true) {
        int ch = getch();
        game.handleInput(ch);
        game.update();
        game.draw();
        usleep(100000); // Control game speed
    }

    endwin();
    return 0;
}
