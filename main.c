#include <curses.h>
#include <stdlib.h>

#define FRAMETIME 120


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
} snake;

typedef struct item {
	int x;
	int y;
	char c;
} item;

typedef struct game {
	snake snake;
	item item;
	WINDOW *arena;
} game;


// function definitions

void grow (snake *s);
void destroy (snake s);


// main

int main () {
	// init ncurses
	initscr();

	cbreak();
	noecho();
	curs_set(FALSE);

	refresh();

	// init game
	game game;

	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);

	cell *head = malloc(sizeof(cell));
	head->y  = 30;
	head->x  = 30;
	head->dy = 0;
	head->dx = 1;
	head->next = NULL;
	head->prev = NULL;

	snake s;
	s.length = 1;
	s.alive = 1;
	s.head = head;
	s.tail = s.head;

	game.snake = s;
	

	int key;
	cell *tmp;

	// game loop
	while (s.alive) {
		// listen for key press
		key = getch();
		switch (key) {
			case KEY_UP:
				s.head->dy = -1;
				s.head->dx = 0;
				break;
			case KEY_DOWN:
				s.head->dy = 1;
				s.head->dx = 0;
				break;
			case KEY_LEFT:
				s.head->dy = 0;
				s.head->dx = -1;
				break;
			case KEY_RIGHT:
				s.head->dy = 0;
				s.head->dx = 1;
				break;
		}

		// update cell positions
		tmp = s.head;
		while (tmp != NULL) {
			tmp->y += tmp->dy;
			tmp->x += tmp->dx;

			tmp = tmp->next;
		}

		// update vectors
		tmp = s.tail;
		while (tmp != NULL && tmp->prev != NULL) {
			tmp->dy = tmp->prev->dy;
			tmp->dx = tmp->prev->dx;

			tmp = tmp->prev;
		}

		// hit detection - bounds
		/*if (s.head->y < 0 || s.head->y > getmaxy(stdscr) ||
			s.head->x < 0 || s.head->x > getmaxx(stdscr))
			break; */

		// walk through walls
		if (s.head->y < 0)
			s.head->y = getmaxy(stdscr);
		if (s.head->y > getmaxy(stdscr))
			s.head->y = 0;
		if (s.head->x < 0)
			s.head->x = getmaxx(stdscr);
		if (s.head->x > getmaxx(stdscr))
			s.head->x = 0;
		
		// hit detection - snake
		if ((mvinch(s.head->y, s.head->x) & A_CHARTEXT) == '#')
			break;

		// hardcode end
		if (s.length < 10)
			grow(&s);
		else
			break;

		// clear screen
		clear();

		// draw
		tmp = s.head;

		while (tmp != NULL) {
			mvaddch(tmp->y, tmp->x, '#');
			tmp = tmp->next;
		}

		refresh();

		// sleep
		napms(FRAMETIME);
	}

	nodelay(stdscr, FALSE);
	getch();
	
	// cleanup
	endwin();
	destroy(s);
	

	return 0;
}

// function implementations

void grow (snake *s) {
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

void destroy (snake s) {
	cell *tmp = s.head;

	while (tmp->next != NULL) {
		tmp = tmp->next;
		free(tmp->prev);
	}

	free(tmp);
}
