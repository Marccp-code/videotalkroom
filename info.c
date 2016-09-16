#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>       //sem_t

#define CommonH
#include "common.h"
//extern char sPath[80];
//����״̬����
//extern struct Local1 Local;
//extern struct LocalCfg1 LocalCfg;
void DisplayInfo_1_Window(void);    //��ʾ��Ϣ���ڣ�һ����
void OperateInfo_2_Window(short wintype, int currwindow);    //��Ϣ���ڲ�����������
void DisplayDeleteAllInfoWindow(void);    //��ʾȫ��ɾ����Ϣ���ڣ�������������
void OperateDeleteAllInfoWindow(short wintype, int currwindow);    //ȫ��ɾ����Ϣ���ڲ�����������������
void DisplayDeleteSingleInfoWindow(void);    //��ʾ����ɾ����Ϣ���ڣ�������������
void OperateDeleteSingleInfoWindow(short wintype, int currwindow);    //����ɾ����Ϣ���ڲ�����������������

void ShowInfoList(int infotype);    //��Ϣ��ʾ����һ����  ��Ϣ�б�
void DisplayInfoContent(int infotype, int infono);    //��Ϣ��ʾ���򣨶�����  ��Ϣ����
void OperateInfoContent(short wintype, int currwindow);    //��Ϣ��ʾ���򣨶�����  ��Ϣ���� ����
void ShowInfoContent(int infotype, int infono);    // ��Ϣ����
void ShowInfoNum(int infotype);  //��ʾ��Ϣ������δ����Ϣ����
void CreateContentRow(char *Content, int nLength, char create_row[MAXROW][40]);  //����Ҫ��ʾ��������
//д��Ϣ�ļ�
void WriteInfoFile(int InfoType);
//д���ļ� ������Ϣ    δ�����Ѷ�
void WriteInfoFileLock(int InfoType, int InfoNo, InfoNode1 *Info_Node);

//extern struct TImageButton info_button[InfoButtonMax];//��Ϣ���ڰ�ť
//��ǰ��Ϣ����״̬
struct InfoStatus1 InfoStatus;

InfoNode1 *CurrInfo_Node; //��ǰ��Ϣ���
//����Ϣ�ṹ
extern struct Info1 Info[INFOTYPENUM];
// *h�����ͷ����ָ�룬*pָ��ǰ����ǰһ����㣬*sָ��ǰ���
extern InfoNode1 *InfoNode_h[INFOTYPENUM];

struct displayinfo1
 {
  int totalpage;
  int pageno;
  int totalrow;
  char content_row[MAXROW][40];
  int isDeleted;  //����ʾ��Ϣ����ʱɾ������Ϣ
 }displayinfo;
//---------------------------------------------------------------------------
void DisplayInfo_1_Window(void)    //��ʾ��Ϣ���ڣ�һ����
{
  int i;
  int xTop;
  char jpgfilename[80];
  int InfoTypeX[5] = {29, 29, 513, 513, 513};
  int InfoTypeY[5] = {127, 175, 223, 271, 319};
  int InfoRowX[3] = {47, 47, 47};
  int InfoRowY[3] = {203, 252, 303};
  xTop = 36;

  Local.PrevWindow = Local.CurrentWindow;
  Local.CurrentWindow = 4;
    
  strcpy(jpgfilename, sPath);
  strcat(jpgfilename,"info.jpg");
  DisplayJPG(0, 0, jpgfilename, 1, SCRWIDTH, SCRHEIGHT, 0);


  //��ͨ��Ϣ
  InfoStatus.CurrType = 0;  //��ǰ��Ϣ����
  InfoStatus.CurrWin = 0;    //��Ϣ�б�
  InfoStatus.CurrNo=0;       //��ǰ��Ϣ���  0---n-1
  Info[InfoStatus.CurrType].CurrentInfoPage = 1; //��ǰ��ϢҳΪ��0ҳ
  Info[InfoStatus.CurrType].CurrNo = 0;

  for(i=0; i<3; i++)
   {
    inforow_button[i].xLeft = InfoRowX[i];
    inforow_button[i].yTop = InfoRowY[i];
   }

//  ShowInfoNum(InfoStatus.CurrType);  //��ʾ��Ϣ������δ����Ϣ����
  ShowInfoList(InfoStatus.CurrType);
  
  for(i=0; i<3; i++)
    DisplayImageButton(&info_button[i], IMAGEUP, 0);

  Local.NewInfo = 0;  //������Ϣ
  ioctl(gpio_fd, IO_CLEAR, 3);
}
//---------------------------------------------------------------------------
void OperateInfo_2_Window(short wintype, int currwindow)    //��Ϣ���ڲ�����������
{
  int i;
  int TmpInfoNo;
  InfoNode1 *tmp_node;
  int numperpage;
  int xLeft, yTop, yHeight;

  if(((wintype >= 0) && (wintype <= 2)))
   {
    DisplayImageButton(&info_button[wintype], IMAGEDOWN, 0);
    usleep(DELAYTIME*1000);
    if(Local.CurrentWindow != currwindow)
      return;
    DisplayImageButton(&info_button[wintype], IMAGEUP, 0);
   }
  if(wintype >= 30)
   {
    DisplayImageButton(&bigmenu_button[wintype-30], IMAGEDOWN, 0);
    usleep(DELAYTIME*1000);
    if(Local.CurrentWindow != currwindow)
      return;
    DisplayImageButton(&bigmenu_button[wintype-30], IMAGEUP, 0);
   }
  switch(wintype)
   {
    case 0://�Ϸ�       a
             if(Info[InfoStatus.CurrType].CurrNo > 0)
              {
               Info[InfoStatus.CurrType].CurrNo --;
               InfoStatus.CurrNo = Info[InfoStatus.CurrType].CurrNo;
               ShowInfoList(InfoStatus.CurrType);
              }
           break;
    case 1://�·�     b
             if(Info[InfoStatus.CurrType].CurrNo < (Info[InfoStatus.CurrType].TotalNum -1))
              {
               Info[InfoStatus.CurrType].CurrNo++;
               InfoStatus.CurrNo = Info[InfoStatus.CurrType].CurrNo;
               ShowInfoList(InfoStatus.CurrType);
              }
           break;
    case 2://ɾ��           c
              //ɾ����ǰ��Ϣ
              CurrInfo_Node=locate_infonode(InfoNode_h[InfoStatus.CurrType], InfoStatus.CurrNo + 1);
              if(CurrInfo_Node != NULL)
               if(CurrInfo_Node->Content.isLocked == 0)
               {
                displayinfo.isDeleted = 1;  //����ʾ��Ϣ����ʱɾ������Ϣ
                delete_infonode(CurrInfo_Node);
                Info[InfoStatus.CurrType].TotalNum --;
                //д���ļ�
                //����������
                pthread_mutex_lock (&Local.save_lock);
                //���ҿ��ô洢���岢���
                for(i=0; i<SAVEMAX; i++)
                 if(Save_File_Buff[i].isValid == 0)
                  {
                   Save_File_Buff[i].Type = 1;
                   Save_File_Buff[i].InfoType = InfoStatus.CurrType;
                   Save_File_Buff[i].isValid = 1;
                   break;
                  }
                //�򿪻�����
                pthread_mutex_unlock (&Local.save_lock);
                sem_post(&save_file_sem);

                if(InfoStatus.CurrNo >= (Info[InfoStatus.CurrType].TotalNum - 1))
                  InfoStatus.CurrNo = Info[InfoStatus.CurrType].TotalNum - 1;
                ShowInfoList(InfoStatus.CurrType);
               }
             break;

    case 4://��Ϣ1��           c
    case 5://��Ϣ2��           c
    case 6://��Ϣ3��           c
           //��ǰ��Ϣ���
             TmpInfoNo = (Info[InfoStatus.CurrType].CurrentInfoPage-1)*INFONUMPERPAGE+(wintype - 4);
             printf("TmpInfoNo = %d, Info[InfoStatus.CurrType].TotalNum = %d \n", TmpInfoNo,
                    Info[InfoStatus.CurrType].TotalNum);
             if(InfoStatus.CurrNo == TmpInfoNo)
              {
               if(TmpInfoNo < (Info[InfoStatus.CurrType].TotalNum))
                {
                 Info[InfoStatus.CurrType].CurrNo = TmpInfoNo;
                 InfoStatus.CurrNo = TmpInfoNo;
                 DisplayInfoContent(InfoStatus.CurrType, InfoStatus.CurrNo);
                }
              }
             else
              {
               if(TmpInfoNo < (Info[InfoStatus.CurrType].TotalNum))
                {
                 Info[InfoStatus.CurrType].CurrNo = TmpInfoNo;
                 InfoStatus.CurrNo = TmpInfoNo;
                 ShowInfoList(InfoStatus.CurrType);
                }
              }
           break;
    case 30:  //��ҳ
    case 31:  //����
    case 32:  //�ҵ�
    case 33:  //�Խ�
    //case 34:  //��Ϣ
    case 35:  //����
           DisplayMainWindow(wintype - 30);
           break;
   }
}
//---------------------------------------------------------------------------
void ShowInfoList(int infotype)    //��Ϣ��ʾ����һ����  ��Ϣ�б�
{
  int xLeft,yTop,yHeight;
  int i, j;
  int PageTotalNum;  //��ҳ��Ϣ����
  char jpgfilename[80];
  char str[3];
  InfoNode1 *tmp_node;
  char tmp_con[20];
  int tmp_len;  //Ԥ������
  int fontcolor;
  int NoInPage;
  int ascnum;
  xLeft = 120;
  yTop = 36;
  tmp_len = 10;


  InfoStatus.CurrWin = 0;    //��Ϣ�б�

  switch(infotype)
   {
    case 0: //��ͨ��Ϣ
    case 1: //������Ϣ
    case 2: //����Ԥ��
    case 3: //��������
           //����Ϣ��ʾ����
           for(i = 0; i < INFONUMPERPAGE; i++)
              DisplayImageButton(&inforow_button[i], IMAGEUP, 0);
           if(Info[infotype].TotalNum > 0)
            {
             //��ҳ��
             if((Info[infotype].TotalNum % INFONUMPERPAGE) == 0)
               Info[infotype].TotalInfoPage = Info[infotype].TotalNum /INFONUMPERPAGE;
             else
               Info[infotype].TotalInfoPage = Info[infotype].TotalNum /INFONUMPERPAGE + 1;

             xLeft = 300;
             yTop = 50;
             yHeight = 40;
             //��ǰҳ
             Info[infotype].CurrentInfoPage = InfoStatus.CurrNo /INFONUMPERPAGE + 1;
             if(Info[infotype].CurrentInfoPage < Info[infotype].TotalInfoPage)
               PageTotalNum = INFONUMPERPAGE;
             else
               PageTotalNum = Info[infotype].TotalNum - (Info[infotype].CurrentInfoPage - 1)*INFONUMPERPAGE;
             //��ǰ��Ϣ�ڱ�ҳ�е�λ��
             NoInPage = (InfoStatus.CurrNo)%INFONUMPERPAGE;

             DisplayImageButton(&inforow_button[NoInPage], IMAGEDOWN, 0);

             for(i = 0; i < PageTotalNum; i++)
              {
               if(i !=  NoInPage)
                 fontcolor = cBlack;
               else
                 fontcolor = cWhite;

               tmp_node=locate_infonode(InfoNode_h[infotype], (Info[infotype].CurrentInfoPage-1)*INFONUMPERPAGE+i+1);

               //���
               sprintf(tmp_con, "%02d\0", (Info[infotype].CurrentInfoPage-1)*INFONUMPERPAGE+i+1);
               outxy24(inforow_button[i].xLeft+50, inforow_button[i].yTop + 10, 1,
                       fontcolor, 1, 1, tmp_con, 0, 0);


               //����
               if(tmp_node->Content.Length >= tmp_len)
                {
                 memcpy(tmp_con, tmp_node->Content.Content, tmp_len);
                 tmp_con[tmp_len] = '\0';
                 for(j=0; j<tmp_len; j++)
                  if(tmp_con[j] == '\r')
                    tmp_con[j] = '\0';
                 if(tmp_con[8] < 0xA1)
                   tmp_con[9] = '\0';

                 ascnum = 0;
                 for(j=0; j<strlen(tmp_con); j++)
                  {
                   if(tmp_con[j] < 0xA1)
                     ascnum ++;
                   else
                     j ++;  
                  }
                 if((ascnum % 2) != 0)
                   tmp_con[9] = '\0';                                   
                }
               else
                {
                 memcpy(tmp_con, tmp_node->Content.Content, tmp_node->Content.Length);
                 tmp_con[tmp_node->Content.Length] = '\0';
                 for(j=0; j<tmp_node->Content.Length; j++)
                  if(tmp_con[j] == '\r')
                    tmp_con[j] = '\0';
                }
               outxy24(inforow_button[i].xLeft+100, inforow_button[i].yTop+10, 0,
                       fontcolor, 1, 1, tmp_con, 0, 0);

               //ʱ��
               memcpy(tmp_con, tmp_node->Content.Time, tmp_len);
                 tmp_con[tmp_len] = '\0';
               outxy24(inforow_button[i].xLeft+250, inforow_button[i].yTop+10, 1,
                       fontcolor, 1, 1, tmp_con, 0, 0);

               outxy24(inforow_button[i].xLeft+400, inforow_button[i].yTop+10, 1,
                       fontcolor, 1, 1, "����", 0, 0);
              }
            }
           break;
    case 4:  //����
           break;
   }
}
//---------------------------------------------------------------------------
void DisplayInfoContent(int infotype, int infono)    //��Ϣ��ʾ���򣨶�����  ��Ϣ����
{

  char jpgfilename[80];
  int i;

  displayinfo.isDeleted = 0;  //����ʾ��Ϣ����ʱɾ������Ϣ

  //�洢����
  SavePicBuf_Func(InfoWin.xLeft, InfoWin.yTop, InfoWin.width, InfoWin.height, InfoWin.image[1], 0);
  DisplayPopupWin(&InfoWin, SHOW, 0);
  Local.TmpWindow = Local.CurrentWindow;
  Local.CurrentWindow = 77;

  for(i=0; i<3; i++)
    DisplayImageButton(&infowin_button[i], IMAGEUP, 0);

  InfoStatus.CurrWin = 1;    //��Ϣ����

  if(PicStatBuf.InfoContent1 == NULL)
   {
    PicStatBuf.InfoContent1 = (unsigned char *)malloc(432*110*3/2);  //����һ����Ļ����
    SavePicBuf_Func(infowin_image.xLeft + 25, infowin_image.yTop + 62, 432, 110, PicStatBuf.InfoContent1, 0);
   }

  ShowInfoContent(infotype, infono);
}
//---------------------------------------------------------------------------
void ShowInfoContent(int infotype, int infono)    // ��Ϣ����
{
  int xLeft,yTop,yHeight;
  int i, j;
  int PageTotalNum;  //��ҳ��Ϣ����
  InfoNode1 *tmp_node;
  char tmp_con[50];
  int tmp_len;  //Ԥ������
  int tmp_row;
  int numperpage;
  char str[3];

  xLeft = 150;
  yTop = 142;
  tmp_len = 20;  

  xLeft = infowin_image.xLeft + 15;
  yTop = infowin_image.yTop + 30;
  yHeight = 38;
  if(infono <= Info[infotype].TotalNum - 1)
   {
    RestorePicBuf_Func(infowin_image.xLeft + 25, infowin_image.yTop + 62, 432, 110, PicStatBuf.InfoContent1, 0);
    printf("infono=%d\n",infono + 1);
    tmp_node=locate_infonode(InfoNode_h[infotype], infono + 1);
    //��ǰ��Ϣ���
    CurrInfo_Node = tmp_node;

    sprintf(tmp_con, "�� %d ��" , infono + 1);
    outxy24(xLeft+10, yTop+0*yHeight, 1, cBlack, 1, 1, tmp_con, 0, 0);

    memcpy(tmp_con, tmp_node->Content.Time, tmp_len);
    tmp_con[tmp_len] = '\0';
    outxy24(xLeft + 90, yTop+0*yHeight, 1, cBlack, 1, 1, tmp_con, 0, 0);

    tmp_len = 32;
    yTop = yTop+1*yHeight;
    printf("tmp_node->Content.Length= %d\n", tmp_node->Content.Length);
    displayinfo.totalpage = 4;
    displayinfo.pageno = 0;
    CreateContentRow(tmp_node->Content.Content, tmp_node->Content.Length, displayinfo.content_row);

    if(displayinfo.totalpage == (displayinfo.pageno + 1))
      numperpage = displayinfo.totalrow - displayinfo.pageno*PAGEPERROW;
    else
      numperpage = PAGEPERROW;
    for(i=0; i<numperpage; i++)
      outxy24(xLeft + 15, yTop+i*yHeight, 2, cBlack, 1, 1, displayinfo.content_row[i], 0, 0);
   }
}
//---------------------------------------------------------------------------
void OperateInfoContent(short wintype, int currwindow)    //��Ϣ��ʾ���򣨶�����  ��Ϣ���� ����
{
  int i;
  int TmpInfoNo;
  InfoNode1 *tmp_node;
  int numperpage;
  int xLeft, yTop, yHeight;

  if(((wintype >= 0) && (wintype <= 2)))
   {

    DisplayImageButton(&infowin_button[wintype], IMAGEDOWN, 0);
    usleep(DELAYTIME*1000);
    if(Local.CurrentWindow != 77)
      return;
    DisplayImageButton(&infowin_button[wintype], IMAGEUP, 0);
   }

  switch(wintype)
   {
    case 0://��Ϣ�Ϸ�           o
            if(displayinfo.pageno > 0)
             {
              RestorePicBuf_Func(infowin_image.xLeft + 25, infowin_image.yTop + 62, 432, 110, PicStatBuf.InfoContent1, 0);
              xLeft = infowin_image.xLeft + 15;
              yTop = infowin_image.yTop + 68;
              yHeight = 38;
              displayinfo.pageno --;
              if(displayinfo.totalpage == (displayinfo.pageno + 1))
                numperpage = displayinfo.totalrow - displayinfo.pageno*PAGEPERROW;
              else
                numperpage = PAGEPERROW;
              for(i=0; i<numperpage; i++)
                outxy24(xLeft + 10, yTop+(i)*yHeight,
                      2, cBlack, 1, 1, displayinfo.content_row[i+displayinfo.pageno*PAGEPERROW], 0, 0);
             }
           break;
    case 1://��Ϣ�·�           p
            if(displayinfo.pageno < (displayinfo.totalpage - 1))
             {
              RestorePicBuf_Func(infowin_image.xLeft + 25, infowin_image.yTop + 62, 432, 110, PicStatBuf.InfoContent1, 0);
              xLeft = infowin_image.xLeft + 15;
              yTop = infowin_image.yTop + 68;
              yHeight = 38;
              displayinfo.pageno ++;
              if(displayinfo.totalpage == (displayinfo.pageno + 1))
                numperpage = displayinfo.totalrow - displayinfo.pageno*PAGEPERROW;
              else
                numperpage = PAGEPERROW;
              for(i=0; i<numperpage; i++)
                outxy24(xLeft + 10, yTop+(i)*yHeight,
                      2, cBlack, 1, 1, displayinfo.content_row[i+displayinfo.pageno*PAGEPERROW], 0, 0);
             }
           break;
    case 2: //����             h

          //�ָ���Ļ
           DisplayPopupWin(&InfoWin, HIDE, 0);
           Local.CurrentWindow = 4;
           if(displayinfo.isDeleted == 1)  //����ʾ��Ϣ����ʱɾ������Ϣ
             ShowInfoList(InfoStatus.CurrType);
           break;
   }         
}
//---------------------------------------------------------------------------
void CreateContentRow(char *Content, int nLength, char create_row[6][40])  //����Ҫ��ʾ��������
{
  int i,j;
  int row;
  int asciinum;
  row = 0;
  for(i=0; i<MAXROW; i++)
   for(j=0; j<(INFOROWLEN + 8); j++)
    create_row[i][j] = '\0';
  j = 0;
  asciinum = 0;
  for(i=0; i<nLength; i++)
   {
    if((Content[i] == '\r')&&(Content[i+1] == '\n'))
     {
      create_row[row][j] = '\0';
      row ++;
      asciinum = 0;
      j = 0;
      i++;
     }
    else
     {
      if((unsigned char)Content[i] >= 0xA1)
       {
        create_row[row][j] = Content[i];
        i ++;
        j ++;
        create_row[row][j] = Content[i];
        j ++;
       }
      else
       {
          create_row[row][j] = Content[i];
          j ++;
          asciinum ++;
       }
      if(j >= (INFOROWLEN-1))
       {
    //    printf("j = %d, asciinum = %d, i = %d\n", j, asciinum, i);
        if((asciinum % 2) != 0)
         {
          create_row[row][j] = ' ';
         }
        create_row[row][j+1] = '\0';
        row ++;
        asciinum = 0;
        j = 0;
       }
     }
    if(row >= MAXROW)
      break;
   }
  displayinfo.totalrow = row+1;
  if((displayinfo.totalrow % PAGEPERROW) == 0)
    displayinfo.totalpage = displayinfo.totalrow / PAGEPERROW;
  else
    displayinfo.totalpage = displayinfo.totalrow / PAGEPERROW + 1;
}
//---------------------------------------------------------------------------
void ShowInfoNum(int infotype)  //��ʾ��Ϣ������δ����Ϣ����
{
  char str[20];
  char infonum[20];
  char jpgfilename[80];
  char infotypename[4][20] = {"��ͨ��Ϣ", "������Ϣ", "����Ԥ��", "��������"};
  int xLeft, yTop;

  xLeft = 50;
  yTop = 79;
  strcpy(jpgfilename, sPath);
  strcat(jpgfilename,"infolist.jpg");
  DisplayJPG(xLeft, yTop, jpgfilename, 1, 0, 0, 0);

  outxy24(xLeft + 20, yTop + 12, 3, cWhite,
          1, 1, infotypename[infotype], 0, 0);

  sprintf(infonum, "%d/%d\0" , Info[infotype].NoReadedNum, Info[infotype].TotalNum);
  outxy24(xLeft + 240, yTop + 12, 3, cWhite,
          1, 1, infonum, 0, 0);
}
//---------------------------------------------------------------------------
//д��Ϣ�ļ�
void WriteInfoFile(int InfoType)
{
  FILE *info_fd;
  InfoNode1 *tmp_node;
  int SeekLength;
  int j;
  uint8_t isValid;
  if((info_fd = fopen(info_name, "rb+")) == NULL)
    printf("�޷�����Ϣ�ļ�\n");
  else
   {
    //��д������Ϣ�ļ�
    SeekLength = 0;
    for(j=0; j<InfoType; j++)
      SeekLength += Info[j].MaxNum;
    fseek(info_fd, sizeof(tmp_node->Content)*SeekLength, SEEK_SET);
    tmp_node=InfoNode_h[InfoType];
    //д������Ϣ
    for(j = 0; j < Info[InfoType].TotalNum; j++)
     {
      tmp_node = tmp_node->rlink;
      if(tmp_node != NULL)
        fwrite(&tmp_node->Content, sizeof(tmp_node->Content), 1, info_fd);
     }
    //д������Ϣ
    isValid = 0;
    for(j = Info[InfoType].TotalNum; j < Info[InfoType].MaxNum; j++)
     {
      fwrite(&isValid, sizeof(isValid), 1, info_fd);
      fseek(info_fd, (sizeof(tmp_node->Content)-1), SEEK_CUR);
     }
    fclose(info_fd);
   }
}

//---------------------------------------------------------------------------
//д���ļ� ������Ϣ
void WriteInfoFileLock(int InfoType, int InfoNo, InfoNode1 *Info_Node)
{
  FILE *info_fd;
  int SeekLength;
  int j;
  uint8_t isValid;
  if((info_fd = fopen(info_name, "rb+")) == NULL)
    printf("�޷�����Ϣ�ļ�\n");
  else
   {
    //��д������Ϣ�ļ�
    SeekLength = 0;
    for(j=0; j<InfoType; j++)
      SeekLength += Info[j].MaxNum;
    fseek(info_fd, sizeof(Info_Node->Content)*SeekLength, SEEK_SET);
    fseek(info_fd, sizeof(Info_Node->Content)*InfoNo, SEEK_CUR);

    fwrite(&Info_Node->Content, sizeof(Info_Node->Content), 1, info_fd);

    fclose(info_fd);
   }
}
//---------------------------------------------------------------------------

