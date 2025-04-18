#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <limits>

int xp[20], yp[20];
const int sz = 6;
int xx[sz], yy[sz];

void bar(int z) {
    // Display the bar position
    std::cout << "Bar position: " << z << std::endl;
}

void up(int& scr) {
    // Update score logic when shooting
    for (int i = 0; i < sz; i++) {
        if (xx[i] != 0) {
            scr++;
            xx[i] = 0; // Reset the object
            std::cout << "Score: " << scr << std::endl;
        }
    }
}

void status(int x) {
    if (x == 1) {
        std::cout << "Status: Playing" << std::endl;
    } else if (x == 2) {
        std::cout << "Status: Paused" << std::endl;
    } else {
        std::cout << "Status: Stopped" << std::endl;
    }
}

void down(int& x) {
    // Simulate a target appearing
    int targetX = rand() % 52 + 7; // Random position for target
    for (int i = 0; i < sz; i++) {
        if (xx[i] == 0) {
            xx[i] = targetX;
            yy[i] = 2; // Set the y position of the target
            std::cout << "Target at (" << xx[i] << ", " << yy[i] << ")" << std::endl;
            break;
        }
    }
}

int main() {
    std::srand(std::time(0)); // Initialize random seed
    char ch;
    int scr = 0;

    // Initialize game state
    std::fill_n(xp, 20, 0);
    std::fill_n(xx, sz, 0);
    std::fill_n(yy, sz, 0);

    int x = 0; // Player's bar position
    bar(x);

    while (true) {
        status(1);
        std::cout << "Controls: p - pause, space - shoot, x - exit, a - left, d - right" << std::endl;

        // Check for player input
        if (std::cin >> ch) {
            ch = std::tolower(ch);
            switch (ch) {
                case 'p':
                    status(2);
                    break;
                case ' ':
                    up(scr); // Increase score on shoot
                    break;
                case 'x':
                    return 0; // Exit the program
                case 'a':
                    if (x > 0) {
                        x--; // Move left
                        bar(x);
                    }
                    break;
                case 'd':
                    if (x < 55) {
                        x++; // Move right
                        bar(x);
                    }
                    break;
                default:
                    break;
            }
        }
        
        down(x); // Update game state
    }

    return 0; // Ensure main returns an int
}
