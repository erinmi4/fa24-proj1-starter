#include "state.h"

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
game_state_t *create_default_state() {
    // 创建蛇
    snake_t *NewSnake = (snake_t*) malloc(sizeof(snake_t));
    if (NewSnake == NULL) {
        printf("蛇的空间分配出错");
        exit(1);
    }
    NewSnake->head_row = 2;
    NewSnake->head_col = 4;
    NewSnake->live = true;
    NewSnake->tail_col = 2;
    NewSnake->tail_row = 2;

    // 创建游戏状态
    game_state_t *NewState = (game_state_t*) malloc(sizeof(game_state_t));
    if (NewState == NULL) {
        printf("游戏状态的空间分配出错");
        exit(1);
    }
    NewState->snakes = NewSnake;
    NewState->num_rows = 18;
    NewState->num_snakes = 1;

    // 创建游戏板
    NewState->board = (char **)malloc(NewState->num_rows * sizeof(char *));
    if (NewState->board == NULL) {
        printf("Memory allocation failed for board.\n");
        exit(1);
    }

    for (int i = 0; i < NewState->num_rows; ++i) {
        NewState->board[i] = (char *)malloc(20 * sizeof(char)); // 假设每行有20列
        if (NewState->board[i] == NULL) {
            printf("Memory allocation failed for board row.\n");
            exit(1);
        }
    }

    // 初始化游戏板内容
    for (int i = 0; i < NewState->num_rows; ++i) {
        for (int j = 0; j < 20; ++j) {
            if (i == 0 || j == 0 || i == NewState->num_rows - 1 || j == 19) {
                NewState->board[i][j] = '#'; // 墙壁
            } else {
                NewState->board[i][j] = ' '; // 空白
            }
        }
    }

    // 放置果实
    NewState->board[2][9] = '*'; // 在(2,9)放置果实

    // 放置蛇的头部和尾部,
    NewState->board[NewSnake->tail_row][NewSnake->tail_col + 1 ] = '>'; // 蛇身的设置其实是硬编码
    NewState->board[NewSnake->tail_row][NewSnake->tail_col] = 'd'; // 设置尾部
    NewState->board[NewSnake->head_row][NewSnake->head_col] = 'D'; // 设置头部

    return NewState; // 返回创建的游戏状态
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
*/
static char next_square(game_state_t *state, unsigned int snum) {
  // TODO: Implement this function.
  char next;
    switch (snum) {
        case 0: //也就是尾巴为s的蛇，会向下移动
            next = get_board_at(state, get_next_row(state->snakes->head_row,'s'), get_next_col(state->snakes->head_col,'s'));
        case 1://尾巴为d的蛇，向右边的蛇
            next = get_board_at(state, get_next_row(state->snakes->head_row,'d'), get_next_col(state->snakes->head_col,'d'));
        case 2://尾巴为a的蛇，向左边移动
            next = get_board_at(state, get_next_row(state->snakes->head_row,'a'), get_next_col(state->snakes->head_col,'a'));
        case 3://尾巴为w的蛇，向上移动
            next =get_board_at(state, get_next_row(state->snakes->head_row,'w'), get_next_col(state->snakes->head_col,'w'));
    }
        return next;
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
  return;
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
  return;
}

/* Task 4.5 */
void update_state(game_state_t *state, int (*add_food)(game_state_t *state)) {
  // TODO: Implement this function.
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