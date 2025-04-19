
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

// Ball class
class Ball : public GameObject {
private:
    Vector2D velocity;
    float speed;
    int symbol;
    
public:
    Ball(float x, float y, float radius, float speed)
        : GameObject(x, y, 1, 1), speed(speed), symbol(ACS_BULLET) {
        // Initialize with random direction
        float angle = (rand() % 60 + 30) * 3.14159f / 180.0f; // 30-90 degrees in radians
        velocity = Vector2D(cos(angle), -sin(angle)) * speed;
    }
    
    void update(float deltaTime) override {
        position.x += velocity.x * deltaTime;
        position.y += velocity.y * deltaTime;
    }
    
    void draw() override {
        int currentX = static_cast<int>(round(position.x));
        int currentY = static_cast<int>(round(position.y));
        
        // Only redraw if position has changed
        if (currentX != lastDrawnX || currentY != lastDrawnY) {
            // Clear previous position
            clearPrevious();
            
            // Draw at new position
            attron(COLOR_PAIR(1)); // Ball color
            mvaddch(currentY, currentX, symbol);
            attroff(COLOR_PAIR(1));
            
            // Update last drawn position
            lastDrawnX = currentX;
            lastDrawnY = currentY;
        } else {
            // Redraw at same position (in case it was overwritten)
            attron(COLOR_PAIR(1));
            mvaddch(currentY, currentX, symbol);
            attroff(COLOR_PAIR(1));
        }
    }
    
    void bounceX() {
        velocity.x = -velocity.x;
    }
    
    void bounceY() {
        velocity.y = -velocity.y;
    }
    
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
    
    void update(float deltaTime) override {
        // Movement is handled in the Game class based on input
    }
    
    void draw() override {
        int currentX = static_cast<int>(round(position.x));
        int currentY = static_cast<int>(round(position.y));
        
        // Clear previous position if it changed
        if (currentX != lastDrawnX || currentY != lastDrawnY) {
            clearPrevious();
            lastDrawnX = currentX;
            lastDrawnY = currentY;
        }
        
        // Draw paddle
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

// BattleBox class (game area)
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
        for (int i = -1; i <= width+1; i++) {
            mvaddch(y, x + i, ' ');              // Top border (space with reverse highlight)
            mvaddch(y + height, x + i, ' ');     // Bottom border
        }
    
        // Draw the left and right borders of the battle box
        for (int i = 0; i <= height; i++) {
            mvaddch(y + i, x, ' ');              // Left border
            mvaddch(y + i, x + width, ' ');      // Right border
            mvaddch(y + i, x-1, ' ');            // Left border
            mvaddch(y + i, x+1 + width, ' ');    // Right border
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

// Game class to manage the game
class BreakoutGame {
private:
    BattleBox* gameArea;
    Ball* ball;
    Paddle* paddle;
    std::vector<Block*> blocks;
    int score;
    int blockHits;
    int minBlockHits;
    float timeRemaining;
    bool gameOver;
    bool win;
    int statusLine;
    
public:
    BreakoutGame(int startX, int startY, int width, int height, float timeLimit, int minBlockHits)
        : score(0), blockHits(0), minBlockHits(minBlockHits), timeRemaining(timeLimit), 
          gameOver(false), win(false) {
        
        // Initialize random seed
        srand(static_cast<unsigned int>(time(nullptr)));
        
        // Create game area
        gameArea = new BattleBox(startX, startY, width, height);
        
        // Set status line position
        statusLine = startY + height + 2;
        
        // Create ball
        float ballRadius = 1.0f;
        float ballSpeed = 20.0f;
        ball = new Ball(startX + width / 2, startY + height / 2, ballRadius, ballSpeed);
        
        // Create paddle
        float paddleWidth = 10.0f;
        float paddleHeight = 1.0f;
        float paddleY = startY + height - 2.0f;
        float paddleSpeed = 30.0f;
        paddle = new Paddle(startX + (width - paddleWidth) / 2, paddleY, paddleWidth, paddleHeight, paddleSpeed);
        
        // Create blocks
        setupBlocks(startX, startY);
    }
    
    ~BreakoutGame() {
        delete gameArea;
        delete ball;
        delete paddle;
        
        for (auto block : blocks) {
            delete block;
        }
        blocks.clear();
    }
    
    void setupBlocks(int startX, int startY) {
        float blockWidth = 5.0f;
        float blockHeight = 2.0f;
        float startBlockX = startX + 2.0f;
        float startBlockY = startY + 3.0f;
        float spacing = 1.0f;
        
        int rows = 5;
        int cols = static_cast<int>((gameArea->getWidth() - 4 + spacing) / (blockWidth + spacing));
        
        for (int row = 0; row < rows; row++) {
            for (int col = 0; col < cols; col++) {
                float x = startBlockX + col * (blockWidth + spacing);
                float y = startBlockY + row * (blockHeight + spacing);
                
                // Assign different hit points and scores based on row
                int hitPoints = std::min(3, rows - row);
                int blockScore = hitPoints * 50;
                int colorPair = 3 + (3 - hitPoints); // Different colors based on hit points
                
                blocks.push_back(new Block(x, y, blockWidth, blockHeight, hitPoints, blockScore, colorPair));
            }
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
        
        // Update time
        timeRemaining -= deltaTime;
        if (timeRemaining <= 0) {
            timeRemaining = 0;
            checkGameOver();
        }
        
        // Update ball
        ball->update(deltaTime);
        
        // Check wall collisions
        Vector2D ballPos = ball->getPosition();
        Vector2D ballSize = ball->getSize();
        Vector2D ballVel = ball->getVelocity();
        
        // Left and right walls
        if (ballPos.x <= gameArea->getX() + 1 || 
            ballPos.x + ballSize.x >= gameArea->getX() + gameArea->getWidth() - 1) {
            ball->bounceX();
        }
        
        // Top wall
        if (ballPos.y <= gameArea->getY() + 1) {
            ball->bounceY();
        }
        
        // Bottom - game over condition
        if (ballPos.y + ballSize.y >= gameArea->getY() + gameArea->getHeight() - 1) {
            gameOver = true;
            win = false;
            return;
        }
        
        // Paddle collision
        if (ball->collidesWith(*paddle)) {
            // Ensure the ball bounces upward
            if (ballVel.y > 0) {
                ball->bounceY();
                
                // Adjust horizontal velocity based on where ball hit the paddle
                float hitPoint = (ballPos.x + ballSize.x/2) - paddle->getPosition().x;
                float paddleWidth = paddle->getSize().x;
                float normalizedHitPoint = (hitPoint / paddleWidth) * 2 - 1; // -1 to 1
                
                // Update x velocity based on hit position (affects angle of bounce)
                Vector2D vel = ball->getVelocity();
                float speed = std::sqrt(vel.x * vel.x + vel.y * vel.y);
                float angle = normalizedHitPoint * 0.5f; // Max 60 degree bounce
                
                // Set new velocity maintaining speed but changing direction
                Vector2D newVel;
                newVel.x = speed * angle;
                newVel.y = -std::sqrt(speed * speed - newVel.x * newVel.x); // Maintain speed magnitude
                
                // Ensure minimum horizontal velocity
                if (std::abs(newVel.x) < 5.0f) {
                    newVel.x = newVel.x > 0 ? 5.0f : -5.0f;
                    // Recalculate y to maintain speed
                    newVel.y = -std::sqrt(speed * speed - newVel.x * newVel.x);
                }
                
                ball->setVelocity(newVel);
            }
        }
        
        // Block collisions
        for (auto block : blocks) {
            if (block->isActive() && ball->collidesWith(*block)) {
                // Determine which side was hit to bounce correctly
                Vector2D blockPos = block->getPosition();
                Vector2D blockSize = block->getSize();
                
                // Simple bounce logic (can be improved for more realistic physics)
                bool hitVertical = false;
                
                // Check if hit bottom or top of block
                if (ballPos.x + ballSize.x/2 >= blockPos.x && 
                    ballPos.x + ballSize.x/2 <= blockPos.x + blockSize.x) {
                    hitVertical = true;
                }
                
                if (hitVertical) {
                    ball->bounceY();
                } else {
                    ball->bounceX();
                }
                
                if (block->hit()) {
                    score += block->getScore();
                    blockHits++;
                }
                
                break; // Only handle one collision per update
            }
        }
        
        // Check win condition - all blocks destroyed or minimum hits reached
        bool allBlocksDestroyed = true;
        for (auto block : blocks) {
            if (block->isActive()) {
                allBlocksDestroyed = false;
                break;
            }
        }
        
        if (allBlocksDestroyed || blockHits >= minBlockHits) {
            gameOver = true;
            win = true;
        }
    }
    
    void render() {
        // Draw game area
        gameArea->draw();
        
        // Draw game objects
        for (auto block : blocks) {
            if (block->isActive()) {
                block->draw();
            }
        }
        
        paddle->draw();
        ball->draw();
        
        // Draw status line
        mvprintw(statusLine, gameArea->getX(), "Score: %d | Blocks: %d/%d | Time: %.1fs", 
                 score, blockHits, minBlockHits, timeRemaining);
        
        // Draw game over or win message
        if (gameOver) {
            attron(A_BOLD);
            if (win) {
                mvprintw(gameArea->getY() + gameArea->getHeight()/2, 
                         gameArea->getX() + gameArea->getWidth()/2 - 5, "YOU WIN!");
            } else {
                mvprintw(gameArea->getY() + gameArea->getHeight()/2, 
                         gameArea->getX() + gameArea->getWidth()/2 - 5, "GAME OVER!");
            }
            attroff(A_BOLD);
        }
    }
    
    void checkGameOver() {
        if (blockHits >= minBlockHits) {
            gameOver = true;
            win = true;
        } else if (timeRemaining <= 0) {
            gameOver = true;
            win = false;
        }
    }
    
    bool isGameOver() const { return gameOver; }
    bool isWin() const { return win; }
};

int main() {
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);  // Hide cursor
    nodelay(stdscr, TRUE);  // Non-blocking input
    
    // Enable function keys, arrow keys, etc.
    keypad(stdscr, TRUE);
    
    // Set up colors if terminal supports them
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_RED, COLOR_BLACK);    // Ball
        init_pair(2, COLOR_WHITE, COLOR_BLUE);   // Paddle
        init_pair(3, COLOR_BLACK, COLOR_RED);    // Strong blocks
        init_pair(4, COLOR_BLACK, COLOR_YELLOW); // Medium blocks
        init_pair(5, COLOR_BLACK, COLOR_GREEN);  // Weak blocks
        init_pair(6, COLOR_BLACK, COLOR_CYAN);   // One-hit blocks
    }

    // Get terminal dimensions
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);

    // Create breakout game
    float timeLimit = 60.0f; // 60 seconds
    int minBlockHits = 10;   // Minimum blocks to hit to win
    BreakoutGame game(maxX/2 - 30, maxY/2 - 15, 60, 30, timeLimit, minBlockHits);
    
    // Instructions
    mvprintw(maxY - 3, 2, "Use LEFT/RIGHT arrows to move paddle");
    mvprintw(maxY - 2, 2, "Press Q to quit");
    
    // Game loop variables
    float lastTime = static_cast<float>(clock()) / CLOCKS_PER_SEC;
    bool running = true;
    
    // Game loop
    while (running && !game.isGameOver()) {
        // Calculate delta time
        float currentTime = static_cast<float>(clock()) / CLOCKS_PER_SEC;
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        
        // Process input
        int ch;
        while ((ch = getch()) != ERR) {
            if (ch == 'q' || ch == 'Q') {
                running = false;
                break;
            }
            
            game.handleInput(ch, deltaTime);
        }
        
        // Update and render game
        game.update(deltaTime);
        game.render();
        
        // Refresh screen
        refresh();
        
        // Control frame rate
        usleep(16667);  // ~60 FPS (1,000,000 microseconds / 60)
    }
    
    // If game is over, show final screen for a few seconds
    if (game.isGameOver()) {
        game.render();
        refresh();
        nodelay(stdscr, FALSE);
        getch(); // Wait for keypress before exiting
    }
    
    // Clean up
    endwin();
    return 0;
}
