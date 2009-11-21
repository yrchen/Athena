/* $Id: film.h,v 1.5 2003/12/28 15:18:07 sby Exp $ */

/* include file for camera */

#define MOVIE_MAX       (200)           /* �ʵe�i�� */
#define MOVIE_SIZE      (128*1024)      /* �ʵe cache size */
#define MOVIE_LINES     (11)            /* �ʵe�̦h�� 11 �C */
#define FILM_SIZ        4000		/* max size for each film */

#define MOVIE2_NUM	(10)		/* �p�ݪO�Ӽ� */
#define MOVIE2_LINES	(10)		/* �p�ݪO�̦h�� 10 �C */

typedef struct
{
  int shot[MOVIE_MAX]; /* Thor.980805: �X�z�d�� 0..MOVIE_MAX - 1 */
  char film[MOVIE_SIZE];
  char movie2[MOVIE2_NUM * MOVIE2_LINES][256];	/* �p�ݪO */
} FCACHE;

/* sby: �p�ݼW�� film �ݥt�~�ק� util/camera.c  */

#define FILM_MOVIE	 13	/* �w�q film ���`�� (0~12) */

#define FILM_WEL_0	 0	/* �i���e�� myth_0 */
#define FILM_WEL_1	 1 	/* �i���e�� myth_1 */
#define FILM_WEL_2	 2 	/* �i���e�� myth_2 */
#define FILM_WEL_3	 3	/* �i���e�� myth_3 */
#define FILM_WEL_4	 4	/* �i���e�� myth_4 */
#define FILM_LOGIN       5	/* etc/Welcome_login */
#define FILM_WEL_BIRTH	 6	/* etc/Welcome_birth */
#define FILM_LOGOUT	 7	/* etc/Logout */
#define FILM_DAYPOST	 8	/* log/day */
#define FILM_POST	 9 	/* etc/post.note */
#define FILM_REGISTER	10 	/* etc/register */

#define FILM_GEM        11	/* ANNOUNCE.help */
#define FILM_MORE       12	/* MORE.help     */

