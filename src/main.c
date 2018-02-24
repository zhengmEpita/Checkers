# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include "board.h"
# include "piece.h"
# include "list.h"
# include "find_move.h"
# include "history.h"
# include "exec_move.h"
# include <SDL/SDL.h>
# include <SDL/SDL_image.h>
# include "constants.h"


void fflush_stdin()
{
  int c = 0;
  while (c != '\n' && c != EOF)
    c = getchar();
}

int parse_input(int *curLine, int *curCol,
                int *destLine, int *destCol,
                int seq, int *i_seq, char *filename)
{
  char input[20] = {0};

  if (fgets(input, 20, stdin) != NULL) // reads 18 char + \n, store to input
  {
    if (strchr(input, '\n') == NULL) // input too long, didn't find \n
      fflush_stdin(); // prevent overflow

    if (strncmp(input, "quit", 4) == 0)
      return 1;

    if (strncmp(input, "help", 4) == 0)
      return 2;

    if (strncmp(input, "undo", 4) == 0)
      return 4;

    if (strncmp(input, "save", 4) == 0)
    {
      char *tmp; // type "save filename"
      tmp = strtok(input, " "); // parse save
      tmp = strtok(NULL, " "); // parse filename
      if (tmp) // if user has typed a filename
        strncpy(filename, tmp, strlen(tmp) - 1);
      return 3;
    }

    if (input[0] >= 48 && input[0] <= 57) // digit
    {
      if (seq) // > 0
      {
        sscanf(input, "%d", i_seq);
        if (*i_seq > seq || *i_seq <= 0)
          return -1;
        else
          return 0;
      }

      sscanf(input, "%d %d %d %d", curLine, curCol, destLine, destCol);
      return 0;
    }

    return -1; // error
  }
  else
    return -1; // error
}

int main(int argc, char **argv)
{
  struct board *board = malloc(sizeof(struct board));

  // Init the board.
  // 1st argument is the name of the file that contains the board.
  // If no argument, the default config is used.
  if (argc >= 2)
  {
    if (-1 == open_board_from_file(board, argv[1]))
    {
      print_error("Can not read file");
      free(board);
      return -1;
    }
  }
  else
    boardInit(board);
  boardInitColor(board);

  //print it
  printf("This is the start of the game\n");
  printBoard(board);
  undo_init(board);

  int *curLine, *curCol, *destLine, *destCol, *i_seq;
  curLine  = malloc(sizeof (int));
  curCol   = malloc(sizeof (int));
  destLine = malloc(sizeof (int));
  destCol  = malloc(sizeof (int));
  i_seq    = calloc(1, sizeof (int));
  char *filename = calloc(1024, 1);
  int res;
  struct moves *moves_list;

//------------------------------SDL init-------------------------------------//
  SDL_Surface *screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT,
                       32, SDL_HWSURFACE | SDL_DOUBLEBUF);
  SDL_Surface *W    = IMG_Load("image_case/caseblanche.png");
  SDL_Surface *B    = IMG_Load("image_case/casemaron.png" );
  SDL_Surface *B_WP = IMG_Load("image_case/casemaronpieceblanche.png");
  SDL_Surface *B_BP = IMG_Load("image_case/casemaronpiecenoire.png");
  SDL_Surface *B_WK = IMG_Load("image_case/casemaronpieceblanchedames.png");
  SDL_Surface *B_BK = IMG_Load("image_case/casemaronpiecenoiredames.png");

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Rect position;
//---------------------------SDL fin init------------------------------------//


  //main loop
  int can_play = 1;
  while (can_play)
  {
//--------------------------SDL print board----------------------------------//
   SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0,0,0));

   for (int i = 0 ; i < 10; i++)
   {
     for (int j = 0 ; j < 10; j++)
     {
       position.y = i * BLOCK_SIZE;
       position.x = j * BLOCK_SIZE;

       struct cell c = board->cells[i][j];

       if (c.background == LIGHT)
         SDL_BlitSurface(W, NULL, screen, &position);
       else
       {
         switch (c.data)
         {
           case 0:
               SDL_BlitSurface(B, NULL, screen, &position);
               break;
           case BP:
               SDL_BlitSurface(B_BP, NULL, screen, &position);
               break;
           case BK:
               SDL_BlitSurface(B_BK , NULL, screen, &position);
               break;
           case WP:
               SDL_BlitSurface(B_WP, NULL, screen, &position);
               break;
           case WK:
               SDL_BlitSurface(B_WK, NULL, screen, &position);
               break;
           }
        }
      }
    }

    SDL_Flip(screen);
//--------------------------SDL fin print------------------------------------//


    if ((board->player == PLAYER_WHITE && board->nb_white == 0)
     || (board->player == PLAYER_BLACK && board->nb_black == 0))
    {
      can_play = 0;
      puts("No more pieces!");
      continue;
    }

    moves_list = build_moves(board);
    int nb_seq = list_len(moves_list);
    if (nb_seq > 0)
    {
      printf("You have %d mandatory moves\n", nb_seq);
      list_print(moves_list);
      puts("Which sequence do you want to play?");
    }

    res = parse_input(curLine, curCol, destLine, destCol, nb_seq,
                      i_seq, filename);
    while (res == -1) //error
    {
      print_error("Problem when reading your input");
      res = parse_input(curLine, curCol, destLine, destCol, nb_seq,
                        i_seq, filename);
    }

    if (res == 0) //normal
    {
      int res2 = -1;
      if (*i_seq > 0) // play the sequence moves_list[i]
        res2 = exec_seq_in_list(board, moves_list, *i_seq);
      else // no capture possible
        res2 = move(board, *curLine, *curCol, *destLine, *destCol);

      if (res2 == 0) // success
        board->player *= -1;

      *i_seq = 0;
      printBoard(board);
    }

    if (res == 1) //quit
    {
      can_play = 0;
      continue;
    }

    if (res == 2) //help
    {
      puts("Type 4 digits separated by space character:"
        " current line and column, destination line and column\n"
        "If you have a mandatory move, just type the corresponding number");
      continue;
    }

    if (res == 3) //save
    {
      if (strlen(filename) == 0)
      {
        // name by default, free crash if static
        filename[0] = 's';
        filename[1] = 'a';
        filename[2] = 'v';
        filename[3] = 'e';
      }
      if (0 != write_board_to_file(board, filename))
        print_error("Can not write board to file");
    }

    if (res == 4) // undo
    {
      if (list_len(board->undo) > 0)
        undo_move(board);
      else
        print_error("No previous move");
      printBoard(board);
    }
  }

  if (board->player == PLAYER_WHITE)
    printf("Black won!\n");
  else
    printf("White won!\n");

//-------free SDL------------------------------------------------------------//
  SDL_FreeSurface(screen);
  SDL_FreeSurface(W);
  SDL_FreeSurface(B);
  SDL_FreeSurface(B_WP);
  SDL_FreeSurface(B_BP);
  SDL_FreeSurface(B_WK);
  SDL_FreeSurface(B_BK);
//-------free SDL------------------------------------------------------------//

  free_moves(moves_list);
  free_moves(board->undo);
  free(board);
  free(curLine);
  free(curCol);
  free(destLine);
  free(destCol);
  free(filename);
  return 0;
}
