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
        // Clear the previous position
        for (int y = 0; y < static_cast<int>(size.y); y++) {
            for (int x = 0; x < static_cast<int>(size.x); x++) {
                mvaddch(lastDrawnY + y, lastDrawnX + x, ' ');
            }
        }
    }
    
    virtual void update(float deltaTime) = 0;
    virtual void draw() = 0;
};

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
        // Set a new direction vector (horizontal only)
        directionX = dx;
        if (dx != 0.0f) {
            moving = true;  // Start moving when a direction is set
        }
    }
    
    void setSpeed(float newSpeed) {
        speed = newSpeed;
    }
    
    void stop() {
        moving = false;
    }
    
    void start() {
        moving = true;
    }
    
    bool isMoving() const {
        return moving;
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
            // Clear previous position if it's different
            clearPrevious();
            
            // Update last drawn position
            lastDrawnX = currentX;
            lastDrawnY = currentY;
        }
        
        // Draw paddle (a line of characters)
        attron(COLOR_PAIR(1)); // Paddle color
        for (int i = 0; i < width; i++) {
            mvaddch(currentY, currentX + i, '=');
        }
        attroff(COLOR_PAIR(1));
    }

    float getX() const { return x; }
    float getY() const { return y; }
    int getWidth() const { return width; }
    float getDirectionX() const { return directionX; }
    float getSpeed() const { return speed; }
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
    
    void update(float deltaTime) override {
        // Blocks don't move, so nothing to update
    }
    
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
        // Change color based on remaining hit points
        colorPair = 3 + (3 - hitPoints);
        return false;
    }
    
    int getScore() const { return score; }
};

class Bullet {
public:
    int x, y;
    Bullet(int startX, int startY) : x(startX), y(startY) {}
    void move() { y--; } // Move bullet upwards
};

class Enemy {
public:
    int x, y;
    Enemy(int startX, int startY) : x(startX), y(startY) {}
};

class Player {
public:
    int x, y;
    Player(int startX, int startY) : x(startX), y(startY) {}
    void move(int dx) { x += dx; }
};

void drawPlayer(const Player& player) {
    mvaddch(player.y, player.x, ACS_CKBOARD); // Player representation
}

void drawBullet(const Bullet& bullet) {
    mvaddch(bullet.y, bullet.x, '|'); // Bullet representation
}

void drawEnemy(const Enemy& enemy) {
    mvaddch(enemy.y, enemy.x, '#'); // Enemy representation
}

bool checkCollision(const Bullet& bullet, const Enemy& enemy) {
    return bullet.x == enemy.x && bullet.y == enemy.y;
}

class Game {
private:
    Player player;
    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    int score;
    int boxX, boxY; // Battle box position

public:
    Game(int startX, int startY) : player(startX + 20, startY + 14), score(0), boxX(startX), boxY(startY) {
        // Create enemies
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 10; j++) {
                enemies.emplace_back(startX + j * 6 + 5, startY + i + 1); // Simple grid formation
            }
        }
    }

    void update() {
        for (int i = 0; i < bullets.size(); i++) {
            bullets[i].move();
            for (int j = 0; j < enemies.size(); j++) {
                if (checkCollision(bullets[i], enemies[j])) {
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
        drawPlayer(player);
        for (auto& bullet : bullets) {
            drawBullet(bullet);
        }
        for (auto& enemy : enemies) {
            drawEnemy(enemy);
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
                bullets.emplace_back(player.x, player.y - 1); // Shoot bullet
                break;
            case 'q':
                endwin();
                exit(0);
        }
    }
};

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
    
        // Draw the top and bottom borders of the battle box
        for (int i = -1; i <= width; i++) {
            mvaddch(y, x + i, ' ');              // Top border
            mvaddch(y + height, x + i, ' ');     // Bottom border
        }
    
        // Draw the left and right borders of the battle box
        for (int i = 0; i <= height; i++) {
            mvaddch(y + i, x, ' ');              // Left border
            mvaddch(y + i, x + width, ' ');      // Right border
        }
    
        // Disable reverse highlighting
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

    // Create battle box
    BattleBox battleBox(maxX/2 - 20, maxY/2 - 8, 40, 16);
    battleBox.draw();

    Game game(battleBox.getX(), battleBox.getY());

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
