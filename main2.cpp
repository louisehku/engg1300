#include <graphics.h> // Only for Turbo C++
#include <SFML/Graphics.hpp>

int xp[20], yp[20];
const int sz = 6;
int xx[sz], yy[sz];

void bar(int z) {
    window(z + 1, 22, 70, 25);
    textcolor(10);
    cprintf(" LLL");
    window(z + 1, 23, 70, 25);
    cprintf(" LLL");
    textcolor(9);
    cprintf(" L");
    textcolor(10);
    cprintf(" LLL");
    window(z + 1, 24, 70, 25);
    cprintf(" L");
    textcolor(9);
    cprintf(" L");
    textcolor(10);
    cprintf(" LLL");
}

void up() {
    int flag = 0, k;
    static int scr = 0;

    for (int i = 0; i < sz; i++) {
        if (xx[i] == 0) {
            continue;
        }
        for (k = 0; k < 20; k++) {
            if (xx[i] == xp[k] && yy[i] == yp[k] && xp[k] != 0) {
                xx[i] = 0;
                window(74, 20, 80, 24);
                scr++;
                textcolor(15);
                cprintf("%4d", scr);
                if (scr < 10) {
                    cprintf(" ");
                }
            }
        }
    }

    textcolor(10);
    for (i = 0; i < 20; i++) {
        if (xp[i] != 0) {
            flag++;
            window(xp[i], yp[i], xp[i] + 1, yp[i] + 1);
            if (yp[i] == 1) {
                cprintf("%c", 4);
                window(xp[i], yp[i] + 1, xp[i] + 1, yp[i] + 2);
            }
            if (yp[i] == 21) {
                cprintf(" ");
                yp[i]--;
                delay(22 / flag);
            }
            if (yp[i] == 0) {
                xp[i] = 0;
            }
        }
    }
    textcolor(WHITE);
}

void status(int x) {
    window(66, 17, 79, 19);
    if (x == 1) {
        textcolor(LIGHTGREEN);
        cprintf(" Playing ");
    } else if (x == 2) {
        cprintf(" Paused ");
    } else {
        textcolor(LIGHTRED);
        cprintf(" Stopped ");
    }
}

void down() {
    int x;
    if (random(10) == 1) {
        x = random(52) + 7;
        textcolor(12);
    }
    for (int i = 0; i < sz; i++) {
        if (xx[i] == 0) {
            xx[i] = x;
            yy[i] = 2;
            window(xx[i], yy[i], xx[i] + 1, yy[i] + 1);
            cprintf(" L ");
            break;
        }
    }
    for (int i = 0; i < sz; i++) {
        if (random(2)) {
            window(xx[i], yy[i], xx[i], yy[i] + 2);
            cprintf(" L ");
            if (yy[i] >= 20) {
                status(3);
                sound(100);
                delay(210);
                nosound();
                // Additional sound effects
                for (int j = 0; j < 3; j++) {
                    sound(250);
                    delay(100);
                    nosound();
                    delay(250);
                }
                getch();
                delay(1500);
                exit(0);
            }
        }
        yy[i]++;
    }
    textcolor(WHITE);
    delay(300);
}

int main() {
    clrscr();
    char ch;
    _setcursortype(_NOCURSOR);
    randomize();
    textcolor(15);
    cprintf(" ");
    
    for (int i = 0; i < 18; i++) {
        cprintf(" ");
    }
    
    window(66, 3, 78, 21);
    textcolor(11);
    cprintf(" press x to exit ");
    textcolor(14);
    cprintf(" press p to pause game ");
    textcolor(11);
    cprintf(" press space to shoot ");
    textcolor(14);
    cprintf(" press left/right to move ");
    textcolor(WHITE);
    cprintf(" status: ");
    cprintf(" score: 0 ");

    for (int i = 0; i < 20; i++) {
        xp[i] = 0;
    }
    for (int i = 0; i < sz; i++) {
        xx[i] = 0;
    }
    
    int x = 0;
    bar(x);
    
    while (1) {
        status(1);
        
        while (kbhit()) {
            up();
            down();
        }
        
        ch = getch();
        switch (tolower(ch)) {
            case 'p':
                status(2);
                sound(1500);
                delay(110);
                nosound();
                break;
            case ' ':
                sound(600);
                delay(40);
                nosound();
                sound(1500);
                delay(40);
                nosound();
                for (int i = 0; i < 20; i++) {
                    if (xp[i] == 0) {
                        xp[i] = x + 5;
                        yp[i] = 21;
                        break;
                    }
                }
                break;
            case 0:
                ch = getch();
                if (ch == 75 && x > 0) {
                    x -= 1;
                    bar(x);
                } else if (ch == 77 && x < 55) {
                    x += 1;
                    bar(x);
                }
                break;
            case 'x':
                return;
        }
    }
}
