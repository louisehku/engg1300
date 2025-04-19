#include <iostream>
#include <vector>
#include <ncursesw/ncurses.h>
#include <unistd.h>

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

void drawEnemy(const Enemy& enemy, int row) {
    int colorPair = 3 + (4 - row); // Different colors based on the row
    attron(COLOR_PAIR(colorPair));
    mvaddch(enemy.y, enemy.x, '#'); // Enemy representation
    attroff(COLOR_PAIR(colorPair));
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
        setupEnemies(); // Call the new setup function
    }

    void setupEnemies() {
        int rows = 5;
        int cols = 10;
        int startX = boxX + 5; 
        int startY = boxY + 1; 
        int spacing = 2; 

        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                int enemyX = startX + j * (spacing + 2);
                int enemyY = startY + i; 
                enemies.emplace_back(enemyX, enemyY); 
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
        for (size_t i = 0; i < enemies.size(); i++) {
            drawEnemy(enemies[i], i / 10); // Assuming 10 enemies per row
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
    BattleBox battleBox(maxX / 2 - 20, maxY / 2 - 8, 40, 16);
    battleBox.draw();

    Game game(battleBox.getX(), battleBox.getY());

    // Initialize colors
    if (has_colors()) {
        start_color();
        init_pair(3, COLOR_RED, COLOR_BLACK);    // Red for the first row
        init_pair(4, COLOR_YELLOW, COLOR_BLACK); // Yellow for the second row
        init_pair(5, COLOR_GREEN, COLOR_BLACK);  // Green for the last row
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
