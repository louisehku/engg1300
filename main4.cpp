#include <iostream>
#include <vector>
#include <ncursesw/ncurses.h>
#include <unistd.h>
#include <cmath>

class GameObject {
protected:
    struct Position {
        float x, y;
    } position;

    struct Size {
        float x, y;
    } size;

    bool active = true;

public:
    GameObject(float x, float y, float width, float height) {
        position = {x, y};
        size = {width, height};
    }

    virtual void update(float deltaTime) = 0;
    virtual void draw() = 0;

    bool isActive() const { return active; }
    void deactivate() { active = false; }
};

class Paddle : public GameObject {
private:
    float speed;

public:
    Paddle(float x, float y, float width, float height, float speed)
        : GameObject(x, y, width, height), speed(speed) {}

    void update(float deltaTime) override {
        // Movement is handled in the Game class based on input
    }

    void draw() override {
        int currentX = static_cast<int>(round(position.x));
        int currentY = static_cast<int>(round(position.y));
        attron(COLOR_PAIR(2)); // Paddle color
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
            deactivate();
            return true;
        }
        colorPair = 3 + (3 - hitPoints);
        return false;
    }

    int getScore() const { return score; }
};

class BattleBox {
private:
    int x, y;         // Top-left corner position
    int width, height; // Box dimensions
    bool needsRedraw;  // Flag to determine if the box needs redrawing

public:
    BattleBox(int startX, int startY, int w, int h)
        : x(startX), y(startY), width(w), height(h), needsRedraw(true) {}

    void draw() {
        if (!needsRedraw) return;

        attron(A_REVERSE);
        for (int i = -1; i <= width + 1; i++) {
            mvaddch(y, x + i, ' ');              // Top border
            mvaddch(y + height, x + i, ' ');     // Bottom border
        }
        for (int i = 0; i <= height; i++) {
            mvaddch(y + i, x, ' ');              // Left border
            mvaddch(y + i, x + width, ' ');      // Right border
            mvaddch(y + i, x - 1, ' ');          // Left border (extended)
            mvaddch(y + i, x + 1 + width, ' ');  // Right border (extended)
        }
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

class Game {
private:
    Paddle paddle;
    std::vector<Block> blocks;
    int score;

public:
    Game(int startX, int startY)
        : paddle(startX + 20, startY + 14, 10, 1, 5), score(0) {
        // Create blocks
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 10; j++) {
                blocks.emplace_back(startX + j * 6 + 5, startY + i + 1); // Simple grid formation
            }
        }
    }

    void update() {
        // Update game logic (e.g., check for collisions)
    }

    void draw() {
        clear();
        paddle.draw();
        for (auto& block : blocks) {
            block.draw();
        }
        mvprintw(0, 0, "Score: %d", score);
        refresh();
    }

    void handleInput(int ch) {
        switch (ch) {
            case KEY_LEFT:
                paddle.moveLeft(1.0f, 0);
                break;
            case KEY_RIGHT:
                paddle.moveRight(1.0f, COLS);
                break;
            case 'q':
                endwin();
                exit(0);
        }
    }
};

int main() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    start_color();
    init_pair(2, COLOR_CYAN, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_GREEN, COLOR_BLACK);

    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);

    BattleBox battleBox(maxX / 2 - 20, maxY / 2 - 8, 40, 16);
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
