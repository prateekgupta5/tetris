#include <unistd.h>
#include <ncurses.h>
#include <string.h>
#include <algorithm>
#include <random>
#include <thread>
#include <chrono>

const int X = 0, Y = 1, R = 2; //everything exept boards are x, y

const uint8_t NO_PIECE = 1;
const uint8_t L        = 2;
const uint8_t J        = 3;
const uint8_t I        = 4;
const uint8_t T        = 5;
const uint8_t Z        = 6;
const uint8_t S        = 7;

//SHAPES[piece][rot][point][x/y]
const int8_t SHAPES[][4][5][2] = {
    { //0
        {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
        {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
        {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
        {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
    },
    
    { //1 NO_PIECE
        {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
        {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
        {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
        {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
    },

    { //2 L
        { //0pi/2
            { 0, 0},
            { 0, 1},
            { 0, 2}, { 1, 2},

            {2, 3} //2 long 3 tall
        },

        { //1pi/2
            { 0, 0}, { 1, 0}, { 2, 0},
            { 0, 1},

            {3, 2} //3 long 2 tall
        },

        { //2pi/2
            { 0, 0}, { 1, 0},
                     { 1, 1},
                     { 1, 2},

            {2, 3} //2 long 3 tall
        },

        { //3pi/2

                              { 2, 0},
            { 0, 1}, { 1, 1}, { 2, 1},

            {3, 2} //3 long 2 tall
        }
    },

    {//3 J
        { //0pi/2
                     { 0, 0},
                     { 0, 1},
            {-1, 2}, { 0, 2},

            {2, 3} //2 long 3 tall
        },

        { //1pi/2
            { 0, 0},
            { 0, 1}, { 1, 1}, { 2, 1},

            {3, 2} //3 long 2 tall
        },

        { //2pi/2
            { 0, 0}, { 1, 0},
            { 0, 1},
            { 0, 2},

            {2, 3} //2 long 3 tall
        },

        { //3pi/2
            { 0, 0}, { 1, 0}, { 2, 0},
                              { 2, 0},

            {3, 2} //3 long 2 tall
        }
    },

    {//4 I
        { //0pi/2
            { 0,-3},
            { 0,-2},
            { 0,-1},
            { 0, 0},

            {1, 4} //1 long 4 tall
        },

        { //1pi/2
            { 0, 0}, { 1, 0}, { 2, 0}, { 3, 0},

            {4, 1} //3 long 1 tall
        },

        { //2pi/2
            { 0, 0},
            { 0, 1},
            { 0, 2},
            { 0, 3},

            {1, 4} //1 long 4 tall
        },

        { //3pi/2
            { -3, 0}, {-2, 0}, {-1, 0}, {0, 0},

            {4, 1} //3 long 1 tall
        }
    },

    {//5 T
        { //0pi/2
                     { 0,-1},
            {-1, 0}, { 0, 0}, { 1, 0},

            {3, 2} //3 long 4 tall
        },

        { //1pi/2
            { 0,-1},
            { 0, 0}, { 1, 0},
            { 0, 1},

            {2, 3} //2 long 3 tall
        },

        { //2pi/2
            { 0,-1}, { 0, 0}, { 1, 0},
                     { 0, 1},

            {3, 2} //3 long 4 tall
        },

        { //3pi/2
                     { 0,-1},
            {-1, 0}, { 0, 0},
                     { 0, 1},

            {2, 3} //2 long 3 tall
        }
    },

    { //6 Z
        { //0pi/2
            {-1,-1}, { 0,-1},
                     { 0, 0}, { 1, 0},

            {2, 2} //2 long 2 tall
        },

        { //1pi/2
                     { 1,-1},
            { 0, 0}, { 1, 0},
            { 0, 1},

            {2, 2} //2 long 2 tall
        },

        { //2pi/2
            {-1, 0}, { 0, 0},
                     { 0, 1}, { 1, 1},

            {2, 2} //2 long 2 tall
        },

        { //3pi/2
                     { 0,-1},
            {-1, 0}, { 0, 0},
            {-1, 1},

            {2, 2} //2 long 2 tall
        }
    },

    { //7 S
        { //0pi/2
            {-1, 0}, {-1, 1},
            { 0, 0}, { 0, 1},

            {2, 2} //2 long 2 tall
        },

        { //1pi/2
            { 0, 0}, { 1, 0},
            { 0, 1}, { 1, 1},

            {2, 2} //2 long 2 tall
        },

        { //2pi/2
            {-1, 0}, { 0, 0},
            {-1, 1}, { 0, 1},

            {2, 2} //2 long 2 tall
        },

        { //3pi/2
            {-1,-1}, {-1,-1},
            {-1, 0}, { 0, 0},

            {2, 2} //2 long 2 tall
        }
    }

};

inline void INIT_COLOR () {                              
    start_color();                                  
    init_pair(NO_PIECE, COLOR_BLACK, COLOR_BLACK  );
    init_pair(L,        COLOR_BLACK, COLOR_GREEN  );
    init_pair(J,        COLOR_BLACK, COLOR_RED    );
    init_pair(I,        COLOR_BLACK, COLOR_BLUE   );
    init_pair(T,        COLOR_BLACK, COLOR_MAGENTA);
    init_pair(Z,        COLOR_BLACK, COLOR_YELLOW );
    init_pair(S,        COLOR_BLACK, COLOR_CYAN   );
}


const uint8_t  MAX_ROWS_AT_ONCE = 4; //no piece is 5 in one dir
const uint16_t POINTS_FOR_SCORING [MAX_ROWS_AT_ONCE + 1] = {
    0,   // pts for 0 rows
    100, // pts for 1 row
    200, // pts for 2 rows 
    400, // pts for 3 rows
    600  // pts for 4 rows
};


const uint8_t NUM_ROWS = 10;
const uint8_t NUM_COLS = 10; //sizeof( a row )
uint8_t board[NUM_ROWS][NUM_COLS]; //y, x
const int SPAWN_POS[2] = {NUM_COLS/2, NUM_ROWS/2};

uint8_t queueDisplay [16][3]; //y, x
uint8_t storeDisplay [4][3]; //y, x

inline void INIT_BOARD () {
    memset(board, 1, NUM_ROWS * NUM_COLS);
    memset(queueDisplay, 1, 36);
    memset(storeDisplay, 1, 9);
}


long score = 0;
char uin = '\0'; //user in
bool isRunning = true;

const char MOVE_L   = 'a';
const char MOVE_R   = 'd';
const char ROTATE_L = 'q';
const char ROTATE_R = 'e';
const char DROP     = 's';
const char SWAP     = 'w';
const char QUIT     = '`';


uint8_t c_tet, s_tet = NO_PIECE; //current and stored tetrimo
uint8_t tet_xyr[3] = {0, 0, 0};  //x,y of tetrimo origin, and rotaion or piece
uint8_t queue[4] = {};

std::uniform_int_distribution<std::mt19937::result_type> nextTet;
std::mt19937 rng;

#define INIT_QUEUE                                                       \
rng = std::mt19937();                                                    \
rng.seed(time(NULL));                                                        \
nextTet = std::uniform_int_distribution<std::mt19937::result_type>(2,7); \
for (int i = 0; i < 4; ++i) {                                            \
    queue[i] = nextTet(rng);                                             \
    printf("ur mom %p\n", queueDisplay);\
    addToBoard((uint8_t**)queueDisplay, 1, 4*i, queue[i]);               \
}


WINDOW *mainWin, *scoreWin, *storeWin, *queueWin, *ctrlsWin;
inline void INIT_WINDOWS () {
    storeWin = newwin(3+4, 6+4 , 0,0);
    scoreWin = newwin(1+4, 14+4, 2,8);
    mainWin  = newwin(NUM_ROWS+2, (NUM_COLS*2)+2, 5, 0);
    queueWin = newwin(16+4, 6+4, NUM_ROWS+2, (NUM_COLS*2)+2);

    box(storeWin, 0,0);
    box(scoreWin, 0,0);
    box(mainWin , 0,0);
    box(queueWin, 0,0);
}


#define PEICES SHAPES[tetrino][0]
inline void addToBoard(uint8_t** b, int8_t stx, int8_t sty, int8_t tetrino) {
    printf("%d | %d | bord: %p\n", tetrino, (int)PEICES[0][1], b);
    printf("%d\n", sty + PEICES[0][Y]);
    if(tetrino == I) sty+=3;
    else if (tetrino == Z || tetrino == T || tetrino == S) sty++;
    printf("%d, %d\n", sty + PEICES[0][Y], stx + PEICES[0][X]);

    for (int i = 0; i < 4; ++i) {
        b
          [sty + PEICES[i][Y]]
          [stx + PEICES[i][X]]
        = tetrino;
    }
}
#undef PEICES

int main () {
    initscr();
    clear();
    INIT_COLOR();
    INIT_QUEUE;
    INIT_BOARD();
    INIT_WINDOWS();
    c_tet = nextTet(rng);

    getch(); //wit for start

    nodelay(stdscr, true); //make getch non-blocking
    curs_set(0);           //make cursor invisible

    bool canGoDown = true;
    int gametick = 0;
    while (isRunning){  

        //get rid of any scored lines (here because we want the tetrino to desplay in its final position in the scored position during the delay period) 
        int linesScored = 0;
        bool isFull;

        for (int i = 0; i < NUM_ROWS; ++i) {
            isFull = true;
            for (int j = 0; j < NUM_COLS; j++) {
                if(board[i][j] == 0) { isFull = false; break; }
            }
            if(isFull) {
                linesScored++;
                memcpy(&board[1], board, NUM_COLS*i);
                memset(board, 0, NUM_COLS);
            }
        }

        score += POINTS_FOR_SCORING[linesScored];

        gametick++;

        //if it can go down and it is time to go down, , move down and updtate canGoDown
        if (canGoDown && !(gametick%4) ) {
            int8_t newY = tet_xyr[Y] + 1;
            int mostDown = SHAPES[c_tet][tet_xyr[R]][0][Y];

            #define POINT_X SHAPES[c_tet][tet_xyr[R]][i][X]
            #define POINT_Y SHAPES[c_tet][tet_xyr[R]][i][Y]

            for (int i = 0; i < 4; ++i) {
                if(POINT_Y > mostDown) continue;
                mostDown = POINT_Y;

                if ( //if it goes past the boundrys
                    POINT_Y + tet_xyr[Y] < 0 //if the y is past the bottom
                ) { canGoDown = false; break; }

                if ( //if it overlapps a piece
                    board
                        [POINT_Y + newY      ] //y
                        [POINT_X + tet_xyr[X]] //x
                    == NO_PIECE
                ) { canGoDown = false; break; }
            }

            if(canGoDown) tet_xyr[Y] = newY;

            #undef POINT_X
            #undef POINT_Y
        }

        //if it cant go down, add ts pieces to the board and check if user looses the game
        if(!canGoDown) {
            for (int i = 0; i < 4; ++i){
                if(
                    board
                        [SHAPES[c_tet][tet_xyr[R]][i][Y] + tet_xyr[Y]]
                        [SHAPES[c_tet][tet_xyr[R]][i][X] + tet_xyr[X]]
                    != NO_PIECE
                ) isRunning = 0;

                board [SHAPES[c_tet][tet_xyr[R]][i][Y]] [SHAPES[c_tet][tet_xyr[R]][i][X]] = c_tet;
            }


            c_tet = NO_PIECE;
        }

        //handel haing no current tetrino
        if(c_tet = NO_PIECE) {
            c_tet = queue[0];
            memcpy(queue, queue+1, 3);
            queue[3] = nextTet(rng);
            tet_xyr[X] = SPAWN_POS[X];
            tet_xyr[Y] = SPAWN_POS[Y];
            tet_xyr[R] = 0;
            canGoDown = true;
            addToBoard((uint8_t**)queueDisplay, 1, 12, queue[3]);

        }

        uin = getch();
        switch (uin){

            //ROTATION
            #define POINT_X SHAPES[c_tet][newRot][i][X]
            #define POINT_Y SHAPES[c_tet][newRot][i][Y]

            case ROTATE_L :{
                int8_t newRot = tet_xyr[R] == 0 ? 3 : tet_xyr[R] - 1;
                bool isValid = true;
                
                for (int i = 0; i < 4; ++i) { //check for alidity
                    if ( //if it goes past the boundrys
                        POINT_Y + tet_xyr[Y] > NUM_ROWS-1 //if the y is below the ground
                     || POINT_X + tet_xyr[X] < 0          //if the x is past the left wall
                     || POINT_X + tet_xyr[X] > NUM_COLS-1 //if the x is past the right wall
                    ) { isValid = false; break; }
                    
                    if ( //if it overlapps a piece
                        board
                            [POINT_Y + tet_xyr[Y]] //y
                            [POINT_X + tet_xyr[X]] //x
                        == NO_PIECE
                    ) { isValid = false; break; }
                }

                if (isValid) tet_xyr[R] = newRot;
                break;
            }

            case ROTATE_R :{
                int8_t newRot = tet_xyr[R] == 3 ? 0 : tet_xyr[R] + 1;
                bool isValid = true;
                
                for (int i = 0; i < 4; ++i) { //check for validity
                    if ( //if it goes past the boundrys
                           POINT_Y + tet_xyr[Y] > NUM_ROWS-1 //if the y is below the ground
                        || POINT_X + tet_xyr[X] < 0          //if the x is past the left wall
                        || POINT_X + tet_xyr[X] > NUM_COLS-1 //if the x is past the right wall
                    ) { isValid = false; break; }
                    
                    if ( //if it overlapps a piece
                        board
                            [POINT_Y + tet_xyr[Y]] //y
                            [POINT_X + tet_xyr[X]] //x
                        == NO_PIECE
                    ) { isValid = false; break; }
                }

                if (isValid) tet_xyr[R] = newRot;
                break;
            }

            #undef POINT_X
            #undef POINT_Y


            //MOVEMENT
            #define POINT_X SHAPES[c_tet][tet_xyr[R]][i][X]
            #define POINT_Y SHAPES[c_tet][tet_xyr[R]][i][X]

            case MOVE_L :{
                int8_t newX = tet_xyr[X] - 1;
                bool isValid = true;
                int mostLeft = SHAPES[c_tet][tet_xyr[R]][0][X];

                for (int i = 0; i < 4; ++i) {
                    if(POINT_X > mostLeft) continue;
                    mostLeft = POINT_X;

                    if ( //if it goes past the boundrys
                        POINT_X + newX < 0          //if the x is past the left wall
                    ) { isValid = false; break; }
                    
                    if ( //if it overlapps a piece
                        board
                            [POINT_Y + tet_xyr[Y]] //y
                            [POINT_X + newX      ] //x
                        == NO_PIECE
                    ) { isValid = false; break; }
                }

                if(isValid) tet_xyr[X] = newX;
                break;
            }

            case MOVE_R :{
                int8_t newX = tet_xyr[X] + 1;
                bool isValid = true;
                int mostRight = SHAPES[c_tet][tet_xyr[R]][0][X];

                for (int i = 0; i < 4; ++i) {
                    if(POINT_X > mostRight) continue;
                    mostRight = POINT_X;

                    if ( //if it goes past the boundrys
                        POINT_X + newX < 0          //if the x is past the left wall
                    ) { isValid = false; break; }
                    
                    if ( //if it overlapps a piece
                        board
                            [POINT_Y + tet_xyr[Y]] //y
                            [POINT_X + newX      ] //x
                        == NO_PIECE
                    ) { isValid = false; break; }
                }

                if(isValid) tet_xyr[X] = newX;
                break;
            }

            case DROP :{
                int8_t newY = tet_xyr[Y] + 1;
                int mostDown = SHAPES[c_tet][tet_xyr[R]][0][Y];

                for (int i = 0; i < 4; ++i) {
                    if(POINT_Y > mostDown) continue;
                    mostDown = POINT_Y;

                    if ( //if it goes past the boundrys
                        POINT_Y + tet_xyr[Y] < 0          //if the y is past the bottom
                    ) { canGoDown = false; break; }
                    
                    if ( //if it overlapps a piece
                        board
                            [POINT_Y + newY      ] //y
                            [POINT_X + tet_xyr[X]] //x
                        == NO_PIECE
                    ) { canGoDown = false; break; }
                }

                if(canGoDown) tet_xyr[Y] = newY;
                break;
            }

            #undef POINT_X
            #undef POINT_Y

            case SWAP :{
                std::swap(c_tet, s_tet);
                addToBoard((uint8_t**)storeDisplay, 1, 0, s_tet);
                break;
            }

            case QUIT:
                isRunning = false;
        }

        //add the current piece to the board for printing
        for (int i = 0; i < 4; ++i) {
            board
                [SHAPES[c_tet][tet_xyr[R]][i][Y] + tet_xyr[Y]]
                [SHAPES[c_tet][tet_xyr[R]][i][X] + tet_xyr[X]]
            = c_tet;
        }
    
        //prep widows for printing
        wclear(mainWin ); wattron(mainWin , NO_PIECE); wmove(mainWin , 1, 1); wrefresh(mainWin );
        wclear(scoreWin); wattron(scoreWin, NO_PIECE); wmove(scoreWin, 1, 1); wrefresh(scoreWin);
        wclear(storeWin); wattron(storeWin, s_tet   ); wmove(storeWin, 1, 1); wrefresh(storeWin);
        wclear(queueWin); wattron(queueWin, NO_PIECE); wmove(queueWin, 1, 1); wrefresh(queueWin);
  
        //print score
        wprintw(scoreWin, "score: %d", score);
        
        //print queue
        int current_peice_type = NO_PIECE;
        wprintw(queueWin, "\n");
        for (int i = 0; i < 12; i += 3) {
            for (int j = 0; j < 3; ++j) {
                wprintw(queueWin, " ");
                
                for (int k = 0; k < 3; ++k) {
                    if (current_peice_type != queueDisplay[i+j][k]) {
                        wattron(queueWin, queueDisplay[i+j][k]);
                        current_peice_type = queueDisplay[i+j][k];
                    }
                    wprintw(queueWin, "[]");
                }

                wprintw(queueWin, " \n");
            }
            wprintw(queueWin, "\n");
        }

        //print stored
        wprintw(storeWin, "\n");
        current_peice_type = NO_PIECE;
        for (int i = 0; i < NUM_ROWS; ++i) {
            wprintw(queueWin, " ");
            for (int j = 0; j < NUM_COLS; ++j) {
                wprintw(storeWin, "[]");
            }
            wprintw(storeWin, " \n");
        }
        wprintw(storeWin, "\n");

        //print main
        current_peice_type = NO_PIECE;
        for (int i = 0; i < NUM_ROWS; ++i) {
            for (int j = 0; j < NUM_COLS; ++j) {
                if (current_peice_type != board[i][j]) {
                    attron(board[i][j]);
                    current_peice_type = board[i][j];
                    wprintw(mainWin, "[]");
                }
            }
            wprintw(mainWin, "\n");
        }

        //refresh screen
        wrefresh(mainWin );
        wrefresh(scoreWin);
        wrefresh(storeWin);
        wrefresh(queueWin);
        refresh();
    
        //remove the current peice from the board after printing
        for (int i = 0; i < 4; ++i) {
            board
                [SHAPES[c_tet][tet_xyr[R]][i][Y] + tet_xyr[Y]]
                [SHAPES[c_tet][tet_xyr[R]][i][X] + tet_xyr[X]]
            = NO_PIECE;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    endwin();
    printf("thank you for playing tetris! Your score was %ld\n", score);
    return 0;
}