/* $Id: film.h,v 1.5 2003/12/28 15:18:07 sby Exp $ */

/* include file for camera */

#define MOVIE_MAX       (200)           /* 動畫張數 */
#define MOVIE_SIZE      (128*1024)      /* 動畫 cache size */
#define MOVIE_LINES     (11)            /* 動畫最多有 11 列 */
#define FILM_SIZ        4000		/* max size for each film */

#define MOVIE2_NUM	(10)		/* 小看板個數 */
#define MOVIE2_LINES	(10)		/* 小看板最多有 10 列 */

typedef struct
{
  int shot[MOVIE_MAX]; /* Thor.980805: 合理範圍為 0..MOVIE_MAX - 1 */
  char film[MOVIE_SIZE];
  char movie2[MOVIE2_NUM * MOVIE2_LINES][256];	/* 小看板 */
} FCACHE;

/* sby: 如需增修 film 需另外修改 util/camera.c  */

#define FILM_MOVIE	 13	/* 定義 film 的總數 (0~12) */

#define FILM_WEL_0	 0	/* 進站畫面 myth_0 */
#define FILM_WEL_1	 1 	/* 進站畫面 myth_1 */
#define FILM_WEL_2	 2 	/* 進站畫面 myth_2 */
#define FILM_WEL_3	 3	/* 進站畫面 myth_3 */
#define FILM_WEL_4	 4	/* 進站畫面 myth_4 */
#define FILM_LOGIN       5	/* etc/Welcome_login */
#define FILM_WEL_BIRTH	 6	/* etc/Welcome_birth */
#define FILM_LOGOUT	 7	/* etc/Logout */
#define FILM_DAYPOST	 8	/* log/day */
#define FILM_POST	 9 	/* etc/post.note */
#define FILM_REGISTER	10 	/* etc/register */

#define FILM_GEM        11	/* ANNOUNCE.help */
#define FILM_MORE       12	/* MORE.help     */

