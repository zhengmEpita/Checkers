# include <stdio.h>
# include <stdlib.h>
# include "board.h"
# include "piece.h"
# include "list.h"
# include "find_move.h"
# include <sys/types.h>
# include <sys/stat.h>
# include <unistd.h>

int is_in_array(struct coords captures[], int nb_captures, int x, int y)
{
  for (int i = 0; i < nb_captures; ++i)
  {
    if (captures[i].x == x && captures[i].y == y)
      return 1;
  }
  return 0;
}

/*
 * Return 0 if no jump possible, else return 1
 * dx dy: relative moves (-1 or 1)
 */
int prise_simple_move(struct board *b,
                      int cur_piece,
                      int x, int y,
                      int dx, int dy,
                      struct moves *moves,
                      struct move_seq *move_seq)
{
  if (is_out_of_board(x + dx, y + dy)
      || is_out_of_board(x + dx + dx, y + dy + dy))
  {
    return 0;
  }

  int jumped_piece = b->cells[x + dx][y + dy].data;
  int dest_piece = b->cells[x + 2*dx][y + 2*dy].data;

  if ((jumped_piece * cur_piece >= 0)
   || (dest_piece != 0)
   || (is_in_array(move_seq->captures, move_seq->nb_captures, x + dx, y + dy)))
  {
    return 0;
  }

  printf("Capture at %d %d from %d %d possible\n", x + dx, y + dy, x, y);

  struct move_seq *elm = malloc(sizeof (struct move_seq));
  elm->orig.x = x;
  elm->orig.y = y;
  elm->capt.x = x + dx;
  elm->capt.y = y + dy;
  elm->dest.x = x + 2*dx;
  elm->dest.y = y + 2*dy;

  seq_push_front(move_seq, elm);

  move_seq->captures[move_seq->nb_captures].x = x + dx;
  move_seq->captures[move_seq->nb_captures].y = y + dy;
  move_seq->nb_captures++;

  if (0 == build_move_seq(b, cur_piece, x + 2*dx, y + 2*dy, moves, move_seq)
      && (move_seq->nb_captures > 0)) // end of sequence
  {
    if (moves_insert(moves, move_seq))
    {
      int ll = list_len(moves);
      printf("  length of moves list after insert =  %d\n", ll);
    }

    else
    {
      puts("  Fail to insert, not the best move\n");
    }
  }

  return 1;
}

/*
 * return 1 if a sequence is found, else 0
 */
int build_move_seq(struct board *b,
                   int cur_piece,
                   int x, int y,
                   struct moves *moves,
                   struct move_seq *move_seq)

{
  if (move_seq == NULL) // create a new sequence
  {
    move_seq = malloc(sizeof (struct move_seq));
    seq_init(move_seq);
  }

  struct move_seq *seq_copy = copy(move_seq); // save move_seq
  int res1, res2, res3, res4;

  // if True seq_copy has been modified, a copy is created
  if ( (res1 = prise_simple_move(b, cur_piece, x, y, -1, -1, moves, seq_copy)))
    seq_copy = copy(move_seq);

  if ( (res2 = prise_simple_move(b, cur_piece, x, y, -1, 1, moves, seq_copy)))
    seq_copy = copy(move_seq);

  if ( (res3 = prise_simple_move(b, cur_piece, x, y, 1, -1, moves, seq_copy)))
    seq_copy = copy(move_seq);

  res4 = prise_simple_move(b, cur_piece, x, y, 1, 1, moves, seq_copy);

  if (res1 == 0 && res2 == 0 && res3 == 0 && res4 == 0)
    return 0;

  return 1;
}

/*
 * return a list of moves
 */
struct moves *build_moves(struct board *b)
{
  struct moves *moves = malloc(sizeof (struct moves));
  moves_init(moves, NULL);

  for (int x = 0; x < 10; ++x)
  {
    for (int y = 0; y < 10; ++y)
    {
      if (get_color(b->cells[x][y].data) == b->player)
      {
        if (1 == build_move_seq(b, b->cells[x][y].data, x, y, moves, NULL))
        {
          printf("\nMandatory jump!\n\n");
        }
      }
    }
  }

  return moves;
}
