#include <iostream>
#include <vector>
#include <ncursesw/ncurses.h>
#include <unistd.h>
#include <cmath>
#include <cstdlib>
#include <ctime>

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

// Block class
class Block {
public:
    int x, y;
    bool active;
    Block(int startX, int startY) : x(startX), y(startY), active(true) {}

    void draw() {
        if (active) {
            mvaddch(y, x, ACS_CKBOARD); // Block representation
        }
    }

    bool checkCollision(const Bullet& bullet) {
        return bullet.x == x && bullet.y == y;
    }

    void hit() {
        active = false; // Mark block as inactive when hit
    }
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
    std::vector<Block> blocks; // Vector to hold blocks
    int score;

public:
    Game() : player(40, 20), score(0) {
        // Create enemies
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 10; j++) {
                enemies.emplace_back(j * 6 + 5, i + 1); // Simple grid formation
            }
        }
        
        // Create blocks
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 10; j++) {
                blocks.emplace_back(j * 6 + 5, i + 10); // Block layout below enemies
            }
        }
    }

    void update() {
        // Move bullets
        for (int i = 0; i < bullets.size(); i++) {
            bullets[i].move();
            // Check for collisions with enemies
            for (int j = 0; j < enemies.size(); j++) {
                if (checkCollision(bullets[i], enemies[j])) {
                    bullets.erase(bullets.begin() + i);
                    enemies.erase(enemies.begin() + j);
                    score++;
                    i--; // Adjust index after removal
                    break; // Exit inner loop
                }
            }
            // Check for collisions with blocks
            for (int j = 0; j < blocks.size(); j++) {
                if (blocks[j].checkCollision(bullets[i])) {
                    bullets.erase(bullets.begin() + i);
                    blocks[j].hit();
                    score += 10; // Increment score for hitting a block
                    i--; // Adjust index after removal
                    break; // Exit inner loop
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
        for (auto& block : blocks) {
            block.draw(); // Draw blocks
        }
        mvprintw(0, 0, "Score: %d", score);
        refresh();
    }

    void handleInput(int ch) {
        switch (ch) {
            case KEY_LEFT:
                if (player.x > 0) player.move(-1);
                break;
            case KEY_RIGHT:
                if (player.x < COLS - 1) player.move(1);
                break;
            case ' ':
                bullets.emplace_back(player.x, player.y - 1); // Shoot bullet
                break;
            case 'q':
                endwin();
                exit(0); // Quit the game
        }
    }

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
};

int main() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    
    Game game;
    
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
