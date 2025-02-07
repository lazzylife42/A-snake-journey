import curses
from random import randint
import time

def main(screen):
    # Configuration initiale
    curses.curs_set(0)  # Cache le curseur
    screen.timeout(100)  # Délai pour getch() en millisecondes
    
    # Dimensions du jeu
    sh, sw = screen.getmaxyx()
    
    # Création du serpent (commence au milieu)
    snake_x = sw//4
    snake_y = sh//2
    snake = [
        [snake_y, snake_x],      # Tête
        [snake_y, snake_x-1],    # Corps
        [snake_y, snake_x-2]     # Queue
    ]
    
    # Création de la nourriture
    food = [sh//2, sw//2]
    screen.addch(food[0], food[1], '*')
    
    # Direction initiale (droite)
    key = curses.KEY_RIGHT
    
    # Score
    score = 0
    
    while True:
        # Affichage du score
        screen.addstr(0, 2, f'Score: {score}')
        
        # Obtention de la prochaine touche pressée
        next_key = screen.getch()
        key = key if next_key == -1 else next_key
        
        # Calcul de la nouvelle position de la tête
        new_head = [snake[0][0], snake[0][1]]
        
        if key == curses.KEY_DOWN:
            new_head[0] += 1
        if key == curses.KEY_UP:
            new_head[0] -= 1
        if key == curses.KEY_RIGHT:
            new_head[1] += 1
        if key == curses.KEY_LEFT:
            new_head[1] -= 1
        
        snake.insert(0, new_head)
        
        # Vérification collision avec les murs
        if (snake[0][0] in [0, sh] or 
            snake[0][1] in [0, sw] or 
            snake[0] in snake[1:]):
            curses.endwin()
            print(f"\nGame Over! Score final: {score}")
            break
        
        # Vérification si on mange la nourriture
        if snake[0] == food:
            score += 1
            food = None
            while food is None:
                new_food = [
                    randint(1, sh-1),
                    randint(1, sw-1)
                ]
                food = new_food if new_food not in snake else None
            screen.addch(food[0], food[1], '*')
        else:
            tail = snake.pop()
            screen.addch(tail[0], tail[1], ' ')
        
        # Dessin du serpent
        screen.addch(snake[0][0], snake[0][1], 'O')  # Tête

curses.wrapper(main)