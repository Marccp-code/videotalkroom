#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>       //sem_t

#define CommonH
#include "common.h"
void DisplayPictureWindow(int pictype);    //��ʾ��Ƭ���ڣ�һ����
void OperatePictureWindow(short wintype, int currwindow);    //��Ƭ���ڲ�����������

void ShowPicList(int pictype);    //��Ƭ��ʾ����һ����  ��Ƭ�б�
void DisplayPicContent(int pictype, int picno);    //��Ƭ��ʾ���򣨶�����  ��Ƭ����
void OperatePicContent(short pictype, int currwindow);    //��Ƭ��ʾ���򣨶�����  ��Ƭ���� ����
void ShowPicContent(int pictype, int picno);    // ��Ƭ����
//д��Ƭ�����ļ�
void WritePicIniFile(int PicType);
//��ǰ��Ϣ����״̬
struct InfoStatus1 PicStatus;

InfoNode1 *CurrPic_Node; //��ǰ��Ϣ���
//��Ƭ�����ṹ
extern struct Info1 PicStrc[2];
// *h�����ͷ����ָ�룬*pָ��ǰ����ǰһ����㣬*sָ��ǰ���
extern InfoNode1 *PicNode_h[2]; //��Ƭ

struct displayinfo1
 {
 	 int totalpage;
	  int pageno;
	  int totalrow;
	  char content_row[MAXROW][40];
	  int isDeleted;  //����ʾ��Ϣ����ʱɾ������Ϣ
 }displayinfo;
//---------------------------------------------------------------------------
void DisplayPictureWindow(int pictype)    //��ʾ��Ƭ���ڣ�һ����
{
  int i;
  int xTop;
  char jpgfilename[80];
  int InfoTypeX[5] = {29, 29, 513, 513, 513};
  int InfoTypeY[5] = {127, 175, 223, 271, 319};
  int InfoRowX[3] = {47, 47, 47};
  int InfoRowY[3] = {203, 252, 303};
  xTop = 36;
  strcpy(jpgfilename, sPath);
  strcat(jpgfilename,"picture.jpg");
  DisplayJPG(0, 0, jpgfilename, 1, SCRWIDTH, SCRHEIGHT, 0);

  //��Ƭ����
  PicStatus.CurrType = pictype;  //��ǰ��Ƭ����
  PicStatus.CurrWin = 0;    //��Ƭ�б�
  PicStatus.CurrNo = 0;       //��ǰ��Ƭ���  0---n-1
  PicStrc[PicStatus.CurrType].CurrentInfoPage = 1; //��ǰ��ƬҳΪ��0ҳ
  PicStrc[PicStatus.CurrType].CurrNo = PicStatus.CurrNo;

  for(i=0; i<3; i++)
   {
    inforow_button[i].xLeft = InfoRowX[i];
    inforow_button[i].yTop = InfoRowY[i];
   }

  ShowPicList(PicStatus.CurrType);
  
  Local.PrevWindow = Local.CurrentWindow;
  Local.CurrentWindow = 21;

  for(i=0; i<3; i++)
    DisplayImageButton(&info_button[i], IMAGEUP, 0);

  ioctl(gpio_fd, IO_CLEAR, 3);
}
//---------------------------------------------------------------------------
void OperatePictureWindow(short wintype, int currwindow)    //��Ƭ���ڲ�����������
{
  int i;
  int TmpPicNo;
  InfoNode1 *tmp_node;
  int numperpage;
  int xLeft, yTop, yHeight;

  if(((wintype >= 0) && (wintype <= 2)))
   {

    DisplayImageButton(&info_button[wintype], IMAGEDOWN , 0);
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

  if(Local.HavePicRecorded_flag == 1) Local.HavePicRecorded_flag = 0;
  
  switch(wintype)
   {
    case 0://�Ϸ�       a
             if(PicStrc[PicStatus.CurrType].CurrNo > 0)
              {
               PicStrc[PicStatus.CurrType].CurrNo --;
               PicStatus.CurrNo = PicStrc[PicStatus.CurrType].CurrNo;
               ShowPicList(PicStatus.CurrType);
              }
           break;
    case 1://�·�     b
             if(PicStrc[PicStatus.CurrType].CurrNo < (PicStrc[PicStatus.CurrType].TotalNum -1))
              {
               PicStrc[PicStatus.CurrType].CurrNo++;
               PicStatus.CurrNo = PicStrc[PicStatus.CurrType].CurrNo;
               ShowPicList(PicStatus.CurrType);
              }
           break;
    case 2://ɾ��           c
              //ɾ����ǰ��Ϣ
             //��ǰ��Ϣ���
             CurrPic_Node=locate_infonode(PicNode_h[PicStatus.CurrType], PicStrc[PicStatus.CurrType].CurrNo + 1);
             if(CurrPic_Node != NULL)
              {
                displayinfo.isDeleted = 1;  //����ʾ��Ϣ����ʱɾ������Ϣ
                unlink(CurrPic_Node->Content);  //ɾ����Ƭ�ļ�
                delete_infonode(CurrPic_Node);
                PicStrc[PicStatus.CurrType].TotalNum --;
                //д���ļ�
                //����������
                pthread_mutex_lock (&Local.save_lock);
                //���ҿ��ô洢���岢���
                for(i=0; i<SAVEMAX; i++)
                 if(Save_File_Buff[i].isValid == 0)
                  {
                   Save_File_Buff[i].Type = 7;
                   Save_File_Buff[i].InfoType = PicStatus.CurrType;
                   Save_File_Buff[i].isValid = 1;
                   break;
                  }
                //�򿪻�����
                pthread_mutex_unlock (&Local.save_lock);
                sem_post(&save_file_sem);

                ShowPicList(PicStatus.CurrType);
             }
           break;
    case 4://��Ϣ1��           c
    case 5://��Ϣ2��           c
    case 6://��Ϣ3��           c
           //��ǰ��Ϣ���
             TmpPicNo = (PicStrc[PicStatus.CurrType].CurrentInfoPage-1)*INFONUMPERPAGE+(wintype - 4);
             printf("TmpPicNo = %d, PicStrc[PicStatus.CurrType].TotalNum = %d \n", TmpPicNo,
                    PicStrc[PicStatus.CurrType].TotalNum);
             if(PicStatus.CurrNo == TmpPicNo)
              {
               if(TmpPicNo < (PicStrc[PicStatus.CurrType].TotalNum))
                {
                 PicStrc[PicStatus.CurrType].CurrNo = TmpPicNo;
                 PicStatus.CurrNo = TmpPicNo;
                 DisplayPicContent(PicStatus.CurrType, PicStatus.CurrNo);
                }
              }
             else
              {
               if(TmpPicNo < (PicStrc[PicStatus.CurrType].TotalNum))
                {
                 PicStrc[PicStatus.CurrType].CurrNo = TmpPicNo;
                 PicStatus.CurrNo = TmpPicNo;
                 ShowPicList(PicStatus.CurrType);
                }
              }
           break;
    case 30:  //��ҳ
    case 31:  //����
    case 32:  //�ҵ�
    case 33:  //�Խ�
    case 34:  //��Ϣ
    case 35:  //����
           DisplayMainWindow(wintype - 30);
           break;
   }
}
//---------------------------------------------------------------------------
void ShowPicList(int pictype)    //��Ƭ��ʾ����һ����  ��Ƭ�б�
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
  xLeft = 120;
  yTop = 36;
  tmp_len = 19;


  PicStatus.CurrWin = 0;    //��Ϣ�б�
  printf("PicStatus.CurrType  = %d\n", PicStatus.CurrType);
  switch(pictype)
   {
    case 0: //������Ƭ
    case 1: //ͨ����Ƭ
           for(i = 0; i < INFONUMPERPAGE; i++)
              DisplayImageButton(&inforow_button[i], IMAGEUP, 0);
           if(PicStrc[pictype].TotalNum > 0)
            {
             //��ҳ��
             if((PicStrc[pictype].TotalNum % INFONUMPERPAGE) == 0)
               PicStrc[pictype].TotalInfoPage = PicStrc[pictype].TotalNum /INFONUMPERPAGE;
             else
               PicStrc[pictype].TotalInfoPage = PicStrc[pictype].TotalNum /INFONUMPERPAGE + 1;

             xLeft = 300;
             yTop = 50;
             yHeight = 40;
             //��ǰҳ
             PicStrc[pictype].CurrentInfoPage = PicStatus.CurrNo /INFONUMPERPAGE + 1;
             if(PicStrc[pictype].CurrentInfoPage < PicStrc[pictype].TotalInfoPage)
               PageTotalNum = INFONUMPERPAGE;
             else
               PageTotalNum = PicStrc[pictype].TotalNum - (PicStrc[pictype].CurrentInfoPage - 1)*INFONUMPERPAGE;
             //��ǰ��Ϣ�ڱ�ҳ�е�λ��
             NoInPage = (PicStatus.CurrNo)%INFONUMPERPAGE;

             DisplayImageButton(&inforow_button[NoInPage], IMAGEDOWN, 0);

             for(i = 0; i < PageTotalNum; i++)
              {
               if(i !=  NoInPage)
                 fontcolor = cBlack;
               else
                 fontcolor = cWhite;

               tmp_node=locate_infonode(PicNode_h[pictype], (PicStrc[pictype].CurrentInfoPage-1)*INFONUMPERPAGE+i+1);

               //���
               sprintf(tmp_con, "%02d\0", (PicStrc[pictype].CurrentInfoPage-1)*INFONUMPERPAGE+i+1);
               outxy24(inforow_button[i].xLeft+50, inforow_button[i].yTop+10, 1,
                       fontcolor, 1, 1, tmp_con, 0, 0);

               if(tmp_node->Content.Type == 0)
                 outxy24(inforow_button[i].xLeft+100, inforow_button[i].yTop+10, 1,
                         fontcolor, 1, 1, "δ����", 0, 0);
               else
                 outxy24(inforow_button[i].xLeft+100, inforow_button[i].yTop+10, 1,
                         fontcolor, 1, 1, "�ѽ���", 0, 0);
               //ʱ��
               memcpy(tmp_con, tmp_node->Content.Time, tmp_len);
                 tmp_con[tmp_len] = '\0';
               outxy24(inforow_button[i].xLeft+250, inforow_button[i].yTop+10, 1,
                       fontcolor, 1, 1, tmp_con, 0, 0);
              }
            }
           break;
    case 4:  //����
           break;
   }
}
//---------------------------------------------------------------------------
void DisplayPicContent(int pictype, int picno)   
{
  int i;
  char jpgfilename[80];

  displayinfo.isDeleted = 0;  //����ʾ��Ϣ����ʱɾ������Ϣ

  strcpy(jpgfilename, sPath);
  strcat(jpgfilename,"picwin.jpg");
  DisplayJPG(175, 22, jpgfilename, 1, SCRWIDTH, SCRHEIGHT, 0);

  Local.CurrentWindow = 22;

  for(i=0; i<3; i++)
    DisplayImageButton(&picwin_button[i], IMAGEUP, 0);

  PicStatus.CurrWin = 1;    //��Ϣ����

  if(PicStatBuf.PicContent == NULL)
   {
    PicStatBuf.PicContent = (unsigned char *)malloc(82*80*3/2);  //����һ����Ļ����
    SavePicBuf_Func(379, 18, 82, 80, PicStatBuf.PicContent, 0);
   }

  ShowPicContent(pictype, picno);
}
//---------------------------------------------------------------------------
void ShowPicContent(int pictype, int picno)    // ��Ƭ����
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

  xLeft = 379;
  yTop = 18;
  tmp_len = 20;  

  yHeight = 20;
  if(picno <= PicStrc[pictype].TotalNum - 1)
   {
    printf("picno=%d\n",picno + 1);
    tmp_node=locate_infonode(PicNode_h[pictype], picno + 1);
    //��ǰ��Ϣ���
    CurrPic_Node = tmp_node;

    DisplayJPG(239, 61, tmp_node->Content.Content, 1, SCRWIDTH, SCRHEIGHT, 0);
   }
}
//---------------------------------------------------------------------------
void OperatePicContent(short wintype, int currwindow)    //��Ƭ��ʾ���򣨶�����  ��Ƭ���� ����
{
  int i;
  int TmpInfoNo;
  InfoNode1 *tmp_node;
  int numperpage;
  int xLeft, yTop, yHeight;
  char jpgfilename[80];

  if(((wintype >= 0) && (wintype <= 2)))
   {
    DisplayImageButton(&picwin_button[wintype], IMAGEDOWN, 0);
    usleep(DELAYTIME*1000);
    if(Local.CurrentWindow != 22)
      return;
    DisplayImageButton(&picwin_button[wintype], IMAGEUP, 0);
   }

  switch(wintype)
   {
    case 0://��Ϣ�Ϸ�           o
             if(PicStrc[PicStatus.CurrType].CurrNo > 0)
              {
               PicStrc[PicStatus.CurrType].CurrNo --;
               PicStatus.CurrNo = PicStrc[PicStatus.CurrType].CurrNo;
               ShowPicContent(PicStatus.CurrType, PicStatus.CurrNo);
              }
           break;
    case 1://��Ϣ�·�           p
             if(PicStrc[PicStatus.CurrType].CurrNo < (PicStrc[PicStatus.CurrType].TotalNum -1))
              {
               PicStrc[PicStatus.CurrType].CurrNo++;
               PicStatus.CurrNo = PicStrc[PicStatus.CurrType].CurrNo;
               ShowPicContent(PicStatus.CurrType, PicStatus.CurrNo);
              }
           break;
    case 2: //����             h
          //�ָ���Ļ
           //DisplayPictureWindow(PicStatus.CurrType);
           strcpy(jpgfilename, sPath);
           strcat(jpgfilename,"picture.jpg");
           DisplayJPG(0, 0, jpgfilename, 1, SCRWIDTH, SCRHEIGHT, 0);


           ShowPicList(PicStatus.CurrType);
  
           Local.CurrentWindow = 21;

           for(i=0; i<3; i++)
             DisplayImageButton(&info_button[i], IMAGEUP, 0);

           break;
   }
}
//---------------------------------------------------------------------------
//д��Ƭ�����ļ�
void WritePicIniFile(int PicType)
{
  FILE *read_fd;
  unsigned char tmpchar[30];
  unsigned char readname[80];

  FILE *pic_fd;
  InfoNode1 *tmp_node;
  int j;
  uint8_t isValid;
  char filename[80];  
  //д��Ƭ�����ļ�
  if(PicType == 0)
    strcpy(filename, picini_name1);
  else
    strcpy(filename, picini_name2);
 // printf("PicType = %d\n", PicType);
  if((pic_fd = fopen(filename, "wb")) == NULL)
    printf("�޷�����Ƭ�����ļ�\n");
  else
   {
    //��д������Ϣ�ļ�
    tmp_node=PicNode_h[PicType];
    //д������Ϣ
    for(j = 0; j < PicStrc[PicType].TotalNum; j++)
     {
      tmp_node = tmp_node->rlink;
      if(tmp_node != NULL)
        fwrite(&tmp_node->Content, sizeof(tmp_node->Content), 1, pic_fd);
     }
    fclose(pic_fd);
   }
}
//---------------------------------------------------------------------------

