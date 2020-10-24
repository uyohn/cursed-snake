#include <curses.h>
#include <stdlib.h>
#include <time.h>


#define FRAMETIME 100


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
void   clear_snake_trail     (game g);
void   destroy_snake         (snake *s);
void   control_snake         (int key, snake *s);

void   update_cell_positions (snake *s);
void   update_cell_vectors   (snake *s);

// item
item  *create_item           ();
void   draw_item             (game g);
void   move_item             (game g);


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
	
	// init random number generator
	time_t t;
	srand((unsigned) time(&t));

	game.item = create_item();
	move_item(game);

	loop(game);

	nodelay(stdscr, FALSE);
	getch();
	
	// cleanup
	endwin();
	destroy_snake(game.snake);
	free(game.item);

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
		clear_snake_trail(game);
		update_snake(game);

		// draw

		box(game.arena.win, 0, 0);

		draw_item(game);

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

	// hit detection - item
	if ((mvwinch(g.arena.win, g.snake->head->y, g.snake->head->x) & A_CHARTEXT) == g.item->c) {
		grow_snake(g.snake);
		move_item(g);
	}
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

void clear_snake_trail (game g) {
	mvwaddch(g.arena.win, g.snake->tail->y, g.snake->tail->x, ' ');
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


// item
item *create_item () {
	item *i = malloc(sizeof(item));
	i->x = 4;
	i->y = 4;
	i->c = '*';

	return i;
}

void draw_item (game g) {
	mvwaddch(g.arena.win, g.item->y, g.item->x, g.item->c);
}

// TODO: don't put new item under snake
// POSSIBLE BUG if item spawns under snake, it get's cleared by clear_snake_trail
void move_item (game g) {
	g.item->y = 1 + (rand() % (g.arena.h - 2));
	g.item->x = 1 + (rand() % (g.arena.w - 2));
}
