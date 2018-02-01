# define BP  1
# define BK  2
# define WP -1
# define WK -2

//def of the struct board
struct board
{
  int cells[10][10];
};


//Init the board with basic pawns
void boardInit(struct board *b);

//Display the board
void printBoard(int cells[10][10]);

int errManage(struct board *b, int curLine, int curCol, int destLine, int
destCol);


void move(struct board *b, int curLine, int curCol, int destLine, int destCol);


int deplacement(struct board *b, int curLine, int curCol, int destLine, int
destCol);
