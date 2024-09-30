#include "state.h"
#include "unit_tests.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"

/* Helper function definitions */
static void set_board_at(game_state_t *state, unsigned int row, unsigned int col, char ch);
static bool is_tail(char c);
static bool is_head(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static unsigned int get_next_row(unsigned int cur_row, char c);
static unsigned int get_next_col(unsigned int cur_col, char c);
static void find_head(game_state_t *state, unsigned int snum);
static char next_square(game_state_t *state, unsigned int snum);
static void update_tail(game_state_t *state, unsigned int snum);
static void update_head(game_state_t *state, unsigned int snum);

/* Task 1 */
game_state_t* create_default_state() {
    
    game_state_t *default_state = malloc(sizeof(game_state_t));
    
    /* Set row number */
    default_state->num_rows = 18;

    /* Init the board */
    default_state->board = malloc(sizeof(char *) * default_state->num_rows);
    for (int i = 0; i < default_state->num_rows; i++) {
        *(default_state->board + i) = malloc(sizeof(char) * 21); // an extra bit for '\0'
        default_state->board[i][20] = '\0';
    }

    /* Set up game board */
    for (int i = 0; i < 20; i++) {
        default_state->board[0][i] = '#';
        default_state->board[17][i] = '#';
    }
    
    for (int i = 0; i < 20; i++) {
        if (i == 0 || i == 19) {
            default_state->board[1][i] = '#';
            continue;
        }
        default_state->board[1][i] = ' ';
    }

    for (int i = 2; i < default_state->num_rows - 1; i++) {
        strcpy(default_state->board[i], default_state->board[1]);
    }

    /* Hardcode snack and friut */
    default_state->board[2][2] = 'd';
    default_state->board[2][3] = '>';
    default_state->board[2][4] = 'D';

    default_state->board[2][9] = '*';

    /* Set up snake */
    default_state->num_snakes = 1;
    default_state->snakes = malloc(sizeof(snake_t) * default_state->num_snakes);
    default_state->snakes->tail_row = 2;
    default_state->snakes->tail_col = 2;
    default_state->snakes->head_row = 2;
    default_state->snakes->head_col = 4;
    default_state->snakes->live = 1;

    return default_state;
}



/*整个state由board，snake组成
 * 需要先将板子释放*/
/* Task 2 */
void free_state(game_state_t *state) {
  // TODO: Implement this function.
    if (state!=NULL) {
        for (int i = 0; i < state->num_rows; ++i) {
            free(state->board[i]);
        }
    }
    free(state->board);
    if (state->snakes!=NULL) {
            free(state->snakes);
    }

    free(state);
}

/* Task 3 */
void print_board(game_state_t *state, FILE *fp) {
    if (fp == NULL) {
        printf("Could not open file\n");
        return;
    }
    for (int i = 0; i < state->num_rows; i++) {
        for (int j = 0; j < 20 ; j++) {
            fprintf(fp, "%c", state->board[i][j]);
        }
        fprintf(fp, "\n");  // Add newline after each row except the last one
    }
    // Do not close fp here
}


/*
  Saves the current state into filename. Does not modify the state object.
  (already implemented for you).
*/
void save_board(game_state_t *state, char *filename) {
  FILE *f = fopen(filename, "w");
  print_board(state, f);
  fclose(f);
}

/* Task 4.1 */

/*
  Helper function to get a character from the board
  (already implemented for you).
*/
char get_board_at(game_state_t *state, unsigned int row, unsigned int col) {
    return state->board[row][col];
}

/*
  Helper function to set a character on the board
  (already implemented for you).
*/
static void set_board_at(game_state_t *state, unsigned int row, unsigned int col, char ch) {
  state->board[row][col] = ch;
}

/*
  Returns true if c is part of the snake's tail.
  The snake consists of these characters: "wasd"
  Returns false otherwise.
*/
static bool is_tail(char c) {
  // TODO: Implement this function.
  return c=='w' || c=='a' ||c=='s' ||c=='d' ;
}

/*
  Returns true if c is part of the snake's head.
  The snake consists of these characters: "WASDx"
  Returns false otherwise.
*/
static bool is_head(char c) {
  // TODO: Implement this function.
  return c=='W' || c=='A' ||c=='S' ||c=='D' ||c=='x';
}

/*
  Returns true if c is part of the snake.
  The snake consists of these characters: "wasd^<v>WASDx"
*/
static bool is_snake(char c) {
  // TODO: Implement this function.
  return c=='^' || c=='<' ||c=='v' ||c=='>'|| is_head(c) || is_tail(c);
}

/*
  Converts a character in the snake's body ("^<v>")
  to the matching character representing the snake's
  tail ("wasd").
  这个其实是用来追踪的，如果尾部的前一个会变成尾部。
*/
static char body_to_tail(char c) {
  // TODO: Implement this function.
    switch (c) {
        case '^':
            return 'w';
        case '>':
            return 'd';
        case '<':
            return 'a';
        case 'v':
            return 's';
        default:
            return '\0';
    }
}

/*
  Converts a character in the snake's head ("WASD")
  to the matching character representing the snake's
  body ("^<v>").
*/
static char head_to_body(char c) {
  // TODO: Implement this function.
      switch (c) {
        case 'W':
            return '^';
        case 'A':
            return '<';
        case 'S':
            return 'v';
        case 'D':
            return '>';
        default:
            return '\0';
    }
}

/*
  Returns cur_row + 1 if c is 'v' or 's' or 'S'.
  Returns cur_row - 1 if c is '^' or 'w' or 'W'.
  Returns cur_row otherwise.
*/
static unsigned int get_next_row(unsigned int cur_row, char c) {
  // TODO: Implement this function.

    switch (c) {
        case 'v':
            return cur_row + 1;
        case 's':
            return cur_row + 1;
        case 'S':
            return cur_row + 1;

        case '^':
            return cur_row - 1;
        case 'w':
            return cur_row - 1;
        case 'W':
            return cur_row - 1;
        default:
            return cur_row;
    }
}

/*
  Returns cur_col + 1 if c is '>' or 'd' or 'D'.
  Returns cur_col - 1 if c is '<' or 'a' or 'A'.
  Returns cur_col otherwise.
*/
static unsigned int get_next_col(unsigned int cur_col, char c) {
  // TODO: Implement this function.
      switch (c) {
        case '>':
            return cur_col + 1;
        case 'd':
            return cur_col + 1;
        case 'D':
            return cur_col + 1;

        case '<':
            return cur_col - 1;
        case 'a':
            return cur_col - 1;
        case 'A':
            return cur_col - 1;
        default:
            return cur_col;
    }
}

/*
  Task 4.2

  Helper function for update_state. Return the character in the cell the snake is moving into.

  This function should not modify anything.
  获得该条蛇下一步的位置上的东西
*/
static char next_square(game_state_t *state, unsigned int snum) {
    unsigned int head_row = state->snakes[snum].head_row;
    unsigned int head_col = state->snakes[snum].head_col;
    char head_state = get_board_at(state, head_row, head_col);

    unsigned int next_row = get_next_row(head_row, head_state);
    unsigned int next_col = get_next_col(head_col, head_state);

    return get_board_at(state, next_row, next_col);
  }

/*
  Task 4.3

  Helper function for update_state. Update the head...

  ...on the board: add a character where the snake is moving

  ...in the snake struct: update the row and col of the head

  Note that this function ignores food, walls, and snake bodies when moving the head.
*/
static void update_head(game_state_t *state, unsigned int snum) {
  // TODO: Implement this function.
  /*注意两件事情
  1. 将下一个位置变为头部
  2. 将头部当前位置变为身体
   */
    unsigned int head_row = state->snakes[snum].head_row;
    unsigned int head_col = state->snakes[snum].head_col;
    char head_state = get_board_at(state, head_row, head_col);

    unsigned int next_row = get_next_row(head_row, head_state);
    unsigned int next_col = get_next_col(head_col, head_state);
// 将头部当前位置变为身体
    set_board_at(state,head_row,head_col,head_to_body(head_state));
//将下一个位置变为头部
    state->snakes->head_row = next_row;
    state->snakes->head_col = next_col;
    set_board_at(state,next_row,next_col,head_state);
}

/*
  Task 4.4

  Helper function for update_state. Update the tail...

  ...on the board: blank out the current tail, and change the new
  tail from a body character (^<v>) into a tail character (wasd)

  ...in the snake struct: update the row and col of the tail
*/
static void update_tail(game_state_t *state, unsigned int snum) {
  // TODO: Implement this function.
  unsigned int tail_row= state->snakes[snum].tail_row;
  unsigned int tail_col= state->snakes[snum].tail_col;
  char tail_state= get_board_at(state,tail_row,tail_col);
  unsigned int next_row = get_next_row(tail_row, tail_state);
  unsigned int next_col = get_next_col(tail_col, tail_state);
//当前位置变为空白
  set_board_at(state,tail_row,tail_col,' ');
//身体变为尾巴，修改当前尾巴位置
  char next_state = get_board_at(state,next_row,next_col);
  state->snakes[snum].tail_col = next_col;
  state->snakes[snum].tail_row = next_row;
  set_board_at(state,next_row,next_col,body_to_tail(next_state));
}

/* Task 4.5 */
void update_state(game_state_t *state, int (*add_food)(game_state_t *state)) {
  // TODO: Implement this function.
  // 更新蛇头和蛇尾
  char next;
  unsigned int  cur_col = state->snakes->head_col;
  unsigned int cur_row = state->snakes->head_row;
  for (int i = 0; i < 3; i++)
  {
    update_head(state,i);
    update_tail(state,i);
    next = next_square(state,i);
  }
  if (next == '#' || is_snake(next))
  {
    //蛇会死亡
    state->snakes->live = false;
    //蛇头变为x
    set_board_at(state,cur_row,cur_col,'x');
  }
  else if (next == '*')
  {
    //将水果部分修改为头部
      for (int i = 0; i < 3; i++){
      update_head(state,i);
    }
    //产生一个新的水果
    add_food(state);
  }
  return;
}

/* Task 5.1 */
char *read_line(FILE *fp) {
  // TODO: Implement this function.
  return NULL;
}

/* Task 5.2 */
game_state_t *load_board(FILE *fp) {
  // TODO: Implement this function.
  return NULL;
}

/*
  Task 6.1

  Helper function for initialize_snakes.
  Given a snake struct with the tail row and col filled in,
  trace through the board to find the head row and col, and
  fill in the head row and col in the struct.
*/
static void find_head(game_state_t *state, unsigned int snum) {
  // TODO: Implement this function.
  return;
}

/* Task 6.2 */
game_state_t *initialize_snakes(game_state_t *state) {
  // TODO: Implement this function.
  return NULL;
}

