#include "unit_tests.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "snake_utils.h"
#include "asserts.h"
// Necessary due to static functions in state.c
#include "state.c"

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

bool clear_unit_test_files() {
  int err_in = remove("unit-test-in.snk");
  int err_out = remove("unit-test-out.snk");
  int err_ref = remove("unit-test-ref.snk");
  return err_in == 0 && err_out == 0 && err_ref == 0;
}

bool test_create_default_state() {
  game_state_t *state = create_default_state();

  if (state == NULL) {
    printf("%s\n", "create_default_state is not implemented.");
    return false;
  }

  if (!assert_equals_unsigned_int("board width", DEFAULT_BOARD_WIDTH, 20)) {
    return false;
  }

  if (!assert_equals_unsigned_int("board height", DEFAULT_BOARD_HEIGHT, state->num_rows)) {
    return false;
  }

  // Test if the board is mutable
  // If it crashes here, your board is not mutable
  char original_char = get_board_at(state, 0, 0);
  set_board_at(state, 0, 0, '0');
  set_board_at(state, 0, 0, original_char);

  for (unsigned int row = 0; row < DEFAULT_BOARD_HEIGHT; row++) {
    for (unsigned int col = 0; col < DEFAULT_BOARD_WIDTH; col++) {
      if (row == 0 || col == 0 || row == DEFAULT_BOARD_HEIGHT - 1 || col == DEFAULT_BOARD_WIDTH - 1) {
        if (!assert_map_equals(state, row, col, '#')) {
          return false;
        }
      } else if (col == 9 && row == 2) {
        if (!assert_map_equals(state, row, col, '*')) {
          return false;
        }
      } else if (col == 2 && row == 2) {
        if (!assert_map_equals(state, row, col, 'd')) {
          return false;
        }
      } else if (col == 3 && row == 2) {
        if (!assert_map_equals(state, row, col, '>')) {
          return false;
        }
      } else if (col == 4 && row == 2) {
        if (!assert_map_equals(state, row, col, 'D')) {
          return false;
        }
      } else {
        if (!assert_map_equals(state, row, col, ' ')) {
          return false;
        }
      }
    }

    if (!(get_board_at(state, row, DEFAULT_BOARD_WIDTH) == '\0' ||
          (get_board_at(state, row, DEFAULT_BOARD_WIDTH) == '\n' &&
           get_board_at(state, row, DEFAULT_BOARD_WIDTH + 1) == '\0'))) {
      printf("Warning: we think row %d of your board does not end in a null byte\n", row);
    }
  }

  if (!assert_equals_unsigned_int("number of snakes", 1, state->num_snakes)) {
    return false;
  }

  if (!assert_equals_unsigned_int("row of snake tail", 2, state->snakes->tail_row)) {
    return false;
  }
  if (!assert_equals_unsigned_int("col of snake tail", 2, state->snakes->tail_col)) {
    return false;
  }
  if (!assert_equals_unsigned_int("row of snake head", 2, state->snakes->head_row)) {
    return false;
  }
  if (!assert_equals_unsigned_int("col of snake head", 4, state->snakes->head_col)) {
    return false;
  }
  if (!assert_true("snake is alive", state->snakes->live)) {
    return false;
  }

  free_state(state);

  return true;
}

bool test_free_state() {
  game_state_t *state = create_default_state();
  free_state(state);

  printf("%s\n",
         "This test case only checks for leaks in Tasks 1 and 2. Make sure that no Valgrind errors are printed!");

  return true;
}

bool test_print_board_1() {
  clear_unit_test_files();

  char *expected = "####################\n"
                   "#                  #\n"
                   "# d>D    *         #\n"
                   "#                  #\n"
                   "#                  #\n"
                   "#                  #\n"
                   "#                  #\n"
                   "#                  #\n"
                   "#                  #\n"
                   "#                  #\n"
                   "#                  #\n"
                   "#                  #\n"
                   "#                  #\n"
                   "#                  #\n"
                   "#                  #\n"
                   "#                  #\n"
                   "#                  #\n"
                   "####################\n";
  size_t file_size = strlen(expected);

  game_state_t *state = create_default_state();
  save_board(state, "unit-test-out.snk");

  // We hardcoded a filesize here since we know the board is 378 characters.
  // You shouldn't hardcode sizes in your own implementation.
  char actual[file_size + 1];

  FILE *f = fopen("unit-test-out.snk", "r");

  if (!assert_file_size("unit-test-out.snk", file_size)) {
    return false;
  }

  fread(actual, file_size, 1, f);
  fclose(f);
  actual[file_size] = '\0';

  if (strcmp(expected, actual) != 0) {
    printf("%s\n", "Your printed board doesn't match the expected output. See unit-test-out.snk for what you printed.");
    return false;
  }

  free_state(state);

  return true;
}

bool test_print_board_2() {
  clear_unit_test_files();

  char *expected = "####################\n"
                   "#                  #\n"
                   "# d>D    *         #\n"
                   "####################\n";
  size_t file_size = strlen(expected);

  game_state_t *state = create_default_state();

  for (int i = 4; i < state->num_rows; i++) {
    free(state->board[i]);
  }

  state->num_rows = 4;
  strncpy(state->board[3], state->board[0], DEFAULT_BOARD_WIDTH);
  save_board(state, "unit-test-out.snk");

  char actual[file_size + 1];

  FILE *f = fopen("unit-test-out.snk", "r");

  if (!assert_file_size("unit-test-out.snk", file_size)) {
    return false;
  }

  fread(actual, file_size, 1, f);
  fclose(f);
  actual[file_size] = '\0';

  if (strcmp(expected, actual) != 0) {
    printf("%s\n", "Your printed board doesn't match the expected output. See unit-test-out.snk for what you printed.");
    return false;
  }

  free_state(state);

  return true;
}

bool test_print_board() {
  if (!test_print_board_1()) {
    printf("%s\n", "test_print_board_1 failed. Check unit-test-out.snk for a diagram of the board.");
    return false;
  }

  if (!test_print_board_2()) {
    printf("%s\n", "test_print_board_2 failed. Check unit-test-out.snk for a diagram of the board.");
    return false;
  }

  return true;
}

bool test_next_square_board_1() {
  clear_unit_test_files();

  /*
  Board 1 (default):
  ####################
  #                  #
  # d>D    *         #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  ####################
  */

  // set up board
  game_state_t *actual_state = create_default_state();
  save_board(actual_state, "unit-test-out.snk");

  // the next square for the snake should be ' '
  if (!assert_equals_char("next_square on board 1", ' ', next_square(actual_state, 0))) {
    return false;
  }

  // check that board has not changed
  game_state_t *expected_state = create_default_state();
  if (!assert_state_equals(expected_state, actual_state)) {
    printf("%s\n", "Error: next_square should not modify board");
    return false;
  }

  free_state(actual_state);
  free_state(expected_state);

  return true;
}

bool test_next_square_board_2() {
  clear_unit_test_files();

  /*
  Board 2:
  ####################
  #                  #
  # d>D*   *         #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  ####################
  */

  game_state_t *state = create_default_state();

  // set up board
  set_board_at(state, 2, 5, '*');
  save_board(state, "unit-test-out.snk");

  // the next square for the snake should be '*'
  if (!assert_equals_char("next_square on board 2", '*', next_square(state, 0))) {
    return false;
  }

  free_state(state);

  return true;
}

bool test_next_square_board_3() {
  clear_unit_test_files();

  /*
  Board 3:
  ####################
  #                  #
  # d>Dx   *         #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  ####################
  */

  game_state_t *state = create_default_state();

  // set up board
  set_board_at(state, 2, 5, 'x');
  save_board(state, "unit-test-out.snk");

  // the next square for the snake should be 'x'
  if (!assert_equals_char("next_square on board 3", 'x', next_square(state, 0))) {
    return false;
  }

  free_state(state);

  return true;
}

bool test_next_square_board_4() {
  clear_unit_test_files();

  /*
  Board 4:
  ####################
  #   #              #
  # d>W    *         #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  ####################
  */

  game_state_t *state = create_default_state();

  // set up board
  set_board_at(state, 2, 4, 'W');
  set_board_at(state, 1, 4, '#');
  state->snakes->head_row = 2;
  state->snakes->head_col = 4;
  save_board(state, "unit-test-out.snk");

  // the next square for the snake should be '#'
  if (!assert_equals_char("next_square on board 4", '#', next_square(state, 0))) {
    return false;
  }

  free_state(state);

  return true;
}

bool test_next_square_board_5() {
  clear_unit_test_files();

  /*
  Board 5:
  ####################
  #                  #
  # d>v    *         #
  #   v              #
  #   S              #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  ####################
  */

  game_state_t *state = create_default_state();

  // set up board
  set_board_at(state, 2, 4, 'v');
  set_board_at(state, 3, 4, 'v');
  set_board_at(state, 4, 4, 'S');
  state->snakes->head_row = 4;
  state->snakes->head_col = 4;
  save_board(state, "unit-test-out.snk");

  // the next square for the snake should be ' '
  if (!assert_equals_char("next_square on board 5", ' ', next_square(state, 0))) {
    return false;
  }

  free_state(state);

  return true;
}

bool test_next_square_board_6() {
  clear_unit_test_files();

  /*
  Board 6:
  ####################
  #                  #
  # d>v    *         #
  #   v              #
  #   S              #
  #   #              #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  ####################
  */

  game_state_t *state = create_default_state();

  // set up board
  set_board_at(state, 2, 4, 'v');
  set_board_at(state, 3, 4, 'v');
  set_board_at(state, 4, 4, 'S');
  set_board_at(state, 5, 4, '#');
  state->snakes->head_row = 4;
  state->snakes->head_col = 4;
  save_board(state, "unit-test-out.snk");

  // the next square for the snake should be '#'
  if (!assert_equals_char("next_square on board 6", '#', next_square(state, 0))) {
    return false;
  }

  free_state(state);

  return true;
}

bool test_next_square() {
  if (!test_next_square_board_1()) {
    printf("%s\n", "test_next_square_board_1 failed. Check unit-test-out.snk for a diagram of the board.");
    return false;
  }

  if (!test_next_square_board_2()) {
    printf("%s\n", "test_next_square_board_2 failed. Check unit-test-out.snk for a diagram of the board.");
    return false;
  }

  if (!test_next_square_board_3()) {
    printf("%s\n", "test_next_square_board_3 failed. Check unit-test-out.snk for a diagram of the board.");
    return false;
  }

  if (!test_next_square_board_4()) {
    printf("%s\n", "test_next_square_board_4 failed. Check unit-test-out.snk for a diagram of the board.");
    return false;
  }

  if (!test_next_square_board_5()) {
    printf("%s\n", "test_next_square_board_5 failed. Check unit-test-out.snk for a diagram of the board.");
    return false;
  }

  if (!test_next_square_board_6()) {
    printf("%s\n", "test_next_square_board_6 failed. Check unit-test-out.snk for a diagram of the board.");
    return false;
  }

  return true;
}

bool test_update_head_board_1() {
  clear_unit_test_files();

  /*
  Board 1 (default):
  ####################            ####################
  #                  #            #                  #
  # d>D    *         #            # d>>D   *         #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  # ---------> #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  ####################            ####################
  */

  // set up expected board
  game_state_t *expected = create_default_state();
  set_board_at(expected, 2, 4, '>');
  set_board_at(expected, 2, 5, 'D');
  expected->snakes->head_row = 2;
  expected->snakes->head_col = 5;
  save_board(expected, "unit-test-ref.snk");

  // set up actual board
  game_state_t *actual = create_default_state();
  save_board(actual, "unit-test-in.snk");

  update_head(actual, 0);
  save_board(actual, "unit-test-out.snk");

  // check that actual board matches expected board
  if (!assert_state_equals(expected, actual)) {
    return false;
  }

  free_state(expected);
  free_state(actual);

  return true;
}

bool test_update_head_board_2() {
  clear_unit_test_files();

  /*
  Board 2:
  ####################            ####################
  #                  #            #   W              #
  # d>W    *         #            # d>^    *         #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  # ---------> #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  ####################            ####################
  */

  // set up expected board
  game_state_t *expected = create_default_state();
  set_board_at(expected, 2, 4, '^');
  set_board_at(expected, 1, 4, 'W');
  expected->snakes->head_row = 1;
  expected->snakes->head_col = 4;
  save_board(expected, "unit-test-ref.snk");

  // set up actual board
  game_state_t *actual = create_default_state();
  set_board_at(actual, 2, 4, 'W');
  actual->snakes->head_row = 2;
  actual->snakes->head_col = 4;
  save_board(actual, "unit-test-in.snk");

  update_head(actual, 0);
  save_board(actual, "unit-test-out.snk");

  // check that actual board matches expected board
  if (!assert_state_equals(expected, actual)) {
    return false;
  }

  free_state(expected);
  free_state(actual);

  return true;
}

bool test_update_head_board_3() {
  clear_unit_test_files();

  /*
  Board 3:
  ####################            ####################
  #   A              #            #  A<              #
  # d>^    *         #            # d>^    *         #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  # ---------> #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  ####################            ####################
  */

  // set up expected board
  game_state_t *expected = create_default_state();
  set_board_at(expected, 2, 4, '^');
  set_board_at(expected, 1, 4, '<');
  set_board_at(expected, 1, 3, 'A');
  expected->snakes->head_row = 1;
  expected->snakes->head_col = 3;
  save_board(expected, "unit-test-ref.snk");

  // set up actual board
  game_state_t *actual = create_default_state();
  set_board_at(actual, 2, 4, '^');
  set_board_at(actual, 1, 4, 'A');
  actual->snakes->head_row = 1;
  actual->snakes->head_col = 4;
  save_board(actual, "unit-test-in.snk");

  update_head(actual, 0);
  save_board(actual, "unit-test-out.snk");

  // check that actual board matches expected board
  if (!assert_state_equals(expected, actual)) {
    return false;
  }

  free_state(expected);
  free_state(actual);

  return true;
}

bool test_update_head() {
  if (!test_update_head_board_1()) {
    printf("%s\n",
           "test_update_head_board_1 failed. Check unit-test-in.snk, unit-test-out.snk, and unit-test-ref.snk.");
    return false;
  }

  if (!test_update_head_board_2()) {
    printf("%s\n",
           "test_update_head_board_2 failed. Check unit-test-in.snk, unit-test-out.snk, and unit-test-ref.snk.");
    return false;
  }

  if (!test_update_head_board_3()) {
    printf("%s\n",
           "test_update_head_board_3 failed. Check unit-test-in.snk, unit-test-out.snk, and unit-test-ref.snk.");
    return false;
  }

  return true;
}

bool test_update_tail_board_1() {
  clear_unit_test_files();

  /*
  Board 1:
  ####################            ####################
  #                  #            #                  #
  # d>>D   *         #            #  d>D   *         #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  # ---------> #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  ####################            ####################
  */

  // set up expected board
  game_state_t *expected = create_default_state();
  set_board_at(expected, 2, 2, ' ');
  set_board_at(expected, 2, 3, 'd');
  set_board_at(expected, 2, 4, '>');
  set_board_at(expected, 2, 5, 'D');
  expected->snakes->head_row = 2;
  expected->snakes->head_col = 5;
  expected->snakes->tail_row = 2;
  expected->snakes->tail_col = 3;
  save_board(expected, "unit-test-ref.snk");

  // set up actual board
  game_state_t *actual = create_default_state();
  set_board_at(actual, 2, 4, '>');
  set_board_at(actual, 2, 5, 'D');
  actual->snakes->head_row = 2;
  actual->snakes->head_col = 5;
  save_board(actual, "unit-test-in.snk");

  update_tail(actual, 0);
  save_board(actual, "unit-test-out.snk");

  // check that actual board matches expected board
  if (!assert_state_equals(expected, actual)) {
    return false;
  }

  free_state(expected);
  free_state(actual);

  return true;
}

bool test_update_tail_board_2() {
  clear_unit_test_files();

  /*
  Board 2:
  ####################            ####################
  #                  #            #                  #
  # dv     *         #            #   s    *         #
  #  v               #            #   v              #
  #  S               #            #   S              #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  # ---------> #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  ####################            ####################
  */

  // set up expected board
  game_state_t *expected = create_default_state();
  set_board_at(expected, 2, 2, ' ');
  set_board_at(expected, 2, 3, 's');
  set_board_at(expected, 2, 4, ' ');
  set_board_at(expected, 3, 3, 'v');
  set_board_at(expected, 4, 3, 'S');
  expected->snakes->head_row = 4;
  expected->snakes->head_col = 3;
  expected->snakes->tail_row = 2;
  expected->snakes->tail_col = 3;
  save_board(expected, "unit-test-ref.snk");

  // set up actual board
  game_state_t *actual = create_default_state();
  set_board_at(actual, 2, 3, 'v');
  set_board_at(actual, 2, 4, ' ');
  set_board_at(actual, 3, 3, 'v');
  set_board_at(actual, 4, 3, 'S');
  actual->snakes->head_row = 4;
  actual->snakes->head_col = 3;
  save_board(actual, "unit-test-in.snk");

  update_tail(actual, 0);
  save_board(actual, "unit-test-out.snk");

  // check that actual board matches expected board
  if (!assert_state_equals(expected, actual)) {
    return false;
  }

  free_state(expected);
  free_state(actual);

  return true;
}

bool test_update_tail_board_3() {
  clear_unit_test_files();

  /*
  Board 3:
  ####################            ####################
  #                  #            #                  #
  # S<     *         #            # Sa     *         #
  #  w               #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  # ---------> #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  ####################            ####################
  */

  // set up expected board
  game_state_t *expected = create_default_state();

  save_board(expected, "unit-test-ref.snk");
  set_board_at(expected, 2, 2, 'S');
  set_board_at(expected, 2, 3, 'a');
  set_board_at(expected, 2, 4, ' ');
  expected->snakes->head_row = 2;
  expected->snakes->head_col = 2;
  expected->snakes->tail_row = 2;
  expected->snakes->tail_col = 3;

  // set up actual board
  game_state_t *actual = create_default_state();
  set_board_at(actual, 2, 2, 'S');
  set_board_at(actual, 2, 3, '<');
  set_board_at(actual, 2, 4, ' ');
  set_board_at(actual, 3, 3, 'w');
  actual->snakes->head_row = 2;
  actual->snakes->head_col = 2;
  actual->snakes->tail_row = 3;
  actual->snakes->tail_col = 3;
  save_board(actual, "unit-test-in.snk");

  update_tail(actual, 0);
  save_board(actual, "unit-test-out.snk");

  // check that actual board matches expected board
  if (!assert_state_equals(expected, actual)) {
    return false;
  }

  free_state(expected);
  free_state(actual);

  return true;
}

bool test_update_tail() {
  if (!test_update_tail_board_1()) {
    printf("%s\n",
           "test_update_tail_board_1 failed. Check unit-test-in.snk, unit-test-out.snk, and unit-test-ref.snk.");
    return false;
  }

  if (!test_update_tail_board_2()) {
    printf("%s\n",
           "test_update_tail_board_2 failed. Check unit-test-in.snk, unit-test-out.snk, and unit-test-ref.snk.");
    return false;
  }

  if (!test_update_tail_board_3()) {
    printf("%s\n",
           "test_update_tail_board_3 failed. Check unit-test-in.snk, unit-test-out.snk, and unit-test-ref.snk.");
    return false;
  }

  return true;
}

bool test_update_state_board_1() {
  clear_unit_test_files();

  /*
  Board 1 (default):
  ####################            ####################
  #                  #            #                  #
  # d>D    *         #            #  d>D   *         #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  # ---------> #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  ####################            ####################
  */

  // set up expected board
  game_state_t *expected = create_default_state();
  set_board_at(expected, 2, 2, ' ');
  set_board_at(expected, 2, 3, 'd');
  set_board_at(expected, 2, 4, '>');
  set_board_at(expected, 2, 5, 'D');
  expected->snakes->head_row = 2;
  expected->snakes->head_col = 5;
  expected->snakes->tail_row = 2;
  expected->snakes->tail_col = 3;
  save_board(expected, "unit-test-ref.snk");

  // set up actual board
  game_state_t *actual = create_default_state();
  save_board(actual, "unit-test-in.snk");

  update_state(actual, corner_food);
  save_board(actual, "unit-test-out.snk");

  // check that actual board matches expected board
  if (!assert_state_equals(expected, actual)) {
    return false;
  }

  free_state(expected);
  free_state(actual);

  return true;
}

bool test_update_state_board_2() {
  clear_unit_test_files();

  /*
  Board 2:
  ####################            ####################
  #                  #            #*                 #
  # d>D*   *         #            # d>>D   *         #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  # ---------> #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  ####################            ####################
  */

  // set up expected board
  game_state_t *expected = create_default_state();
  set_board_at(expected, 2, 4, '>');
  set_board_at(expected, 2, 5, 'D');
  set_board_at(expected, 1, 1, '*');
  expected->snakes->head_row = 2;
  expected->snakes->head_col = 5;
  save_board(expected, "unit-test-ref.snk");

  // set up actual board
  game_state_t *actual = create_default_state();
  set_board_at(actual, 2, 5, '*');
  save_board(actual, "unit-test-in.snk");

  update_state(actual, corner_food);
  save_board(actual, "unit-test-out.snk");

  // check that actual board matches expected board
  if (!assert_state_equals(expected, actual)) {
    return false;
  }

  free_state(expected);
  free_state(actual);

  return true;
}

bool test_update_state_board_3() {
  clear_unit_test_files();

  /*
  Board 3:
  ####################            ####################
  # >W               #            # >x               #
  # w      *         #            # w      *         #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  # ---------> #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  ####################            ####################
  */

  // set up expected board
  game_state_t *expected = create_default_state();
  set_board_at(expected, 2, 2, 'w');
  set_board_at(expected, 2, 3, ' ');
  set_board_at(expected, 2, 4, ' ');
  set_board_at(expected, 1, 2, '>');
  set_board_at(expected, 1, 3, 'x');
  expected->snakes->head_row = 1;
  expected->snakes->head_col = 3;
  expected->snakes->live = false;
  save_board(expected, "unit-test-ref.snk");

  // set up actual board
  game_state_t *actual = create_default_state();
  set_board_at(actual, 2, 2, 'w');
  set_board_at(actual, 2, 3, ' ');
  set_board_at(actual, 2, 4, ' ');
  set_board_at(actual, 1, 2, '>');
  set_board_at(actual, 1, 3, 'W');
  actual->snakes->head_row = 1;
  actual->snakes->head_col = 3;
  actual->snakes->tail_row = 2;
  actual->snakes->tail_col = 2;
  save_board(actual, "unit-test-in.snk");

  update_state(actual, 0);
  save_board(actual, "unit-test-out.snk");

  // check that actual board matches expected board
  if (!assert_state_equals(expected, actual)) {
    return false;
  }

  free_state(expected);
  free_state(actual);

  return true;
}

bool test_update_state_board_4() {
  clear_unit_test_files();

  /*
  Board 4:
  ####################            ####################
  # >v               #            # >v               #
  # wA     *         #            # wx     *         #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  # ---------> #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  #                  #            #                  #
  ####################            ####################
  */

  // set up expected board
  game_state_t *expected = create_default_state();
  set_board_at(expected, 2, 2, 'w');
  set_board_at(expected, 2, 3, 'x');
  set_board_at(expected, 2, 4, ' ');
  set_board_at(expected, 1, 2, '>');
  set_board_at(expected, 1, 3, 'v');
  expected->snakes->head_row = 2;
  expected->snakes->head_col = 3;
  expected->snakes->live = false;
  save_board(expected, "unit-test-ref.snk");

  // set up actual board
  game_state_t *actual = create_default_state();
  set_board_at(actual, 2, 2, 'w');
  set_board_at(actual, 1, 2, '>');
  set_board_at(actual, 1, 3, 'v');
  set_board_at(actual, 2, 3, 'A');
  set_board_at(actual, 2, 4, ' ');
  actual->snakes->head_row = 2;
  actual->snakes->head_col = 3;
  actual->snakes->tail_row = 2;
  actual->snakes->tail_col = 2;
  save_board(actual, "unit-test-in.snk");

  update_state(actual, corner_food);
  save_board(actual, "unit-test-out.snk");

  // check that actual board matches expected board
  if (!assert_state_equals(expected, actual)) {
    return false;
  }

  free_state(expected);
  free_state(actual);

  return true;
}

bool test_update_state() {
  if (!test_update_state_board_1()) {
    printf("%s\n",
           "test_update_state_board_1 failed. Check unit-test-in.snk, unit-test-out.snk, and unit-test-ref.snk.");
    return false;
  }

  if (!test_update_state_board_2()) {
    printf("%s\n",
           "test_update_state_board_2 failed. Check unit-test-in.snk, unit-test-out.snk, and unit-test-ref.snk.");
    return false;
  }

  if (!test_update_state_board_3()) {
    printf("%s\n",
           "test_update_state_board_3 failed. Check unit-test-in.snk, unit-test-out.snk, and unit-test-ref.snk.");
    return false;
  }

  if (!test_update_state_board_4()) {
    printf("%s\n",
           "test_update_state_board_4 failed. Check unit-test-in.snk, unit-test-out.snk, and unit-test-ref.snk.");
    return false;
  }

  return true;
}

bool test_read_line_1() {
  clear_unit_test_files();

  FILE *fp = fopen("tests/01-simple-in.snk", "r");
  char *expected = "####################\n";
  char *actual = read_line(fp);
  fclose(fp);

  if (actual == NULL) return false;
  if (strcmp(expected, actual) != 0) {
    free(actual);
    return false;
  }
  free(actual);
  return true;
}

bool test_read_line_2() {
  clear_unit_test_files();

  FILE *fp = fopen("tests/13-sus-in.snk", "r");

  char *expected[] = {"##############\n", "#            #\n", "#  ########  #\n", "#  #      #  #####\n"};

  bool success = true;
  for (int i = 0; i < 4; i++) {
    char *actual = read_line(fp);
    if (actual == NULL) return false;
    if (strcmp(expected[i], actual) != 0) {
      success = false;
      break;
    }
    free(actual);
  }

  fclose(fp);
  return success;
}

bool test_read_line() {
  if (!test_read_line_1()) {
    printf("%s\n",
           "test_read_line_1 failed. Check the first line of tests/01-simple-in.snk for a diagram of the line.");
    return false;
  }

  if (!test_read_line_2()) {
    printf("%s\n",
           "test_read_line_2 failed. Check the first four lines of tests/13-sus-in.snk for a diagram of the board.");
    return false;
  }

  return true;
}

bool test_load_board_1() {
  clear_unit_test_files();

  /*
  Board 01-simple (default):
  ####################
  #                  #
  # d>D    *         #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  ####################
  */
  char *expected = "####################\n"
                   "#                  #\n"
                   "# d>D    *         #\n"
                   "#                  #\n"
                   "#                  #\n"
                   "#                  #\n"
                   "#                  #\n"
                   "#                  #\n"
                   "#                  #\n"
                   "#                  #\n"
                   "#                  #\n"
                   "#                  #\n"
                   "#                  #\n"
                   "#                  #\n"
                   "#                  #\n"
                   "#                  #\n"
                   "#                  #\n"
                   "####################\n";
  FILE *fp = fopen("tests/01-simple-in.snk", "r");
  bool check = assert_load_equals(fp, expected);
  fclose(fp);
  return check;
}

bool test_load_board_2() {
  clear_unit_test_files();

  /*
  Board 06-small (small):
  #####
  #   #
  # W #
  # ^ #
  # w #
  #####
  */
  char *expected = "#####\n"
                   "#   #\n"
                   "# W #\n"
                   "# ^ #\n"
                   "# w #\n"
                   "#####\n";
  FILE *fp = fopen("tests/06-small-in.snk", "r");
  bool check = assert_load_equals(fp, expected);
  fclose(fp);
  return check;
}

bool test_load_board_3() {
  clear_unit_test_files();

  /*
  Board 13-sus (non square):
  ##############
  #            #
  #  ########  #
  #  #      #  #####
  #  ########      #
  #                #
  #      #         #
  #                #
  #   ######   #####
  #   #    #   #
  #   #    #   #
  #####    #####

  */
  char *expected = "##############\n"
                   "#            #\n"
                   "#  ########  #\n"
                   "#  #      #  #####\n"
                   "#  ########      #\n"
                   "#                #\n"
                   "#      #         #\n"
                   "#                #\n"
                   "#   ######   #####\n"
                   "#   #    #   #\n"
                   "#   #    #   #\n"
                   "#####    #####\n";
  FILE *fp = fopen("tests/13-sus-in.snk", "r");
  bool check = assert_load_equals(fp, expected);
  fclose(fp);
  return check;
}

bool test_load_board() {
  if (!test_load_board_1()) {
    printf("%s\n", "test_load_board_1 failed. Check tests/01-simple-in.snk for a diagram of the board.");
    return false;
  }

  if (!test_load_board_2()) {
    printf("%s\n", "test_load_board_2 failed. Check tests/06-small-in.snk for a diagram of the board.");
    return false;
  }
  if (!test_load_board_3()) {
    printf("%s\n", "test_load_board_3 failed. Check tests/13-sus-in.snk for a diagram of the board.");
    return false;
  }

  return true;
}

bool test_find_head_board_1() {
  clear_unit_test_files();

  /*
  Board 1:
  ####################
  #                  #
  # d>v    *         #
  #   v  W           #
  #   v  ^           #
  #   >>>^           #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  ####################
  */

  // set up expected board
  game_state_t *expected = create_default_state();
  set_board_at(expected, 2, 4, 'v');
  set_board_at(expected, 3, 4, 'v');
  set_board_at(expected, 4, 4, 'v');
  set_board_at(expected, 5, 4, '>');
  set_board_at(expected, 5, 5, '>');
  set_board_at(expected, 5, 6, '>');
  set_board_at(expected, 5, 7, '^');
  set_board_at(expected, 4, 7, '^');
  set_board_at(expected, 3, 7, 'W');
  expected->snakes->head_row = 3;
  expected->snakes->head_col = 7;
  save_board(expected, "unit-test-out.snk");

  // set up actual board
  game_state_t *actual = create_default_state();
  set_board_at(actual, 2, 4, 'v');
  set_board_at(actual, 3, 4, 'v');
  set_board_at(actual, 4, 4, 'v');
  set_board_at(actual, 5, 4, '>');
  set_board_at(actual, 5, 5, '>');
  set_board_at(actual, 5, 6, '>');
  set_board_at(actual, 5, 7, '^');
  set_board_at(actual, 4, 7, '^');
  set_board_at(actual, 3, 7, 'W');

  find_head(actual, 0);

  // check that actual board matches expected board
  if (!assert_state_equals(expected, actual)) {
    return false;
  }

  free_state(expected);
  free_state(actual);

  return true;
}

bool test_find_head_board_2() {
  clear_unit_test_files();

  /*
  Board 2:
  ####################
  #                  #
  # d>Ds   *         #
  #    v             #
  #    S             #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  ####################
  */

  // set up expected board
  game_state_t *expected = create_default_state();
  set_board_at(expected, 2, 5, 's');
  set_board_at(expected, 3, 5, 'v');
  set_board_at(expected, 4, 5, 'S');
  expected->snakes = realloc(expected->snakes, sizeof(snake_t) * 2);
  snake_t *tmp = expected->snakes;
  tmp->tail_row = 2;
  tmp->tail_col = 2;
  tmp->head_row = 2;
  tmp->head_col = 4;
  tmp++;
  tmp->tail_row = 2;
  tmp->tail_col = 5;
  tmp->head_row = 4;
  tmp->head_col = 5;
  tmp->live = true;
  expected->num_snakes = 2;
  save_board(expected, "unit-test-out.snk");

  // set up actual board
  game_state_t *actual = create_default_state();
  set_board_at(actual, 2, 5, 's');
  set_board_at(actual, 3, 5, 'v');
  set_board_at(actual, 4, 5, 'S');
  actual->snakes = realloc(actual->snakes, sizeof(snake_t) * 2);
  tmp = actual->snakes;
  tmp->tail_row = 2;
  tmp->tail_col = 2;
  tmp->head_row = 2;
  tmp->head_col = 4;
  tmp++;
  tmp->tail_row = 2;
  tmp->tail_col = 5;
  tmp->live = true;
  actual->num_snakes = 2;

  find_head(actual, 1);

  // check that actual board matches expected board
  if (!assert_state_equals(expected, actual)) {
    return false;
  }

  free_state(expected);
  free_state(actual);

  return true;
}

bool test_find_head() {
  if (!test_find_head_board_1()) {
    printf("%s\n", "test_find_head_board_1 failed. Check unit-test-out.snk for a diagram of the board.");
    return false;
  }

  if (!test_find_head_board_2()) {
    printf("%s\n", "test_find_head_board_2 failed. Check unit-test-out.snk for a diagram of the board.");
    return false;
  }

  return true;
}

bool test_initialize_snakes_board_1() {
  clear_unit_test_files();

  /*
  Board 1 (default):
  ####################
  #                  #
  # d>D    *         #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  ####################
  */

  // set up expected board
  game_state_t *expected = create_default_state();
  save_board(expected, "unit-test-out.snk");

  // set up actual board
  game_state_t *actual = create_default_state();
  free(actual->snakes);
  actual->snakes = NULL;
  actual->num_snakes = 0;

  actual = initialize_snakes(actual);

  if (actual == NULL) {
    printf("%s\n", "initialize_snakes is not implemented.");
    return false;
  }

  // check that actual board matches expected board
  if (!assert_state_equals(expected, actual)) {
    return false;
  }

  free_state(expected);
  free_state(actual);

  return true;
}

bool test_initialize_snakes_board_2() {
  clear_unit_test_files();

  /*
  Board 2:
  ####################
  #                  #
  # d>v    *         #
  #   v  W           #
  #   v  ^           #
  #   >>>^           #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  #                  #
  ####################
  */

  // set up expected board
  game_state_t *expected = create_default_state();
  set_board_at(expected, 2, 4, 'v');
  set_board_at(expected, 3, 4, 'v');
  set_board_at(expected, 4, 4, 'v');
  set_board_at(expected, 5, 4, '>');
  set_board_at(expected, 5, 5, '>');
  set_board_at(expected, 5, 6, '>');
  set_board_at(expected, 5, 7, '^');
  set_board_at(expected, 4, 7, '^');
  set_board_at(expected, 3, 7, 'W');
  expected->snakes->head_row = 3;
  expected->snakes->head_col = 7;
  save_board(expected, "unit-test-out.snk");

  // set up actual board
  game_state_t *actual = create_default_state();
  set_board_at(actual, 2, 4, 'v');
  set_board_at(actual, 3, 4, 'v');
  set_board_at(actual, 4, 4, 'v');
  set_board_at(actual, 5, 4, '>');
  set_board_at(actual, 5, 5, '>');
  set_board_at(actual, 5, 6, '>');
  set_board_at(actual, 5, 7, '^');
  set_board_at(actual, 4, 7, '^');
  set_board_at(actual, 3, 7, 'W');
  free(actual->snakes);
  actual->snakes = NULL;
  actual->num_snakes = 0;

  actual = initialize_snakes(actual);

  if (actual == NULL) {
    printf("%s\n", "initialize_snakes is not implemented.");
    return false;
  }

  // check that actual board matches expected board
  if (!assert_state_equals(expected, actual)) {
    return false;
  }

  free_state(expected);
  free_state(actual);

  return true;
}

bool test_initialize_snakes() {
  if (!test_initialize_snakes_board_1()) {
    printf("%s\n", "test_initialize_snakes_board_1 failed. Check unit-test-out.snk for a diagram of the board.");
    return false;
  }

  if (!test_initialize_snakes_board_2()) {
    printf("%s\n", "test_initialize_snakes_board_2 failed. Check unit-test-out.snk for a diagram of the board.");
    return false;
  }

  return true;
}

/*
  Feel free to comment out tests.
*/
int main(int argc, char *argv[]) {
  bool MEMCHECK_MODE = false;

  // Parse arguments
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-m") == 0) {
      MEMCHECK_MODE = true;
      continue;
    }
    fprintf(stderr, "Usage: %s [-m]\n", argv[0]);
    return 1;
  }

  init_colors();

  printf("%s\n", "Reminder: These tests are not comprehensive, and passing them does not guarantee that your "
                 "implementation is working.");

  if (MEMCHECK_MODE) {
    printf("\nTesting free_state...\n");
    if (!test_free_state()) {
      return 0;
    }
  } else {
    if (!test_and_print("create_default_state (Task 1)", test_create_default_state)) {
      return 0;
    }
    if (!test_and_print("print_board (Task 3)", test_print_board)) {
      return 0;
    }
    if (!test_and_print("next_square (Task 4)", test_next_square)) {
      return 0;
    }
    if (!test_and_print("update_head (Task 4)", test_update_head)) {
      return 0;
    }
    if (!test_and_print("update_tail (Task 4)", test_update_tail)) {
      return 0;
    }
    if (!test_and_print("update_state (Task 4)", test_update_state)) {
      return 0;
    }
    if (!test_and_print("read_line (Task 5)", test_read_line)) {
      return 0;
    }
    if (!test_and_print("load_board (Task 5)", test_load_board)) {
      return 0;
    }
    if (!test_and_print("find_head (Task 6)", test_find_head)) {
      return 0;
    }
    if (!test_and_print("initialize_snakes (Task 6)", test_initialize_snakes)) {
      return 0;
    }
  }
}
