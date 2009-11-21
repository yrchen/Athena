//chc.c skybinary.bbs@starriver.twbbs.org
#include "bbs.h"


//struct.h of ptt
#define BRD_ROW           10
#define BRD_COL           9
typedef int board_t[BRD_ROW][BRD_COL];

typedef struct {
    int r, c;
} rc_t;


//chc_draw.c
#define SIDE_ROW          10
#define TURN_ROW          11
#define STEP_ROW          12
#define TIME_ROW          13
#define WARN_ROW          15
#define MYWIN_ROW         17
#define HISWIN_ROW        18

static char *turn_str[2] = {"黑的", "紅的"};

static char *num_str[10] = {
    "", "一", "二", "三", "四", "五", "六", "七", "八", "九"
};

static char *chess_str[2][8] = {
    /* 0     1     2     3     4     5     6     7 */
    {"  ", "將", "士", "象", "車", "馬", "包", "卒"},
    {"  ", "帥", "仕", "相", "車", "傌", "炮", "兵"}
};

static char *chess_brd[BRD_ROW * 2 - 1] = {
    /*0   1   2   3   4   5   6   7   8*/
    "┌─┬─┬─┬─┬─┬─┬─┬─┐", /* 0 */
    "│  │  │  │＼│／│  │  │  │",
    "├─┼─┼─┼─┼─┼─┼─┼─┤", /* 1 */
    "│  │  │  │／│＼│  │  │  │",
    "├─┼─┼─┼─┼─┼─┼─┼─┤", /* 2 */
    "│  │  │  │  │  │  │  │  │",
    "├─┼─┼─┼─┼─┼─┼─┼─┤", /* 3 */
    "│  │  │  │  │  │  │  │  │",
    "├─┴─┴─┴─┴─┴─┴─┴─┤", /* 4 */
    "│  楚    河          漢    界  │",
    "├─┬─┬─┬─┬─┬─┬─┬─┤", /* 5 */
    "│  │  │  │  │  │  │  │  │",
    "├─┼─┼─┼─┼─┼─┼─┼─┤", /* 6 */
    "│  │  │  │  │  │  │  │  │",
    "├─┼─┼─┼─┼─┼─┼─┼─┤", /* 7 */
    "│  │  │  │＼│／│  │  │  │",
    "├─┼─┼─┼─┼─┼─┼─┼─┤", /* 8 */
    "│  │  │  │／│＼│  │  │  │",
    "└─┴─┴─┴─┴─┴─┴─┴─┘"  /* 9 */
};

static char *hint_str[] = {
    "  q      認輸離開",
    "  p      要求和棋",
    "方向鍵   移動遊標",
    "Enter    選擇/移動"
};

void chc_movecur(int r, int c) {
    move(r * 2 + 3, c * 4 + 4);
}

#define BLACK_COLOR       "\033[1;36m"
#define RED_COLOR         "\033[1;31m"
#define BLACK_REVERSE     "\033[1;37;46m"
#define RED_REVERSE       "\033[1;37;41m"
#define TURN_COLOR        "\033[1;33m"

//chc_play.c
typedef int (*play_func_t)(int, board_t, board_t);
#define CHC_TIMEOUT           300
#define SIDE_ROW          10
#define TURN_ROW          11
#define STEP_ROW          12
#define TIME_ROW          13
#define WARN_ROW          15
#define MYWIN_ROW         17
#define HISWIN_ROW        18

//chc_rule.c
#define CENTER(a, b)      (((a) + (b)) >> 1)

//chc_net.c
typedef struct drc_t {
    rc_t from, to;
} drc_t;

//chc in ptt 
#define CHE_O(c)          ((c) >> 3)
#define CHE_P(c)          ((c) & 7)
#define RTL(x)            (((x) - 3) >> 1)
#define LTR(x)            ((x) * 2 + 3)
#define dim(x)          (sizeof(x) / sizeof(x[0]))
#define CHE(a, b)         ((a) | ((b) << 3))


//chc for star version.

typedef struct chc_star {
    unsigned short chc_win;
    unsigned short chc_lose;
    unsigned short chc_tie;
} chc_star;
