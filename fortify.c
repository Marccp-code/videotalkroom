#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>       //sem_t

#define CommonH
#include "common.h"

void DisplayCancelFortifyWindow(void);    //��ʾ�������ڣ�������
void OperateCancelFortifyWindow(short wintype, int currwindow);    //�������ڲ�����������
void DisplayFortifyWindow(void);    //��ʾ�������ڣ�������
void OperateFortifyWindow(short wintype, int currwindow);    //�������ڲ�����������
void DisplayAlarmWindow(void);  //��ʾ�������ڣ�������
void OperateAlarmWindow(short wintype, int currwindow);    //�������ڲ�����������
void DisplayCancelAlarmWindow(void);    //��ʾȡ���������ڣ�������
void OperateCancelAlarmWindow(short wintype, int currwindow);    //ȡ���������ڲ�����������
//---------------------------------------------------------------------------
void DisplayCancelFortifyWindow(void)    //��ʾ�������ڣ�������
{
  int yTop;
  int i;
  char jpgfilename[80];
  char infonum[20];
  char str[3];
  yTop = 36;

  strcpy(jpgfilename, sPath);
  strcat(jpgfilename,"fortify.jpg");
  DisplayJPG(0, 0, jpgfilename, 1, SCRWIDTH, SCRHEIGHT, 0);

  //�洢����
  SavePicBuf_Func(CancelFortifyWin.xLeft, CancelFortifyWin.yTop, CancelFortifyWin.width, CancelFortifyWin.height, CancelFortifyWin.image[1], 0);
  DisplayPopupWin(&CancelFortifyWin, SHOW, 0);
  Local.TmpWindow = Local.CurrentWindow;
  Local.CurrentWindow = 31;

  outxy24(167, 161, 2, cBlack, 1, 1, "�������볷��", 0, 0);

  for(i=0; i<12; i++)
   {
    num1_button[i].xLeft = NumxLeft2[i];
    num1_button[i].yTop = NumyTop2[i];
   }

  cancelfortify_button[0].xLeft = 132;
  cancelfortify_button[0].yTop = 272;

  cancelfortify_button[1].xLeft = 212;
  cancelfortify_button[1].yTop = 272;

  cancelfortify_button[2].xLeft = 290;
  cancelfortify_button[2].yTop = 272;

  cancelfortify_edit.xLeft = 156;
  cancelfortify_edit.yTop = 202;
  cancelfortify_edit.BoxLen = 0;
  cancelfortify_edit.Text[0] = '\0';
  CurrBox = 0;
  //�򿪹��
  WriteCursor(&cancelfortify_edit, 1, 1, 0);
  DisplayEdit(&cancelfortify_edit, 0);

  for(i=0; i<3; i++)
    DisplayImageButton(&cancelfortify_button[i], IMAGEUP, 0);
  for(i=0; i<12; i++)
    DisplayImageButton(&num1_button[i], IMAGEUP, 0);


  Local.PassLen = 0;

  Local.CurrentWindow = 31;
}
//---------------------------------------------------------------------------
void OperateCancelFortifyWindow(short wintype, int currwindow)    //�������ڲ�����������
{
  char wavFile[80];
  int xLeft,yTop;
  int i, j;
  char jpgfilename[80];
  char str[3];
  xLeft = 120;
  yTop = 36;

  if(wintype < 12)
   {
    DisplayImageButton(&num1_button[wintype], IMAGEDOWN, 0);
    usleep(DELAYTIME*1000);
    if(Local.CurrentWindow != currwindow)
      return;
    DisplayImageButton(&num1_button[wintype], IMAGEUP, 0);
   }
  if((wintype >= 12)&&(wintype <= 14))
   {
    DisplayImageButton(&cancelfortify_button[wintype-12], IMAGEDOWN, 0);
    usleep(DELAYTIME*1000);
    if(Local.CurrentWindow != currwindow)
      return;
    DisplayImageButton(&cancelfortify_button[wintype-12], IMAGEUP, 0);
   }

  //ֹͣ���,����һ��
  WriteCursor(&cancelfortify_edit, 0, 0, 0);
  switch(wintype)
   {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
           if(cancelfortify_edit.BoxLen < cancelfortify_edit.MaxLen)
            {
             str[0] = '0' + wintype;
             str[1] = '\0';
             strcat(cancelfortify_edit.Text, str);
             str[0] = '*';
             str[1] = '\0';
             outxy24(cancelfortify_edit.xLeft + cancelfortify_edit.CursorX + cancelfortify_edit.BoxLen*cancelfortify_edit.fWidth,
                     cancelfortify_edit.yTop + cancelfortify_edit.CursorY, 2, cBlack, 1, 1, str, 0, 0);
             cancelfortify_edit.BoxLen ++;
            }
           break;
    case 10: //���
    case 13: //���
          cancelfortify_edit.Text[0] = 0;
          cancelfortify_edit.BoxLen = 0;
          DisplayEdit(&cancelfortify_edit, 0);
          break;
    case 11: //ȷ��
    case 12:  //ȷ��
          printf("cancelfortify_edit.BoxLen = %d\n", cancelfortify_edit.BoxLen);
          cancelfortify_edit.Text[cancelfortify_edit.BoxLen] = '\0';
          LocalCfg.EngineerPass[4] = '\0';
          if(strcmp(cancelfortify_edit.Text, LocalCfg.EngineerPass) == 0)
           {
        //     ShowStatusText(0, 0 , 3, cBlack, 1, 1, "�����ɹ�", 0);
              if((Local.AlarmDelayFlag[0] == 1)||(Local.AlarmDelayFlag[1] == 1))
               {
                StopPlayWavFile();  //�رձ�����ʱ��ʾ��
                usleep(200*1000);  //��ʱ��Ϊ�˵ȴ�����������ɣ������������
               }
              for(i=0; i<2; i++)
               {
                Local.AlarmDelayFlag[i] = 0;     //������ʱ��־
                Local.AlarmDelayTime[i] = 0;
               }

              sprintf(wavFile, "%scancelfortify.wav\0", wavPath);
              StartPlayWav(wavFile, 0);


              //��������
              cancelfortify_edit.Text[0] = 0;
              cancelfortify_edit.BoxLen = 0;
              DisplayEdit(&cancelfortify_edit, 0);

              ioctl(gpio_fd, IO_CLEAR, 2);
              LocalCfg.DefenceStatus = 0;
                //����״̬
                if(LocalCfg.DefenceStatus == 0)
                 if(Local.CurrentWindow == 0)
                  DisplayImage(&state_image[1], 0);
                else
                 if(Local.CurrentWindow == 0)
                  DisplayImage(&state_image[0], 0);              
              DisplayFortifyWindow();
           }
          else
             {
              sprintf(wavFile, "%spasserror.wav\0", wavPath);
              StartPlayWav(wavFile, 0);
              cancelfortify_edit.Text[0] = 0;
              cancelfortify_edit.BoxLen = 0;
              DisplayEdit(&cancelfortify_edit, 0);
             }
          break;
    case 14:  //����
           DisplayMainWindow(0);
           break;
   }
   if(Local.CurrentWindow == 31)
     //�򿪹��
       WriteCursor(&cancelfortify_edit, 1, 1, 0);
}
//---------------------------------------------------------------------------
void DisplayFortifyWindow(void)    //��ʾ�������ڣ�������
{
  int yTop;
  int i;
  char jpgfilename[80];
  char infonum[20];
  char str[3];

  if(Local.PrevWindow != 32)
   {
    Local.PrevWindow = Local.CurrentWindow;
   }
  Local.CurrentWindow = 32;

  yTop = 36;
  strcpy(jpgfilename, sPath);
  strcat(jpgfilename,"fortify.jpg");
  DisplayJPG(0, 0, jpgfilename, 1, SCRWIDTH, SCRHEIGHT, 0);
  //�������ڣ�һ������ʾ��
  if(fortify_label.image == NULL)
   {
    fortify_label.width = TIPW;
    fortify_label.height = TIPH;
    fortify_label.xLeft = SETUP1X;
    fortify_label.yTop = SETUP1Y;
    fortify_label.image = (unsigned char *)malloc(TIPW*TIPH*3/2);  //����һ����Ļ����
    SavePicBuf_Func(SETUP1X, SETUP1Y, TIPW, TIPH, fortify_label.image, 0);
   }

  for(i=0; i<3; i++)
    DisplayImageButton(&fortify_button[i], IMAGEUP, 0);
}
//---------------------------------------------------------------------------
void OperateFortifyWindow(short wintype, int currwindow)    //�������ڲ�����������
{
  char wavFile[80];
  int xLeft,yTop;
  int i, j;
  char jpgfilename[80];
  char str[3];
  xLeft = 120;
  yTop = 36;
  printf("wintype = %d\n", wintype);
  printf("DefenceCfg.DefenceStatus = %d\n", LocalCfg.DefenceStatus);
  if((wintype >= 2)&&(wintype < 3))
   {
    DisplayImageButton(&fortify_button[wintype], IMAGEDOWN, 0);
    usleep(DELAYTIME*1000);
    if(Local.CurrentWindow != currwindow)
      return;
    DisplayImageButton(&fortify_button[wintype], IMAGEUP, 0);
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
    case 0: //�������
           if((LocalCfg.DefenceStatus !=1)&&(LocalCfg.DefenceStatus !=4))
            {
             DisplayImageButton(&fortify_button[0], IMAGEDOWN, 0);
             DisplayImageButton(&fortify_button[1], IMAGEUP, 0);
             usleep(200*1000);
             sprintf(wavFile, "%sfortifydelay.wav\0", wavPath);
             StartPlayWav(wavFile, 0);

             LocalCfg.DefenceStatus = 4;
             Local.DefenceDelayFlag = 1;    //������ʱ��־
             Local.DefenceDelayTime = 0;    //����
            }
           break;
    case 1: //�ڼҲ���
           if((LocalCfg.DefenceStatus !=2)&&(LocalCfg.DefenceStatus !=5))
            {
             DisplayImageButton(&fortify_button[0], IMAGEUP, 0);
             DisplayImageButton(&fortify_button[1], IMAGEDOWN, 0);
             usleep(200*1000);
             sprintf(wavFile, "%sfortifydelay.wav\0", wavPath);
             StartPlayWav(wavFile, 0);
                    
             LocalCfg.DefenceStatus = 5;
             Local.DefenceDelayFlag = 1;    //������ʱ��־
             Local.DefenceDelayTime = 0;    //����
            }
           break;
    case 2: //ȡ��������ʱ
             ioctl(gpio_fd, IO_CLEAR, 2); 
             LocalCfg.DefenceStatus = 0;
             Local.DefenceDelayFlag = 0;    //������ʱ��־
             Local.DefenceDelayTime = 0;    //����
             for(i=0; i<3; i++)
               DisplayImageButton(&fortify_button[i], IMAGEUP, 0);
           break;
    case 30:  //��ҳ
    //case 31:  //����
    case 32:  //�ҵ�
    case 33:  //�Խ�
    case 34:  //��Ϣ
    case 35:  //����
           DisplayMainWindow(wintype - 30);
           break;
   }
}
//---------------------------------------------------------------------------
void DisplayAlarmWindow(void)  //��ʾ�������ڣ�������
{
  int xLeft,yTop;
  int i, j;
  char jpgfilename[80];
  char alarm_str[10];
  char tmp_str[10];
  char wavFile[80];
  xLeft = 120;
  yTop = 36;

  PicStatBuf.Flag = 0;

  if(Local.CurrentWindow != 34)
   {
    strcpy(jpgfilename, sPath);
    strcat(jpgfilename,"alarm.jpg");
    DisplayJPG(0, 0, jpgfilename, 1, SCRWIDTH, SCRHEIGHT, 0);

    DisplayImageButton(&alarm_button[0], IMAGEUP, 0);

    sprintf(wavFile, "%salarm.wav\0", wavPath);
    StartPlayWav(wavFile, 1);
   }
  for(i=0; i<6; i++)
   if(LocalCfg.DefenceInfo[i][3] != 0)
    {
     sprintf(tmp_str, "%d\0", i+1);
     outxy16(325+16*i, 413, 3, cWhite, 1, 1, tmp_str, 0, 0);
    }
  AlarmGif.Visible = 1;

  Local.CurrentWindow = 34;
}
//---------------------------------------------------------------------------
void OperateAlarmWindow(short wintype, int currwindow)    //�������ڲ�����������
{
  int xLeft,yTop;
  int i, j;
  char jpgfilename[80];
  char str[3];
  xLeft = 120;
  yTop = 36;
  if(wintype < 1)
   {
    DisplayImageButton(&alarm_button[wintype], IMAGEDOWN, 0);
    usleep(DELAYTIME*1000);
    if(Local.CurrentWindow != currwindow)
      return;
    DisplayImageButton(&alarm_button[wintype], IMAGEUP, 0);
   }
  switch(wintype)
   {
    case 0:  //ȡ������
           AlarmGif.Visible = 0;
           StopPlayWavFile();  //�رձ�����
           usleep(200*1000);  //��ʱ��Ϊ�˵ȴ�����������ɣ������������
           DisplayCancelAlarmWindow();
           break;
   }
}
//---------------------------------------------------------------------------
void DisplayCancelAlarmWindow(void)    //��ʾȡ���������ڣ�������
{
  int yTop;
  int i;
  char jpgfilename[80];
  char infonum[20];
  char str[3];
  yTop = 36;

  strcpy(jpgfilename, sPath);
  strcat(jpgfilename,"fortify.jpg");
  DisplayJPG(0, 0, jpgfilename, 1, SCRWIDTH, SCRHEIGHT, 0);

  //�洢����
  SavePicBuf_Func(CancelFortifyWin.xLeft, CancelFortifyWin.yTop, CancelFortifyWin.width, CancelFortifyWin.height, CancelFortifyWin.image[1], 0);
  DisplayPopupWin(&CancelFortifyWin, SHOW, 0);
  Local.TmpWindow = Local.CurrentWindow;
  Local.CurrentWindow = 31;

  outxy24(167, 161, 2, cBlack, 1, 1, "������������", 0, 0);

  for(i=0; i<12; i++)
   {
    num1_button[i].xLeft = NumxLeft2[i];
    num1_button[i].yTop = NumyTop2[i];
   }

  cancelfortify_button[0].xLeft = 150;
  cancelfortify_button[0].yTop = 272;

  cancelfortify_button[1].xLeft = 272;
  cancelfortify_button[1].yTop = 272;

  cancelfortify_edit.xLeft = 156;
  cancelfortify_edit.yTop = 202;
  cancelfortify_edit.BoxLen = 0;
  cancelfortify_edit.Text[0] = '\0';
  CurrBox = 0;
  //�򿪹��
  WriteCursor(&cancelfortify_edit, 1, 1, 0);
  DisplayEdit(&cancelfortify_edit, 0);

  for(i=0; i<2; i++)
    DisplayImageButton(&cancelfortify_button[i], IMAGEUP, 0);
  for(i=0; i<12; i++)
    DisplayImageButton(&num1_button[i], IMAGEUP, 0);


  Local.PassLen = 0;

  Local.CurrentWindow = 35;
}
//---------------------------------------------------------------------------
void OperateCancelAlarmWindow(short wintype, int currwindow)    //ȡ���������ڲ�����������
{
  char wavFile[80];
  int xLeft,yTop;
  int i, j;
  char jpgfilename[80];
  char str[3];
  int Special;
  char SpecialPass[10];
  xLeft = 120;
  yTop = 36;

  if(wintype < 12)
   {
    DisplayImageButton(&num1_button[wintype], IMAGEDOWN, 0);
    usleep(DELAYTIME*1000);
    if(Local.CurrentWindow != currwindow)
      return;
    DisplayImageButton(&num1_button[wintype], IMAGEUP, 0);
   }
  if((wintype >= 12)&&(wintype <= 13))
   {
    DisplayImageButton(&cancelfortify_button[wintype-12], IMAGEDOWN, 0);
    usleep(DELAYTIME*1000);
    if(Local.CurrentWindow != currwindow)
      return;
    DisplayImageButton(&cancelfortify_button[wintype-12], IMAGEUP, 0);
   }

  //ֹͣ���,����һ��
  WriteCursor(&cancelfortify_edit, 0, 0, 0);
  switch(wintype)
   {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
           if(cancelfortify_edit.BoxLen < cancelfortify_edit.MaxLen)
            {
             str[0] = '0' + wintype;
             str[1] = '\0';
             strcat(cancelfortify_edit.Text, str);
             str[0] = '*';
             str[1] = '\0';
             outxy16(cancelfortify_edit.xLeft + cancelfortify_edit.CursorX + cancelfortify_edit.BoxLen*cancelfortify_edit.fWidth,
                     cancelfortify_edit.yTop + cancelfortify_edit.CursorY, 2, cBlack, 1, 1, str, 0, 0);
             cancelfortify_edit.BoxLen ++;
            }
           break;
    case 10: //���
    case 13: //���
          cancelfortify_edit.Text[0] = 0;
          cancelfortify_edit.BoxLen = 0;
          DisplayEdit(&cancelfortify_edit, 0);
          break;
    case 11: //ȷ��
    case 12:  //ȷ��
          printf("cancelfortify_edit.BoxLen = %d\n", cancelfortify_edit.BoxLen);
          cancelfortify_edit.Text[cancelfortify_edit.BoxLen] = '\0';
          LocalCfg.EngineerPass[4] = '\0';
          printf("LocalCfg.EngineerPass = %s,cancelfortify_edit.Text = %s\n", LocalCfg.EngineerPass, cancelfortify_edit.Text);
          Special = atoi(LocalCfg.EngineerPass);
          Special ++;
          if(Special > 9999)
            Special = 0;
          sprintf(SpecialPass, "%04d\0", Special);
          if((strcmp(cancelfortify_edit.Text, LocalCfg.EngineerPass) == 0)||(strcmp(cancelfortify_edit.Text, SpecialPass) == 0))
           {
              for(i=0; i<8; i++)
               for(j=0; j<10; j++)
                LocalCfg.DefenceInfo[i][3] = 0;
        //     ShowStatusText(0, 0 , 3, cBlack, 1, 1, "�����ɹ�", 0);
              SendCancelAlarmFunc(); //ȡ����������

              sprintf(wavFile, "%scancelalarm.wav\0", wavPath);
              StartPlayWav(wavFile, 0);

              //�ٳֱ���
              if(strcmp(cancelfortify_edit.Text, SpecialPass) == 0)
                SendAlarmFunc(0x01, 0);
              //ioctl(gpio_fd, IO_CLEAR, 4);
              
              //��ʱ����ʾ��Ϣ��־
              PicStatBuf.Type = Local.CurrentWindow;
              PicStatBuf.Time = 0;
              PicStatBuf.Flag = 1;
              //��������
              cancelfortify_edit.Text[0] = 0;
              cancelfortify_edit.BoxLen = 0;
              DisplayEdit(&cancelfortify_edit, 0);

              Local.CurrentWindow = 0;      
              DisplayMainWindow(0);
           }
          else
             {
              sprintf(wavFile, "%spasserror.wav\0", wavPath);
              StartPlayWav(wavFile, 0);

              //��������
              cancelfortify_edit.Text[0] = 0;
              cancelfortify_edit.BoxLen = 0;
              DisplayEdit(&cancelfortify_edit, 0);
             }
          break;
   }
   if(Local.CurrentWindow == 35)
     //�򿪹��
       WriteCursor(&cancelfortify_edit, 1, 1, 0);
}
//---------------------------------------------------------------------------
