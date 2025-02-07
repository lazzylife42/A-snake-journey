#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <string.h>
#include <time.h>

// Définition des caractères spéciaux
#define HEAD "O"   // Au lieu de "◉"
#define BODY "o"   // Au lieu de "○"
#define FOOD "*"   // Au lieu de "★"
#define EMPTY " "

typedef struct {
    int x, y;
} Point;

typedef struct {
    Point* body;
    int length;
    Point food;
    int dx, dy;
    int score;
    int width;
    int height;
    int is_running;
} Game;

static struct termios orig_term;
static Game game;

// Nettoyage et restauration du terminal
void cleanup(void) {
    free(game.body);
    printf("\033[?25h"); // Afficher le curseur
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_term);
}

// Gestionnaire Ctrl+C
void handle_signal(int sig) {
    game.is_running = 0;
}

// Initialisation du terminal
void init_terminal(void) {
    struct termios new_term;
    struct winsize w;

    tcgetattr(STDIN_FILENO, &orig_term);
    new_term = orig_term;
    new_term.c_lflag &= ~(ICANON | ECHO);
    new_term.c_cc[VMIN] = 0;
    new_term.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);

    printf("\033[?25l");

    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    game.width = w.ws_col - 2;
    game.height = w.ws_row - 3;

    atexit(cleanup);
    signal(SIGINT, handle_signal);
}

void init_game(void) {
    game.body = malloc(sizeof(Point) * (game.width * game.height));
    if (!game.body) {
        fprintf(stderr, "Erreur mémoire\n");
        exit(1);
    }

    game.body[0].x = game.width / 4;
    game.body[0].y = game.height / 2;
    game.length = 1;
    game.dx = 1;
    game.dy = 0;
    game.score = 0;
    game.is_running = 1;

    srand(time(NULL));
    game.food.x = game.width * 3 / 4;
    game.food.y = game.height / 2;

    printf("\033[2J");
}

void handle_input(void) {
    char c;
    if (read(STDIN_FILENO, &c, 1) == 1) {
        if (c == '\x1b') {
            char seq[2];
            if (read(STDIN_FILENO, seq, 2) == 2) {
                if (seq[0] == '[') {
                    switch (seq[1]) {
                        case 'A': // Haut
                            if (game.dy != 1) { game.dx = 0; game.dy = -1; }
                            break;
                        case 'B': // Bas
                            if (game.dy != -1) { game.dx = 0; game.dy = 1; }
                            break;
                        case 'C': // Droite
                            if (game.dx != -1) { game.dx = 1; game.dy = 0; }
                            break;
                        case 'D': // Gauche
                            if (game.dx != 1) { game.dx = -1; game.dy = 0; }
                            break;
                    }
                }
            } else {
                game.is_running = 0;
            }
        }
        else if (c == 'q' || c == 'Q') {
            game.is_running = 0;
        }
    }
}

void update(void) {
    Point new_head = game.body[0];
    new_head.x += game.dx;
    new_head.y += game.dy;

    if (new_head.x < 0) new_head.x = game.width - 1;
    if (new_head.x >= game.width) new_head.x = 0;
    if (new_head.y < 0) new_head.y = game.height - 1;
    if (new_head.y >= game.height) new_head.y = 0;

    for (int i = 0; i < game.length; i++) {
        if (new_head.x == game.body[i].x && new_head.y == game.body[i].y) {
            game.is_running = 0;
            return;
        }
    }

    for (int i = game.length - 1; i > 0; i--) {
        game.body[i] = game.body[i-1];
    }
    game.body[0] = new_head;

    if (new_head.x == game.food.x && new_head.y == game.food.y) {
        game.length++;
        game.score += 10;
        do {
            game.food.x = rand() % game.width;
            game.food.y = rand() % game.height;
        } while (game.food.x == new_head.x && game.food.y == new_head.y);
    }
}

void render(void) {
    printf("\033[H");

    printf("Score: %d (Utilisez les flèches, Q ou ESC pour quitter)\n", game.score);

    printf("+");
    for (int i = 0; i < game.width; i++) printf("-");
    printf("+\n");

    for (int y = 0; y < game.height; y++) {
        printf("|");
        for (int x = 0; x < game.width; x++) {
            const char* c = EMPTY;
            for (int i = 0; i < game.length; i++) {
                if (x == game.body[i].x && y == game.body[i].y) {
                    c = (i == 0) ? HEAD : BODY;
                    break;
                }
            }
            if (x == game.food.x && y == game.food.y) {
                c = FOOD;
            }
            printf("%s", c);
        }
        printf("|\n");
    }

    printf("+");
    for (int i = 0; i < game.width; i++) printf("-");
    printf("+");
    
    fflush(stdout);
}

int main(void) {
    init_terminal();
    init_game();

    while (game.is_running) {
        handle_input();
        update();
        render();
        usleep(100000);
    }

    printf("\033[2J\033[H");
    printf("Game Over! Score final: %d\n", game.score);

    return 0;
}