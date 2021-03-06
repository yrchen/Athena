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
  char *board_admin_menu[] = {"00)取消",
                              "11)改中文名稱",
                              "22)看板說明",
                              "33)進板畫面",
                              "44)可見名單",
                              "55)設密碼",
                              "66)改文章類別",
                              "77)買上限",
                              "88)看板PO文注意事項",
                              "99)看板節日",
                              "aA)置底個人化設定",  
                              "bB)改屬性",
                              "cC)看今日紀錄",
                              "dD)改類別"};

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

    switch(win_select("看板管理", "", board_admin_menu, (mode == 1) ? 14 : 11 ,'0'))
    {
      case '1':
        move (1, 0);
        clrtoeol ();
        getdata (1, 0, "請輸入看板新中文敘述:"
          ,genbuf, 34, DOECHO, bp.title + 7);
        if (!genbuf[0]) return 0;
        strip_ansi (genbuf, genbuf, 0);
        if(strcmp(bp.title+7,genbuf))
        {
          sprintf(buf,"%-13.13s 更換看板 %s 敘述 [%s] -> [%s] , %s",
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
        char *choose_desc[3] = {"sS)存檔離開", "rR)重新編輯", msg_choose_cancel};
        
        strlcpy(desc[0], bp.desc[0], 80);
        strlcpy(desc[1], bp.desc[1], 80);
        strlcpy(desc[2], bp.desc[2], 80);

        clrchyiuan(i, 5);
        move(i++, 0);
        outs("對本看板的描述 (共三行)");
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
          
          sprintf(buf,"%-13.13s 更換看板 %s 說明 , %s",
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

          getdata (3, 0, "請設定有效期限(0 - 9999)天？", buf, 5, DOECHO, "9999");
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
          if(!getdata (1, 0, "請輸入原本的密碼" ,genbuf, PASSLEN, PASS, 0) ||
             !chkpasswd(bp.passwd, genbuf))
          {
               pressanykey("密碼錯誤");
               return -1;
          }
        }
        if (!getdata(1, 0, "請設定新密碼：", genbuf, PASSLEN, PASS,0))
        {
          pressanykey("密碼設定取消, 繼續使用舊密碼");
          return -1;
        }
        strncpy(buf, genbuf, PASSLEN);

        getdata(1, 0, "請檢查新密碼：", genbuf, PASSLEN, PASS,0);
        if (strncmp(genbuf, buf, PASSLEN))
        {
          pressanykey("新密碼確認失敗, 無法設定新密碼");
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
目前看板的文章上限為 %-5d 篇\n \
          保留時間為 %-5d 天\n",bp.maxpost,bp.maxtime);
        outs("一個單位為[1;32m一百篇文章[m或是[1;32m三十天[m , 一個單位需 [1;33m3000 金幣[m");
        getdata(7, 0, "你要 (1)買文章上限 (2)買保存時間", buf, 2, DOECHO, 0);
        if (buf[0] == '1' || buf[0] == '2')
        {
          int num = 0;

              while (num <= 0)
              {
                getdata(9, 0, "你要買幾個單位", genbuf, 3, DOECHO, 0);
                num = atoi(genbuf);
              }

              if (check_money(num * 3000, GOLD))
                break;

              if (buf[0] == '1')
              {
                if (bp.maxpost >= 99999)
                {
                  pressanykey("文章數已達上限");
                  break;
                }
                else
                {
                  bp.maxpost += num*100;
                  sprintf(buf, "%-13.13s 購買看板 %s 文章上限 %d 篇 , %s",
                    cuser.userid, bp.brdname,num*100, ctime(&now));
                  f_cat(BBSHOME"/log/board_edit", buf);
                  log_usies ("BuyPL", currboard);
                  pressanykey("看板文章上限增加為 %d 篇", bp.maxpost);
                }
              }
              else
              {
                if (bp.maxtime >= 9999)
                {
                  pressanykey("保存時間已達上限");
                  break;
                }
                else
                {
                  bp.maxtime += num * 30;
                  sprintf(buf,"%-13.13s 購買看板 %s 文章保留時間 %d 天 , %s",
                    cuser.userid,bp.brdname,num*30,ctime(&now));
                  f_cat(BBSHOME"/log/board_edit",buf);
                  log_usies ("BuyBT", currboard);
                  pressanykey("看板文章保留時間增加為 %d 天",bp.maxtime);
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
         if (win_select("自訂post注意事項", "是否用自訂post注意事項", 0, 2, 'n') == 'y')
           vedit(buf, NA, NULL);
         else
           unlink(buf);
       }
       break;

       case '9':
       {
         char *choose_feast[4] = {"cC)修改節日","dD)刪除節日設定", "hH)閱\讀設定說明",msg_choose_cancel};

         setbfile(buf, currboard, FN_BRDFEAST);

         switch(win_select("看板節日設定", "請選擇所需要的動作?", choose_feast, 4, 'h'))
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
         getdata(b_lines-1, 0, "請輸入看板置底詞: ", buf, 5, DOECHO, bp.bottom);
         
         if(!buf[0] && getans2(b_lines, 0, "將回復預設置底詞，請您確定? ", 0, 2, 'n') == 'y')
           bp.bottom[0] = '\0';
         else if(buf[0] != '\0')
           strlcpy(bp.bottom, buf, sizeof(bp.bottom));

         color_selector(bp.botm_color);
       }
       break;

/* 以下是 私人版才有的功能!! */
          case 'b':
          {
            int oldattr=bp.brdattr;
            char *brd_type[3]={"oO)開放","pP)私人","hH)隱藏"};

            ans = getans2(1, 0,"看板狀態更改為", brd_type, 3, 'o');

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
              sprintf(buf,"%-13.13s 更改看板 [%s] 之屬性為 %s , %s",
                cuser.userid,bp.brdname,
                ans == 'p' ? "私人" : ans == 'h' ? "隱藏" : "開放",ctime(&now));
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
            getdata (1, 0, "請輸入看板新類別:",genbuf, 5, DOECHO, bp.title );
            if (!genbuf[0]) return 0;
            strip_ansi (genbuf, genbuf, 0);
            if(strncmp(bp.title,genbuf,4))
            {
              sprintf(buf,"%-13.13s 更換看板 %s 類別 [%-4.4s] -> [%s] , %s",
                cuser.userid,bp.brdname,bp.title,genbuf,ctime(&now));
              f_cat(BBSHOME"/log/board_edit",buf);
              log_usies("PREFIX",currboard);
              strncpy (bp.title , genbuf, 4);
            }
          break;

      default:
        pressanykey("放棄修改");
        break;
    }
    substitute_record (fn_board, &bp, sizeof (boardheader), bid);
    touch_boards ();
    return RC_FULL;
  }
  return 0;
}
