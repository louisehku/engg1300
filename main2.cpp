#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <limits>
#include <ncursesw/ncurses.h>
#include <unistd.h>
#include <cmath>
#include <cstring>

class Heart {
private:
    float x, y;
    int lastDrawnX, lastDrawnY;
    float directionX, directionY;
    float baseSpeed;
    float aspectRatio;
    bool moving;
    int symbol;

public:
    Heart(int startX, int startY) : 
        x(static_cast<float>(startX)), y(static_cast<float>(startY)), 
        lastDrawnX(startX), lastDrawnY(startY),
        directionX(0.0f), directionY(0.0f),
        baseSpeed(0.3f), aspectRatio(2.0f), 
        moving(false), symbol(ACS_DIAMOND) {}

    void update() {
        if (moving) {
            x += directionX * baseSpeed * aspectRatio;
            y += directionY * baseSpeed;
        }
    }

    void setDirection(float dx, float dy) {
        if (dx != 0.0f || dy != 0.0f) {
            float length = sqrt(dx * dx + dy * dy);
            directionX = dx / length;
            directionY = dy / length;
            moving = true;
        }
    }

    void setAspectRatio(float ratio) { aspectRatio = ratio; }
    void setSpeed(float speed) { baseSpeed = speed; }
    void stop() { moving = false; }
    void start() { moving = true; }
    bool isMoving() const { return moving; }
    void setPosition(float newX, float newY) { x = newX; }
    void clearPrevious() { mvaddch(lastDrawnY, lastDrawnX, ' '); }
    
    void draw() {
        int currentX = static_cast<int>(round(x));
        int currentY = static_cast<int>(round(y));
        
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
    
    float getX() const { return x; }
    float getY() const { return y; }
};

class BattleBox {
private:
    int x, y, width, height;
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
    int getX() const { return x; }
    int getY() const { return y; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
};

int main() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);
    
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_RED, COLOR_BLACK);
    }

    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);

    BattleBox battleBox(maxX / 2 - 20, maxY / 2 - 8, 40, 16);
    Heart heart(maxX / 2, maxY / 2);

    battleBox.draw();
    mvprintw(maxY - 3, 2, "Arrow keys to move, Space to stop/start");
    mvprintw(maxY - 2, 2, "Q to quit");

    bool running = true;
    while (running) {
        int ch = getch();
        if (ch != ERR) {
            if (ch == 'q' || ch == 'Q') {
                running = false;
            } else if (ch == ' ') {
                if (heart.isMoving()) {
                    heart.stop();
                } else {
                    heart.start();
                }
            } else if (ch == KEY_UP) {
                heart.setDirection(0.0f, -1.0f);
            } else if (ch == KEY_DOWN) {
                heart.setDirection(0.0f, 1.0f);
            } else if (ch == KEY_LEFT) {
                heart.setDirection(-1.0f, 0.0f);
            } else if (ch == KEY_RIGHT) {
                heart.setDirection(1.0f, 0.0f);
            }
        }

        heart.update();
        heart.draw();
        refresh();
        usleep(16667);
    }

    endwin();
    return 0;
}
