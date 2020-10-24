#include <curses.h>
#include <stdlib.h>

#define FRAMETIME 240


// structs

typedef struct cell {
	int y;
	int x;
	int dy;
	int dx;
	struct cell *next;
	struct cell *prev;
} cell;

// TODO: add function callbacks
typedef struct snake {
	int length;
	int alive;
	cell *head;
	cell *tail;
	char c;
} snake;

typedef struct item {
	int x;
	int y;
	char c;
} item;

typedef struct arena {
	WINDOW *win;
	int h;
	int w;
} arena;

typedef struct game {
	snake *snake;
	item *item;
	arena arena;
} game;



// function definitions
int    ncurses_setup         ();
int    loop                  (game game);

// snake
snake *create_snake          (int y, int x, int dy, int dx);
void   update_snake          (game g);
void   draw_snake            (game g);
void   grow_snake            (snake *s);
void   destroy_snake         (snake *s);
void   control_snake            (int key, snake *s);

void   update_cell_positions (snake *s);
void   update_cell_vectors   (snake *s);


// main

int main () {
	if ( ncurses_setup() != 0 )
		return -1;

	// init game
	game game;

	arena arena;
	arena.w = 40;
	arena.h = 20;
	arena.win = newwin(arena.h, arena.w, 2, 4);


	game.arena = arena;
	box(arena.win, 0, 0);
	refresh();
	wrefresh(arena.win);

	keypad(arena.win, TRUE);
	nodelay(arena.win, TRUE);

	game.snake = create_snake(10, 10, 0, 1);

	loop(game);

	nodelay(stdscr, FALSE);
	getch();
	
	// cleanup
	endwin();
	destroy_snake(game.snake);
	

	return 0;
}

// game loop
int loop (game game) {
	int key;

	while (game.snake->alive) {
		// listen for key press
		key = wgetch(game.arena.win);
		control_snake(key, game.snake);


		// update
		update_snake(game);

		// draw
		wclear(game.arena.win);
		box(game.arena.win, 0, 0);
		draw_snake(game);
		wrefresh(game.arena.win);

		// sleep
		napms(FRAMETIME);
	}

	return 0;
}

// function implementations

int ncurses_setup () {
	// init ncurses
	initscr();

	if (has_colors() == FALSE) {
		endwin();
		printf("Your terminal doesn't support colors!\n");
		return -1;
	}

	cbreak();
	noecho();
	curs_set(FALSE);

	refresh();

	return 0;
}

snake *create_snake (int y, int x, int dy, int dx) {
	cell *head = malloc(sizeof(cell));
	head->y  = y;
	head->x  = x;
	head->dy = dy;
	head->dx = dx;
	head->next = NULL;
	head->prev = NULL;

	snake *s = malloc(sizeof(snake));
	s->length = 1;
	s->alive = 1;
	s->head = head;
	s->tail = s->head;

	s->c = '+';

	return s;
}

void update_cell_positions (snake *s) {
		cell *tmp;

		tmp = s->head;
		while (tmp != NULL) {
			tmp->y += tmp->dy;
			tmp->x += tmp->dx;

			tmp = tmp->next;
		}
}

void update_cell_vectors (snake *s) {
		cell *tmp;

		tmp = s->tail;
		while (tmp != NULL && tmp->prev != NULL) {
			tmp->dy = tmp->prev->dy;
			tmp->dx = tmp->prev->dx;

			tmp = tmp->prev;
		}
}

void update_snake (game g) {
	// update cell positions
	update_cell_positions(g.snake);

	// update vectors
	update_cell_vectors(g.snake);

	// hit detection - wall
	if (g.snake->head->y <= 0 ||
		g.snake->head->y >= getmaxy(g.arena.win) - 1 ||
		g.snake->head->x <= 0 ||
		g.snake->head->x >= getmaxx(g.arena.win) - 1)
		g.snake->alive = 0;
	
	// hit detection - snake
	if ((mvwinch(g.arena.win, g.snake->head->y, g.snake->head->x) & A_CHARTEXT) == g.snake->c)
		g.snake->alive = 0;

	// hardcode grow
	if (g.snake->length < 20)
		grow_snake(g.snake);
}

void draw_snake (game g) {
		cell *tmp = g.snake->head;

		wattron(g.arena.win, A_REVERSE);

		while (tmp != NULL) {
			mvwaddch(g.arena.win, tmp->y, tmp->x, g.snake->c);
			tmp = tmp->next;
		}

		wattroff(g.arena.win, A_REVERSE);
}

void control_snake(int key, snake *s) {
		switch (key) {
			case KEY_UP:
				s->head->dy = -1;
				s->head->dx = 0;
				break;
			case KEY_DOWN:
				s->head->dy = 1;
				s->head->dx = 0;
				break;
			case KEY_LEFT:
				s->head->dy = 0;
				s->head->dx = -1;
				break;
			case KEY_RIGHT:
				s->head->dy = 0;
				s->head->dx = 1;
				break;
		}
}

void grow_snake (snake *s) {
	cell *new = malloc(sizeof(cell));
	
	new->y = s->tail->y - s->tail->dy;
	new->x = s->tail->x - s->tail->dx;

	new->dy = s->tail->dy;
	new->dx = s->tail->dx;

	new->next = NULL;
	new->prev = s->tail;

	s->tail->next = new;
	s->tail = new;

	s->length++;
}

void destroy_snake (snake *s) {
	cell *tmp = s->head;

	while (tmp->next != NULL) {
		tmp = tmp->next;
		free(tmp->prev);
	}

	free(tmp);
	free(s);
}
