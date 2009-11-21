#include "bbs.h"
#include "chess.h"
#include<stdarg.h> 

rc_t chc_from, chc_to, chc_select, chc_cursor;
int chc_my;
char chc_warnmsg[64];
int chc_selected,chc_turn, chc_firststep;
int chc_lefttime;
int chc_hiswin, chc_hislose, chc_histie;

static int chc_ipass = 0, chc_hepass = 0;

chc_star my;

user_info *uin;

int chc_load(char id[14],int i) {
 FILE *fp;
 char buf[200];
 int chc_win,chc_lose,chc_tie;
 sprintf(buf,"/home/bbs/home/%s/.chc",id);
 if(fp = fopen(buf,"r"))
 {
  fscanf(fp,"%d %d %d", &chc_win, &chc_lose, &chc_tie);
  fclose(fp);
 }
 else
 {
  chc_win = 0;
  chc_lose = 0;
  chc_tie = 0;
 }
 if(i == 1) //my
 {
   my.chc_win = chc_win;
   my.chc_lose = chc_lose;
   my.chc_tie = chc_tie;
 }
 else if(i == 2)
 {
    chc_hiswin = chc_win;
    chc_hislose = chc_lose;
    chc_histie = chc_tie;
 }

}

int chc_save(char id[14],int i) {
 FILE *fp;
 char buf[200];
 int chc_win,chc_lose,chc_tie;
 sprintf(buf,"/home/bbs/home/%s/.chc", id);

 if (i == 1)
 {
   chc_win = my.chc_win;
   chc_lose = my.chc_lose;
   chc_tie = my.chc_tie;
 }
 else if(i == 2)
 {
     chc_win = chc_hiswin;
     chc_lose = chc_hislose;
     chc_tie = chc_histie;
 }
 fp = fopen(buf,"w");
 fprintf(fp,"%d %d %d", chc_win, chc_lose, chc_tie);
 fclose(fp);
}


int chc_recvmove(int s) {
    drc_t buf;

    if(read(s, &buf, sizeof(buf)) != sizeof(buf))
        return 1;
    chc_from = buf.from, chc_to = buf.to;
    return 0;
}

void chc_sendmove(int s) {
    drc_t buf;

    buf.from = chc_from, buf.to = chc_to;
    write(s, &buf, sizeof(buf));
}


void chc_init_board(board_t board) {
    memset(board, 0, sizeof(board_t));
    board[0][4] = CHE(1, chc_my ^ 1);                    /* 將 */
    board[0][3] = board[0][5] = CHE(2, chc_my ^ 1);      /* 士 */
    board[0][2] = board[0][6] = CHE(3, chc_my ^ 1);      /* 象 */
    board[0][0] = board[0][8] = CHE(4, chc_my ^ 1);      /* 車 */
    board[0][1] = board[0][7] = CHE(5, chc_my ^ 1);      /* 馬 */
    board[2][1] = board[2][7] = CHE(6, chc_my ^ 1);      /* 包 */
    board[3][0] = board[3][2] = board[3][4] =
        board[3][6] = board[3][8] = CHE(7, chc_my ^ 1);  /* 卒 */

    board[9][4] = CHE(1, chc_my);                    /* 帥 */
    board[9][3] = board[9][5] = CHE(2, chc_my);      /* 仕 */
    board[9][2] = board[9][6] = CHE(3, chc_my);      /* 相 */
    board[9][0] = board[9][8] = CHE(4, chc_my);      /* 車 */
    board[9][1] = board[9][7] = CHE(5, chc_my);      /* 傌 */
    board[7][1] = board[7][7] = CHE(6, chc_my);      /* 炮 */
    board[6][0] = board[6][2] = board[6][4] =
        board[6][6] = board[6][8] = CHE(7, chc_my);  /* 兵 */
}

void chc_movechess(board_t board) {
    board[chc_to.r][chc_to.c] = board[chc_from.r][chc_from.c];
    board[chc_from.r][chc_from.c] = 0;
}

static int dist(rc_t from, rc_t to, int rowcol) {
    int d;

    d = rowcol ? from.c - to.c : from.r - to.r;
    return d > 0 ? d : -d;
}

static int between(board_t board, rc_t from, rc_t to, int rowcol) {
    int i, rtv = 0;

    if(rowcol) {
        if(from.c > to.c)
            i = from.c, from.c = to.c, to.c = i;
        for(i = from.c + 1; i < to.c; i++)
            if(board[to.r][i]) rtv++;
    } else {
        if(from.r > to.r)
            i = from.r, from.r = to.r, to.r = i;
        for(i = from.r + 1; i < to.r; i++)
            if(board[i][to.c]) rtv++;
    }
    return rtv;
}

int chc_canmove(board_t board, rc_t from, rc_t to) {
    int i;
    int rd, cd, turn;

    rd = dist(from, to, 0);
    cd = dist(from, to, 1);
    turn = CHE_O(board[from.r][from.c]);

    /* general check */
    if(board[to.r][to.c] && CHE_O(board[to.r][to.c]) == turn)
        return 0;

    /* individual check */
    switch(CHE_P(board[from.r][from.c])) {
    case 1: /* 將 帥 */
        if(!(rd == 1 && cd == 0) &&
           !(rd == 0 && cd == 1))
            return 0;
        if((turn == (chc_my ^ 1) && to.r > 2) ||
           (turn == chc_my && to.r < 7) ||
            to.c < 3 || to.c > 5)
                return 0;
        break;
    case 2: /* 士 仕 */
        if(!(rd == 1 && cd == 1))
            return 0;
        if((turn == (chc_my ^ 1) && to.r > 2) ||
           (turn == chc_my && to.r < 7) ||
            to.c < 3 || to.c > 5)
                return 0;
        break;
    case 3: /* 象 相 */
        if(!(rd == 2 && cd == 2))
            return 0;
        if((turn == (chc_my ^ 1) && to.r > 4) ||
           (turn == chc_my && to.r < 5))
            return 0;
        /* 拐象腿 */
        if(board[CENTER(from.r, to.r)][CENTER(from.c, to.c)])
            return 0;
        break;
    case 4: /* 車 */
        if(!(rd > 0 && cd == 0) &&
           !(rd == 0 && cd > 0))
            return 0;
        if(between(board, from, to, rd == 0))
            return 0;
        break;
    case 5: /* 馬 傌 */
        if(!(rd == 2 && cd == 1) &&
           !(rd == 1 && cd == 2))
            return 0;
        /* 拐馬腳 */
        if(rd == 2) {
            if(board[CENTER(from.r, to.r)][from.c])
                return 0;
        } else {
            if(board[from.r][CENTER(from.c, to.c)])
                return 0;
        }
        break;
    case 6: /* 包 炮 */
        if(!(rd > 0 && cd == 0) &&
           !(rd == 0 && cd > 0))
            return 0;
        i = between(board, from, to, rd == 0);
        if((i > 1) ||
           (i == 1 && !board[to.r][to.c]) ||
           (i == 0 && board[to.r][to.c]))
           return 0;
        break;
    case 7: /* 卒 兵 */
        if(!(rd == 1 && cd == 0) &&
           !(rd == 0 && cd == 1))
            return 0;
        if(((turn == (chc_my ^ 1) && to.r < 5) ||
            (turn == chc_my && to.r > 4)) &&
            cd != 0)
            return 0;
        if((turn == (chc_my ^ 1) && to.r < from.r) ||
           (turn == chc_my && to.r > from.r))
            return 0;
        break;
    }
    return 1;
}

static void findking(board_t board, int turn, rc_t *buf) {
    int i, r, c;

    r = (turn == (chc_my ^ 1)) ? 0 : 7;
    for(i = 0; i < 3; r++, i++)
        for(c = 3; c < 6; c++)
            if(CHE_P(board[r][c]) == 1 &&
               CHE_O(board[r][c]) == turn) {
                buf->r = r, buf->c = c;
                return ;
            }
}

int chc_iskfk(board_t board) {
    rc_t from, to;

    findking(board, 0, &to);
    findking(board, 1, &from);
    if(from.c == to.c && between(board, from, to, 0) == 0)
        return 1;
    return 0;
}

int chc_ischeck(board_t board, int turn) {
    rc_t from, to;

    findking(board, turn, &to);
    for(from.r = 0;from.r < BRD_ROW; from.r++)
        for(from.c = 0; from.c < BRD_COL; from.c++)
            if(board[from.r][from.c] &&
               CHE_O(board[from.r][from.c]) != turn)
                if(chc_canmove(board, from, to))
                    return 1;
    return 0;
}


static void showstep(board_t board) {
    int turn, fc, tc, eatten;
    char *dir;

    turn = CHE_O(board[chc_from.r][chc_from.c]);
    fc = (turn == (chc_my ^ 1) ? chc_from.c + 1 : 9 - chc_from.c);
    tc = (turn == (chc_my ^ 1) ? chc_to.c + 1 : 9 - chc_to.c);
    if(chc_from.r == chc_to.r)
        dir = "平";
    else {
        if(chc_from.c == chc_to.c)
            tc = chc_from.r - chc_to.r;
        if(tc < 0) tc = -tc;

        if((turn == (chc_my ^ 1) && chc_to.r > chc_from.r) ||
           (turn == chc_my && chc_to.r < chc_from.r))
            dir = "進";
        else
            dir = "退";
    }
    prints("%s%s%s%s%s",
           turn == 0 ? BLACK_COLOR : RED_COLOR,
           chess_str[turn][CHE_P(board[chc_from.r][chc_from.c])],
           num_str[fc], dir, num_str[tc]);
    eatten = board[chc_to.r][chc_to.c];
    if(eatten)
        prints("： %s%s",
               CHE_O(eatten) == 0 ? BLACK_COLOR : RED_COLOR,
               chess_str[CHE_O(eatten)][CHE_P(eatten)]);
    prints("\033[m");
}
void chc_drawline(board_t board, int line) {
    int i, j;

    move(line, 0);
    clrtoeol();
    if(line == 0) {
        prints("\033[1;46m   象棋對奕   \033[45m%30s VS %-30s\033[m",
               cuser.userid, uin->userid);
    } else if(line >= 3 && line <= 21) {
        outs("   ");
        for(i = 0; i < 9; i++) {
            j = board[RTL(line)][i];
            if((line & 1) == 1 && j) {
                if(chc_selected &&
                   chc_select.r == RTL(line) && chc_select.c == i)
                    prints("%s%s\033[m",
                           CHE_O(j) == 0 ? BLACK_REVERSE : RED_REVERSE,
                           chess_str[CHE_O(j)][CHE_P(j)]);
                else
                    prints("%s%s\033[m",
                           CHE_O(j) == 0 ? BLACK_COLOR : RED_COLOR,
                           chess_str[CHE_O(j)][CHE_P(j)]);
            } else
                prints("%c%c", chess_brd[line - 3][i * 4],
                       chess_brd[line - 3][i * 4 + 1]);
            if(i != 8)
                prints("%c%c", chess_brd[line - 3][i * 4 + 2],
                       chess_brd[line - 3][i * 4 + 3]);
        }
        outs("        ");

        if(line >= 3 && line < 3 + dim(hint_str)) {
            outs(hint_str[line - 3]);
        } else if(line == SIDE_ROW) {
            prints("\033[1m你是%s%s\033[m",
                   chc_my == 0 ? BLACK_COLOR : RED_COLOR,
                   turn_str[chc_my]);
        } else if(line == TURN_ROW) {
            prints("%s%s\033[m",
                   TURN_COLOR,
                   chc_my == chc_turn ? "輪到你下棋了" : "等待對方下棋");
        } else if(line == STEP_ROW && !chc_firststep) {
            showstep(board);
        } else if(line == TIME_ROW) {
            prints("剩餘時間 %d:%02d", chc_lefttime / 60, chc_lefttime %60);
        } else if(line == WARN_ROW) {
            outs(chc_warnmsg);
        } else if(line == MYWIN_ROW) {
            prints("\033[1;33m%12.12s    "
                   "\033[1;31m%2d\033[37m勝 "
                   "\033[34m%2d\033[37m敗 "
                   "\033[36m%2d\033[37m和\033[m",
                   cuser.userid,
                   my.chc_win, my.chc_lose , my.chc_tie);
        } else if(line == HISWIN_ROW) {
            prints("\033[1;33m%12.12s    "
                   "\033[1;31m%2d\033[37m勝 "
                   "\033[34m%2d\033[37m敗 "
                   "\033[36m%2d\033[37m和\033[m",
                   uin->userid,
                   chc_hiswin, chc_hislose , chc_histie);
        }
    } else if(line == 2 || line == 22) {
        outs("   ");
        if(line == 2)
            for(i = 1; i <= 9; i++)
                prints("%s  ", num_str[i]);
        else
            for(i = 9; i >= 1; i--)
                prints("%s  ", num_str[i]);
    }
}

void chc_redraw(board_t board) {
    int i;

    for(i = 0; i <= 22; i++)
        chc_drawline(board, i);
}

static int hisplay(int s, board_t board, board_t tmpbrd) {
    int start_time;
    int endgame = 0, endturn = 0;

    start_time = time(NULL);
    while(!endturn) {
        chc_lefttime = CHC_TIMEOUT - (time(NULL) - start_time);
        if(chc_lefttime < 0) {
            chc_lefttime = 0;

            /* to make him break out igetkey() */
            chc_from.r = -2;
            chc_sendmove(s);
        }
        chc_drawline(board, TIME_ROW);
        move(1, 0);
        oflush();
        switch(igetkey()) {
        case 'q':
            endgame = 2;
            endturn = 1;
           break;
        case 'p':
            if(chc_hepass) {
                chc_from.r = -1;
                chc_sendmove(s);
                endgame = 3;
                endturn = 1;
            }
            break;
        case I_OTHERDATA:
            if(chc_recvmove(s)) { /* disconnect */
                endturn = 1;
                endgame = 1;
            } else {
                if(chc_from.r == -1) {
                    chc_hepass = 1;
                    strcpy(chc_warnmsg, "\033[1;33m要求和局!\033[m");
                    chc_drawline(board, WARN_ROW);
                } else {
                    chc_from.r = 9 - chc_from.r, chc_from.c = 8 -chc_from.c;
                    chc_to.r = 9 - chc_to.r, chc_to.c = 8 - chc_to.c;
                    chc_cursor = chc_to;
                    if(CHE_P(board[chc_to.r][chc_to.c]) == 1)
                        endgame = 2;
                    endturn = 1;
                    chc_hepass = 0;
                    chc_drawline(board, STEP_ROW);
                    chc_movechess(board);
                    chc_drawline(board, LTR(chc_from.r));
                    chc_drawline(board, LTR(chc_to.r));
                }
            }
            break;
        }
    }
    return endgame;
}

static int myplay(int s, board_t board, board_t tmpbrd) {
    int ch, start_time;
    int endgame = 0, endturn = 0;

    chc_ipass = 0, chc_selected = 0;
    start_time = time(NULL);
    chc_lefttime = CHC_TIMEOUT - (time(NULL) - start_time);
    bell();
    while(!endturn) {
        chc_drawline(board, TIME_ROW);
        chc_movecur(chc_cursor.r, chc_cursor.c);
        oflush();
        ch = igetkey();
        chc_lefttime = CHC_TIMEOUT - (time(NULL) - start_time);
        if(chc_lefttime < 0)
            ch = 'q';
        switch(ch) {
        case I_OTHERDATA:
            if(chc_recvmove(s)) { /* disconnect */
                endgame = 1;
                endturn = 1;
            } else if(chc_from.r == -1 && chc_ipass) {
                endgame = 3;
                endturn = 1;
            }
            break;
        case KEY_UP:
            chc_cursor.r--;
            if(chc_cursor.r < 0)
                chc_cursor.r = BRD_ROW - 1;
            break;
        case KEY_DOWN:
            chc_cursor.r++;
            if(chc_cursor.r >= BRD_ROW)
                chc_cursor.r = 0;
            break;
        case KEY_LEFT:
            chc_cursor.c--;
            if(chc_cursor.c < 0)
                chc_cursor.c = BRD_COL - 1;
            break;
        case KEY_RIGHT:
            chc_cursor.c++;
            if(chc_cursor.c >= BRD_COL)
                chc_cursor.c = 0;
            break;
        case 'q':
            endgame = 2;
            endturn = 1;
            break;
        case 'p':
            chc_ipass = 1;
            chc_from.r = -1;
            chc_sendmove(s);
            strcpy(chc_warnmsg, "\033[1;33m要求和棋!\033[m");
            chc_drawline(board, WARN_ROW);
            bell();
            break;
        case '\r':
        case '\n':
        case ' ':
            if(chc_selected) {
                if(chc_cursor.r == chc_select.r &&
                   chc_cursor.c == chc_select.c) {
                    chc_selected = 0;
                    chc_drawline(board, LTR(chc_cursor.r));
                } else if(chc_canmove(board, chc_select, chc_cursor)) {
                    if(CHE_P(board[chc_cursor.r][chc_cursor.c]) == 1)
                        endgame = 1;
                    chc_from = chc_select;
                    chc_to = chc_cursor;
                    if(!endgame) {
                        memcpy(tmpbrd, board, sizeof(board_t));
                        chc_movechess(tmpbrd);
                    }
                    if(endgame || !chc_iskfk(tmpbrd)) {
                        chc_drawline(board, STEP_ROW);
                        chc_movechess(board);
                        chc_sendmove(s);
                        chc_selected = 0;
                        chc_drawline(board, LTR(chc_from.r));
                        chc_drawline(board, LTR(chc_to.r));
                        endturn = 1;
                    } else {
                        strcpy(chc_warnmsg,"\033[1;33m不可以王見王\033[m");
                        bell();
                        chc_drawline(board, WARN_ROW);
                    }
                }
            } else if(board[chc_cursor.r][chc_cursor.c] &&
                      CHE_O(board[chc_cursor.r][chc_cursor.c]) == chc_turn) {
                chc_selected = 1;
                chc_select = chc_cursor;
                chc_drawline(board, LTR(chc_cursor.r));
            }
            break;
        }
    }
    return endgame;
}
static void mainloop(int s, board_t board) {
    int endgame;
    board_t tmpbrd;
    play_func_t play_func[2];

    play_func[chc_my] = myplay;
    play_func[chc_my ^ 1] = hisplay;
    for(chc_turn = 1, endgame = 0; !endgame; chc_turn ^= 1) {
        chc_firststep = 0;
        chc_drawline(board, TURN_ROW);
        if(chc_ischeck(board, chc_turn)) {
            strcpy(chc_warnmsg, "\033[1;31m將軍!\033[m");

            bell();
        } else
            chc_warnmsg[0] = 0;
        chc_drawline(board, WARN_ROW);
        endgame = play_func[chc_turn](s, board, tmpbrd);
    }

    if(endgame == 1) {
        strcpy(chc_warnmsg, "對方認輸了!");
        my.chc_win++;
        chc_hislose++;
    } else if(endgame == 2) {
        strcpy(chc_warnmsg, "你認輸了!");
        my.chc_lose++;
        chc_hiswin++;
    } else {
        strcpy(chc_warnmsg, "和棋");
        my.chc_tie++;
        chc_histie++;
    }

    chc_save(cuser.userid,1);
    chc_save(uin->userid,2);
    chc_drawline(board, WARN_ROW);
    bell();
    oflush();
}
static void chc_init(int s, board_t board) {

    user_info *my = uin;
    setutmpmode(CHESS);
    clear();
    chc_warnmsg[0] = 0;
    chc_my = currutmp->turn;

    chc_firststep = 1;
    chc_init_board(board);
    chc_redraw(board);
    chc_cursor.r = 9, chc_cursor.c = 0;
    add_io(s, 0);
    chc_load(cuser.userid,1);

    if(my->turn) chc_recvmove(s);
    chc_load(uin->userid,2);

//    if(!currutmp->turn) {
        chc_sendmove(s);
//        chc_hislose++;
//    }

    chc_redraw(board);
}



int chc(int s,user_info *uin2) {
    board_t board;
    uin = uin2;
    chc_init(s, board);
    mainloop(s, board);
    close(s);
    add_io(0, 0);
    if(chc_my) pressanykey("下次再來繼續對決吧....:p"); 
}

//#include<stdarg.h>
int va_chc(va_list pvar)
{
 int fd;
 user_info *uin;
 fd = va_arg(pvar, int);
 uin = va_arg(pvar, user_info *);
 return chc( fd, uin);
}

