#include <cmath>
#include <cstdlib>
#include <ctime>
#include <curses.h>
#include <unistd.h>
#include <vector>

#define gravity 1
#define friction 0.5
#define maxVelx 5
#define maxVely 8
#define ballAspectRatio 2.0f

struct Ball {
  int x, y;
  double velx, vely;
  float ry;
  float rx = ry * ballAspectRatio;
};

struct Bullet {
  int x, y;
  double velx, vely;
};

void addBullets(std::vector<Bullet> &bullets, int numOfBullets) {
  for (int i = 0; i < numOfBullets; i++) {
    bullets.push_back({-1, -1});
  }
}

void printBall(Ball ball) {
  attron(COLOR_PAIR(1));
  for (int i = -ball.ry; i <= ball.ry; i++) {
    for (int j = -ball.rx; j <= ball.rx; j++) {
      if (pow(i, 2) / pow(ball.ry, 2) + pow(j, 2) / pow(ball.rx, 2) <= 1) {
        mvaddch(ball.y + i, ball.x + j, '#');
      }
    }
  }
  attroff(COLOR_PAIR(1));
}

bool checkBulletCollision(Ball &ball, Bullet &bullet) {
  if (bullet.x >= ball.x - ball.rx && bullet.x <= ball.x + ball.rx &&
      bullet.y >= ball.y - ball.ry - 1 && bullet.y <= ball.y + ball.ry + 1) {
    return true;
  }
  return false;
}

int main(int argc, char *argv[]) {
  srand(time(NULL));

  WINDOW *win = initscr();
  curs_set(0);
  keypad(stdscr, TRUE);
  nodelay(win, TRUE);
  timeout(0);

  use_default_colors();
  start_color();
  init_pair(1, COLOR_GREEN, -1);
  init_pair(2, COLOR_RED, -1);

  int numOfBullets = 5;
  float startRadius = 5.0;
  if (COLS < 20 || LINES < 10) {
    numOfBullets = 2;
    startRadius = 3.0;
  } else if (COLS < 100 || LINES < 35) {
    numOfBullets = 3;
    startRadius = 4.0;
  }

  bool gameOver = false;
  int sec = 50000;
  Ball ball = {COLS / 2, 10, 0.0, 0.0, startRadius};
  int score = 0;
  int addBulletInverval = 100;

  std::vector<Bullet> bullets;
  addBullets(bullets, numOfBullets);

  while (!gameOver) {
    MEVENT event;
    int ch;
    bool jump = false, moveDown = false, moveLeft = false, moveRight = false;

    // Input handling
    while ((ch = wgetch(win)) != ERR) {
      if (ch == 'q') {
        gameOver = true;
        break;
      }
      if (ch == KEY_UP || ch == ' ' || ch == 'k') {
        jump = true;
      } else if (ch == KEY_LEFT || ch == 'h') {
        moveLeft = true;
      } else if (ch == KEY_RIGHT || ch == 'l') {
        moveRight = true;
      }
    }
    if (jump && ball.vely > -maxVely) {
      ball.vely = -5;
    }
    if (moveLeft && ball.velx > -maxVelx) {
      ball.velx -= 3;
    }
    if (moveRight && ball.velx < maxVelx) {
      ball.velx += 3;
    }

    ball.vely += gravity;
    ball.x += ball.velx;
    ball.y += ball.vely;
    if (ball.velx > friction) {
      ball.velx -= friction;
    } else if (ball.velx < friction) {
      ball.velx += friction;
    } else {
      ball.velx = 0;
    }

    if (score > addBulletInverval) {
      addBullets(bullets, 1);
      addBulletInverval += score;
    }
    if (score % 300 == 0) {
      sec -= 1000;
    }

    for (int i = 0; i < bullets.size(); i++) {
      bullets[i].x += bullets[i].velx;
      bullets[i].y += bullets[i].vely;

      if (checkBulletCollision(ball, bullets[i])) {
        if (ball.ry > 1) {
          ball.ry--;
          ball.rx = ball.ry * ballAspectRatio;
        } else {
          gameOver = true;
          break;
        }
        bullets[i].x = -1;
        bullets[i].y = -1;
      }

      // Reset bullets
      if (bullets[i].y >= LINES - 1 || bullets[i].x >= COLS - 1 || bullets[i].x < 0 ||
          bullets[i].y < 0) {
        score++;
        if (rand() % 2 == 1) {
          bullets[i].y = 0;
          bullets[i].x = rand() % COLS;
          bullets[i].velx = rand() % 2 - 1;
          bullets[i].vely = rand() % 2 + 1;
        } else {
          bullets[i].x = 0;
          bullets[i].y = rand() % LINES + 15;
          bullets[i].velx = rand() % 3 + 2;
          bullets[i].vely = 0;
        }
      }
    }

    // Boundary Collisions
    if (ball.y >= LINES - ball.ry - 1) {
      ball.y = LINES - ball.ry - 1;
      ball.vely = 0;
    } else if (ball.y < ball.ry) {
      ball.y = ball.ry;
      ball.vely = 0;
    }

    if (ball.x < ball.rx || ball.x >= COLS - ball.rx) {
      ball.velx = 0;
      ball.x = (ball.x < ball.rx) ? ball.rx : ball.x;
      ball.x = (ball.x >= COLS - ball.rx) ? COLS - ball.rx - 1 : ball.x;
    }

    erase();

    printBall(ball);
    for (auto bullet : bullets) {
      attron(COLOR_PAIR(2));
      mvaddch(bullet.y, bullet.x, '@');
      attroff(COLOR_PAIR(2));
    }
    mvprintw(0, COLS / 2 - 4, "Score: %d", score);
    usleep(sec);
  }

  endwin();
  printf("Game Over\nScore: %d\n", score);
  return 0;
}
