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
  char *board_admin_menu[] = {"00)¨ú®ø",
                              "11)§ï¤¤¤å¦WºÙ",
                              "22)¬ÝªO»¡©ú",
                              "33)¶iªOµe­±",
                              "44)¥i¨£¦W³æ",
                              "55)³]±K½X",
                              "66)§ï¤å³¹Ãþ§O",
                              "77)¶R¤W­­",
                              "88)¬ÝªOPO¤åª`·N¨Æ¶µ",
                              "99)¬ÝªO¸`¤é",
                              "aA)¸m©³­Ó¤H¤Æ³]©w",  
                              "bB)§ïÄÝ©Ê",
                              "cC)¬Ý¤µ¤é¬ö¿ý",
                              "dD)§ïÃþ§O"};

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

    switch(win_select("¬ÝªOºÞ²z", "", board_admin_menu, (mode == 1) ? 14 : 11 ,'0'))
    {
      case '1':
        move (1, 0);
        clrtoeol ();
        getdata (1, 0, "½Ð¿é¤J¬ÝªO·s¤¤¤å±Ô­z:"
          ,genbuf, 34, DOECHO, bp.title + 7);
        if (!genbuf[0]) return 0;
        strip_ansi (genbuf, genbuf, 0);
        if(strcmp(bp.title+7,genbuf))
        {
          sprintf(buf,"%-13.13s §ó´«¬ÝªO %s ±Ô­z [%s] -> [%s] , %s",
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
        char *choose_desc[3] = {"sS)¦sÀÉÂ÷¶}", "rR)­«·s½s¿è", msg_choose_cancel};
        
        strlcpy(desc[0], bp.desc[0], 80);
        strlcpy(desc[1], bp.desc[1], 80);
        strlcpy(desc[2], bp.desc[2], 80);

        clrchyiuan(i, 5);
        move(i++, 0);
        outs("¹ï¥»¬ÝªOªº´y­z (¦@¤T¦æ)");
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
          
          sprintf(buf,"%-13.13s §ó´«¬ÝªO %s »¡©ú , %s",
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

          getdata (3, 0, "½Ð³]©w¦³®Ä´Á­­(0 - 9999)¤Ñ¡H", buf, 5, DOECHO, "9999");
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
          if(!getdata (1, 0, "½Ð¿é¤J­ì¥»ªº±K½X" ,genbuf, PASSLEN, PASS, 0) ||
             !chkpasswd(bp.passwd, genbuf))
          {
               pressanykey("±K½X¿ù»~");
               return -1;
          }
        }
        if (!getdata(1, 0, "½Ð³]©w·s±K½X¡G", genbuf, PASSLEN, PASS,0))
        {
          pressanykey("±K½X³]©w¨ú®ø, Ä~Äò¨Ï¥ÎÂÂ±K½X");
          return -1;
        }
        strncpy(buf, genbuf, PASSLEN);

        getdata(1, 0, "½ÐÀË¬d·s±K½X¡G", genbuf, PASSLEN, PASS,0);
        if (strncmp(genbuf, buf, PASSLEN))
        {
          pressanykey("·s±K½X½T»{¥¢±Ñ, µLªk³]©w·s±K½X");
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
¥Ø«e¬ÝªOªº¤å³¹¤W­­¬° %-5d ½g\n \
          «O¯d®É¶¡¬° %-5d ¤Ñ\n",bp.maxpost,bp.maxtime);
        outs("¤@­Ó³æ¦ì¬°[1;32m¤@¦Ê½g¤å³¹[m©Î¬O[1;32m¤T¤Q¤Ñ[m , ¤@­Ó³æ¦ì»Ý [1;33m3000 ª÷¹ô[m");
        getdata(7, 0, "§A­n (1)¶R¤å³¹¤W­­ (2)¶R«O¦s®É¶¡", buf, 2, DOECHO, 0);
        if (buf[0] == '1' || buf[0] == '2')
        {
          int num = 0;

              while (num <= 0)
              {
                getdata(9, 0, "§A­n¶R´X­Ó³æ¦ì", genbuf, 3, DOECHO, 0);
                num = atoi(genbuf);
              }

              if (check_money(num * 3000, GOLD))
                break;

              if (buf[0] == '1')
              {
                if (bp.maxpost >= 99999)
                {
                  pressanykey("¤å³¹¼Æ¤w¹F¤W­­");
                  break;
                }
                else
                {
                  bp.maxpost += num*100;
                  sprintf(buf, "%-13.13s ÁÊ¶R¬ÝªO %s ¤å³¹¤W­­ %d ½g , %s",
                    cuser.userid, bp.brdname,num*100, ctime(&now));
                  f_cat(BBSHOME"/log/board_edit", buf);
                  log_usies ("BuyPL", currboard);
                  pressanykey("¬ÝªO¤å³¹¤W­­¼W¥[¬° %d ½g", bp.maxpost);
                }
              }
              else
              {
                if (bp.maxtime >= 9999)
                {
                  pressanykey("«O¦s®É¶¡¤w¹F¤W­­");
                  break;
                }
                else
                {
                  bp.maxtime += num * 30;
                  sprintf(buf,"%-13.13s ÁÊ¶R¬ÝªO %s ¤å³¹«O¯d®É¶¡ %d ¤Ñ , %s",
                    cuser.userid,bp.brdname,num*30,ctime(&now));
                  f_cat(BBSHOME"/log/board_edit",buf);
                  log_usies ("BuyBT", currboard);
                  pressanykey("¬ÝªO¤å³¹«O¯d®É¶¡¼W¥[¬° %d ¤Ñ",bp.maxtime);
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
         if (win_select("¦Û­qpostª`·N¨Æ¶µ", "¬O§_¥Î¦Û­qpostª`·N¨Æ¶µ", 0, 2, 'n') == 'y')
           vedit(buf, NA, NULL);
         else
           unlink(buf);
       }
       break;

       case '9':
       {
         char *choose_feast[4] = {"cC)­×§ï¸`¤é","dD)§R°£¸`¤é³]©w", "hH)¾\\Åª³]©w»¡©ú",msg_choose_cancel};

         setbfile(buf, currboard, FN_BRDFEAST);

         switch(win_select("¬ÝªO¸`¤é³]©w", "½Ð¿ï¾Ü©Ò»Ý­nªº°Ê§@?", choose_feast, 4, 'h'))
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
         getdata(b_lines-1, 0, "½Ð¿é¤J¬ÝªO¸m©³µü: ", buf, 5, DOECHO, bp.bottom);
         
         if(!buf[0] && getans2(b_lines, 0, "±N¦^´_¹w³]¸m©³µü¡A½Ð±z½T©w? ", 0, 2, 'n') == 'y')
           bp.bottom[0] = '\0';
         else if(buf[0] != '\0')
           strlcpy(bp.bottom, buf, sizeof(bp.bottom));

         color_selector(bp.botm_color);
       }
       break;

/* ¥H¤U¬O ¨p¤Hª©¤~¦³ªº¥\¯à!! */
          case 'b':
          {
            int oldattr=bp.brdattr;
            char *brd_type[3]={"oO)¶}©ñ","pP)¨p¤H","hH)ÁôÂÃ"};

            ans = getans2(1, 0,"¬ÝªOª¬ºA§ó§ï¬°", brd_type, 3, 'o');

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
              sprintf(buf,"%-13.13s §ó§ï¬ÝªO [%s] ¤§ÄÝ©Ê¬° %s , %s",
                cuser.userid,bp.brdname,
                ans == 'p' ? "¨p¤H" : ans == 'h' ? "ÁôÂÃ" : "¶}©ñ",ctime(&now));
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
            getdata (1, 0, "½Ð¿é¤J¬ÝªO·sÃþ§O:",genbuf, 5, DOECHO, bp.title );
            if (!genbuf[0]) return 0;
            strip_ansi (genbuf, genbuf, 0);
            if(strncmp(bp.title,genbuf,4))
            {
              sprintf(buf,"%-13.13s §ó´«¬ÝªO %s Ãþ§O [%-4.4s] -> [%s] , %s",
                cuser.userid,bp.brdname,bp.title,genbuf,ctime(&now));
              f_cat(BBSHOME"/log/board_edit",buf);
              log_usies("PREFIX",currboard);
              strncpy (bp.title , genbuf, 4);
            }
          break;

      default:
        pressanykey("©ñ±ó­×§ï");
        break;
    }
    substitute_record (fn_board, &bp, sizeof (boardheader), bid);
    touch_boards ();
    return RC_FULL;
  }
  return 0;
}
