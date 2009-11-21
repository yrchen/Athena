/*-------------------------------------------------------*/
/* bm.c             ( AT-BBS/WD_hialan V2.5 )            */
/*-------------------------------------------------------*/
/* target : board admin                                  */
/* create : 2003/10/07                                   */
/*-------------------------------------------------------*/
#include "bbs.h"
char *genpasswd(char *pw);
int board_edit()
{
  boardheader bp;
  int bid, mode = 0;
  time_t now = time(0);
  char *board_admin_menu[] = {"00)����",
                              "11)�襤��W��",
                              "22)�ݪO����",
                              "33)�i�O�e��",
                              "44)�i���W��",
                              "55)�]�K�X",
                              "66)��峹���O",
                              "77)�R�W��",
                              "88)�ݪOPO��`�N�ƶ�",
                              "99)�ݪO�`��",
                              "aA)�m���ӤH�Ƴ]�w",  
                              "bB)���ݩ�",
                              "cC)�ݤ������",
                              "dD)�����O"};

  if (currmode & MODE_BOARD)
  {
    char genbuf[BTLEN+1], buf[200], ans;
    bid = getbnum (currboard);

    if (rec_get (fn_board, &bp, sizeof (boardheader), bid) == -1)
    {
      pressanykey (err_bid);
      return -1;
    }

    if (bp.brdattr & BRD_PERSONAL || HAS_PERM(PERM_SYSOP))
      mode = 1;

    switch(win_select("�ݪO�޲z", "", board_admin_menu, (mode == 1) ? 14 : 11 ,'0'))
    {
      case '1':
        move (1, 0);
        clrtoeol ();
        getdata (1, 0, "�п�J�ݪO�s����ԭz:"
          ,genbuf, 34, DOECHO, bp.title + 7);
        if (!genbuf[0]) return 0;
        strip_ansi (genbuf, genbuf, 0);
        if(strcmp(bp.title+7,genbuf))
        {
          sprintf(buf,"%-13.13s �󴫬ݪO %s �ԭz [%s] -> [%s] , %s",
            cuser.userid,bp.brdname,bp.title+7,genbuf,ctime(&now));
          f_cat(BBSHOME"/log/board_edit",buf);
          log_usies ("NameBoard", currboard);
          strcpy (bp.title + 7, genbuf);
        }
        break;

      case '2':
      {
        int i=2, j=0;
        char desc[3][80];
        char *choose_desc[3] = {"sS)�s�����}", "rR)���s�s��", msg_choose_cancel};
        
        strlcpy(desc[0], bp.desc[0], 80);
        strlcpy(desc[1], bp.desc[1], 80);
        strlcpy(desc[2], bp.desc[2], 80);

        clrchyiuan(i, 5);
        move(i++, 0);
        outs("�糧�ݪO���y�z (�@�T��)");
        do
        {
          for(j=0;j<3;j++)
            getdata(i+j, 0, ":", desc[j], 79, DOECHO, desc[j]);
          
          j = getans2(i+j, 0, "", choose_desc, 3, 's');
          
        }while(j == 'r');
        
        if(j != 'q')
        {
          strlcpy(bp.desc[0], desc[0], sizeof(bp.desc[0]));
          strlcpy(bp.desc[1], desc[1], sizeof(bp.desc[1]));
          strlcpy(bp.desc[2], desc[2], sizeof(bp.desc[2]));
          
          sprintf(buf,"%-13.13s �󴫬ݪO %s ���� , %s",
            cuser.userid,bp.brdname,ctime(&now));
          f_cat("log/board_edit",buf);
          log_usies ("SetBoardDesc", currboard);
        }
        break;
      }
      case '3':
        setbfile (buf, currboard, fn_notes);
        if (vedit (buf, NA) == -1)
          pressanykey (msg_cancel);
        else
        {
          int aborted;

          getdata (3, 0, "�г]�w���Ĵ���(0 - 9999)�ѡH", buf, 5, DOECHO, "9999");
          aborted = atol (buf);
          bp.bupdate = aborted ? time (0) + aborted * 86400 : 0;
        }
        break;

      case '4':
        setbfile(buf, currboard, FN_LIST);
        ListEdit(buf);
        hbflreload(bid);
        return RC_FULL;

      case '5':
      {
        char genbuf[PASSLEN+1],buf[PASSLEN+1];

        move (1, 0);
        clrtoeol ();
        if(!HAS_PERM(PERM_ALLBOARD))
        {
          if(!getdata (1, 0, "�п�J�쥻���K�X" ,genbuf, PASSLEN, PASS, 0) ||
             !chkpasswd(bp.passwd, genbuf))
          {
               pressanykey("�K�X���~");
               return -1;
          }
        }
        if (!getdata(1, 0, "�г]�w�s�K�X�G", genbuf, PASSLEN, PASS,0))
        {
          pressanykey("�K�X�]�w����, �~��ϥ��±K�X");
          return -1;
        }
        strncpy(buf, genbuf, PASSLEN);

        getdata(1, 0, "���ˬd�s�K�X�G", genbuf, PASSLEN, PASS,0);
        if (strncmp(genbuf, buf, PASSLEN))
        {
          pressanykey("�s�K�X�T�{����, �L�k�]�w�s�K�X");
          return -1;
        }
        buf[8] = '\0';
        strncpy(bp.passwd, genpasswd(buf), PASSLEN);
        log_usies ("SetBrdPass", currboard);
      }
      break;

      case '6':
        prefix_edit();
      break;

      case '7':
      {
        clrchyiuan(1, 15);
        move(3, 0);
        prints(" \
�ثe�ݪO���峹�W���� %-5d �g\n \
          �O�d�ɶ��� %-5d ��\n",bp.maxpost,bp.maxtime);
        outs("�@�ӳ�쬰[1;32m�@�ʽg�峹[m�άO[1;32m�T�Q��[m , �@�ӳ��� [1;33m3000 ����[m");
        getdata(7, 0, "�A�n (1)�R�峹�W�� (2)�R�O�s�ɶ�", buf, 2, DOECHO, 0);
        if (buf[0] == '1' || buf[0] == '2')
        {
          int num = 0;

              while (num <= 0)
              {
                getdata(9, 0, "�A�n�R�X�ӳ��", genbuf, 3, DOECHO, 0);
                num = atoi(genbuf);
              }

              if (check_money(num * 3000, GOLD))
                break;

              if (buf[0] == '1')
              {
                if (bp.maxpost >= 99999)
                {
                  pressanykey("�峹�Ƥw�F�W��");
                  break;
                }
                else
                {
                  bp.maxpost += num*100;
                  sprintf(buf, "%-13.13s �ʶR�ݪO %s �峹�W�� %d �g , %s",
                    cuser.userid, bp.brdname,num*100, ctime(&now));
                  f_cat(BBSHOME"/log/board_edit", buf);
                  log_usies ("BuyPL", currboard);
                  pressanykey("�ݪO�峹�W���W�[�� %d �g", bp.maxpost);
                }
              }
              else
              {
                if (bp.maxtime >= 9999)
                {
                  pressanykey("�O�s�ɶ��w�F�W��");
                  break;
                }
                else
                {
                  bp.maxtime += num * 30;
                  sprintf(buf,"%-13.13s �ʶR�ݪO %s �峹�O�d�ɶ� %d �� , %s",
                    cuser.userid,bp.brdname,num*30,ctime(&now));
                  f_cat(BBSHOME"/log/board_edit",buf);
                  log_usies ("BuyBT", currboard);
                  pressanykey("�ݪO�峹�O�d�ɶ��W�[�� %d ��",bp.maxtime);
                }
              }
              degold(num*3000);
        }
      }
        break;

       case '8':
       {
         setbfile(buf, currboard,  FN_POST_NOTE );
         if(more(buf,NA) == -1)
           more(FN_POST_NOTE , NA);
         if (win_select("�ۭqpost�`�N�ƶ�", "�O�_�Φۭqpost�`�N�ƶ�", 0, 2, 'n') == 'y')
           vedit(buf, NA, NULL);
         else
           unlink(buf);
       }
       break;

       case '9':
       {
         char *choose_feast[4] = {"cC)�ק�`��","dD)�R���`��]�w", "hH)�\\Ū�]�w����",msg_choose_cancel};

         setbfile(buf, currboard, FN_BRDFEAST);

         switch(win_select("�ݪO�`��]�w", "�п�ܩһݭn���ʧ@?", choose_feast, 4, 'h'))
         {
           case 'd':
             unlink(buf);
             break;

           case 'c':
             vedit(buf, NA, NULL);
             break;
           case 'h':
             more("etc/brdfeast", YEA);
             break;
         }

         load_feast(currboard);
       }
       break;
       
       case 'a':
       {
         getdata(b_lines-1, 0, "�п�J�ݪO�m����: ", buf, 5, DOECHO, bp.bottom);
         
         if(!buf[0] && getans2(b_lines, 0, "�N�^�_�w�]�m�����A�бz�T�w? ", 0, 2, 'n') == 'y')
           bp.bottom[0] = '\0';
         else if(buf[0] != '\0')
           strlcpy(bp.bottom, buf, sizeof(bp.bottom));

         color_selector(bp.botm_color);
       }
       break;

/* �H�U�O �p�H���~�����\��!! */
          case 'b':
          {
            int oldattr=bp.brdattr;
            char *brd_type[3]={"oO)�}��","pP)�p�H","hH)����"};

            ans = getans2(1, 0,"�ݪO���A��אּ", brd_type, 3, 'o');

            if(ans == 'p')
            {
              bp.brdattr &= ~BRD_POSTMASK;
              bp.brdattr |= BRD_HIDE;
            }
            else if(ans == 'h')
            {
              bp.brdattr |= BRD_POSTMASK;
              bp.brdattr |= BRD_HIDE;
            }
            else
            {
              bp.brdattr &= ~BRD_POSTMASK;
              bp.brdattr &= ~BRD_HIDE;
            }

            if(bp.brdattr != oldattr)
            {
              sprintf(buf,"%-13.13s ���ݪO [%s] ���ݩʬ� %s , %s",
                cuser.userid,bp.brdname,
                ans == 'p' ? "�p�H" : ans == 'h' ? "����" : "�}��",ctime(&now));
              f_cat(BBSHOME"/log/board_edit",buf);
              log_usies("ATTR_Board",currboard);
            }
          }
          break;

          case 'c':
          {
            sprintf(buf,"/usr/bin/grep \"USE %s \" %s/usboard > %s/tmp/usboard.%s",
              currboard, BBSHOME, BBSHOME, currboard);
            system(buf);
            sprintf(buf,BBSHOME"/tmp/usboard.%s",currboard);
            more(buf, YEA);
            log_usies("BOARDLOG", currboard);
          }
          break;

          case 'd':
            move (1, 0);
            clrtoeol ();
            getdata (1, 0, "�п�J�ݪO�s���O:",genbuf, 5, DOECHO, bp.title );
            if (!genbuf[0]) return 0;
            strip_ansi (genbuf, genbuf, 0);
            if(strncmp(bp.title,genbuf,4))
            {
              sprintf(buf,"%-13.13s �󴫬ݪO %s ���O [%-4.4s] -> [%s] , %s",
                cuser.userid,bp.brdname,bp.title,genbuf,ctime(&now));
              f_cat(BBSHOME"/log/board_edit",buf);
              log_usies("PREFIX",currboard);
              strncpy (bp.title , genbuf, 4);
            }
          break;

      default:
        pressanykey("���ק�");
        break;
    }
    substitute_record (fn_board, &bp, sizeof (boardheader), bid);
    touch_boards ();
    return RC_FULL;
  }
  return 0;
}
