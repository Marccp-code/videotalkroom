#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>       //sem_t

#define CommonH
#include "common.h"
extern char sPath[80];
extern char NullAddr[21];   //���ַ���

void DisplayWiringWindow(void);    //��ʾ�ҵ���ƴ��ڣ�һ����
void OperateWiringWindow(short wintype, int currwindow);    //�ҵ���ƴ��ڲ�����һ����

void DisplaySigWiringWindow(void);    //��ʾ�������ƴ��ڣ�������
void OperateSigWiringWindow(short wintype, int currwindow);    //�������ƴ��ڲ�����������

void DisplaySceneWiringWindow(void);    //��ʾ����ģʽ���ڣ�������
void OperateSceneWiringWindow(short wintype, int currwindow);    //����ģʽ���ڲ�����������
//---------------------------------------------------------------------------
void DisplayWiringWindow(void)    //��ʾ�ҵ���ƴ��ڣ�һ����
{
  int xTop;
  int i;
  char jpgfilename[80];
  char infonum[20];
  char str[3];
  xTop = 0;//36;

   strcpy(jpgfilename, sPath);
   strcat(jpgfilename,"wiring.jpg");
   DisplayJPG(0, 0, jpgfilename, 1, SCRWIDTH, SCRHEIGHT, 0);

  Local.PrevWindow = Local.CurrentWindow;
  Local.CurrentWindow = 2;

  for(i=0; i<2; i++)
    DisplayImageButton(&wiring_button[i], IMAGEUP, 0);
}
//---------------------------------------------------------------------------
void OperateWiringWindow(short wintype, int currwindow)    //�ҵ���ƴ��ڲ�����һ����
{
  int xLeft,yTop;
  int i, j;
  char jpgfilename[80];
  char str[3];
  xLeft = 120;
  yTop = 36;

  if(wintype < 2)
   {
    DisplayImageButton(&wiring_button[wintype], IMAGEDOWN, 0);
    usleep(DELAYTIME*1000);
    if(Local.CurrentWindow != currwindow)
      return;
    DisplayImageButton(&wiring_button[wintype], IMAGEUP, 0);
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
    case 0:  //��������
           DisplaySigWiringWindow();
           break;
    case 1:  //����ģʽ
           DisplaySceneWiringWindow();
           break;
    case 30:  //��ҳ
    case 31:  //����
    //case 32:  //�ҵ�
    case 33:  //�Խ�
    case 34:  //��Ϣ
    case 35:  //����
           if(Local.Status == 0)
             DisplayMainWindow(wintype - 30);
           break;
   }
}
//---------------------------------------------------------------------------
void DisplaySigWiringWindow(void)    //��ʾ�������ƴ��ڣ�������
{
  int xTop;
  int i;
  char jpgfilename[80];
  char infonum[20];
  char str[3];
  xTop = 0;//36;

  strcpy(jpgfilename, sPath);
  strcat(jpgfilename,"single.jpg");
  DisplayJPG(0, 0, jpgfilename, 1, SCRWIDTH, SCRHEIGHT, 0);

  Local.PrevWindow = Local.CurrentWindow;
  Local.CurrentWindow = 91;

  for(i=0; i<4; i++)
    DisplayImage(&sigwiring_image[i], 0);

  //���ص�
  wiringcrtl_button[0].xLeft = sigwiring_image[0].xLeft - 25;
  wiringcrtl_button[0].yTop = sigwiring_image[0].yTop + 124;
  sigwiring_button[0] = wiringcrtl_button[0];

  wiringcrtl_button[1].xLeft = sigwiring_image[0].xLeft + 64;
  wiringcrtl_button[1].yTop = sigwiring_image[0].yTop + 124;
  sigwiring_button[1] = wiringcrtl_button[1];

  //�����
  wiringcrtl_button[3].xLeft = sigwiring_image[1].xLeft - 25;
  wiringcrtl_button[3].yTop = sigwiring_image[1].yTop + 124;
  sigwiring_button[2] = wiringcrtl_button[3];
  wiringcrtl_button[4].xLeft = sigwiring_image[1].xLeft + 64;
  wiringcrtl_button[4].yTop = sigwiring_image[1].yTop + 124;
  sigwiring_button[3] = wiringcrtl_button[4];
  //����
  wiringcrtl_button[0].xLeft = sigwiring_image[2].xLeft - 43;
  wiringcrtl_button[0].yTop = sigwiring_image[2].yTop + 124;
  sigwiring_button[4] = wiringcrtl_button[0];
  wiringcrtl_button[1].xLeft = sigwiring_image[2].xLeft + 77;
  wiringcrtl_button[1].yTop = sigwiring_image[2].yTop + 124;
  sigwiring_button[5] = wiringcrtl_button[1];
  wiringcrtl_button[2].xLeft = sigwiring_image[2].xLeft + 19;
  wiringcrtl_button[2].yTop = sigwiring_image[2].yTop + 124;
  sigwiring_button[6] = wiringcrtl_button[2];
  //�յ�
  wiringcrtl_button[0].xLeft = sigwiring_image[3].xLeft - 25;
  wiringcrtl_button[0].yTop = sigwiring_image[3].yTop + 124;
  sigwiring_button[7] = wiringcrtl_button[0];
  wiringcrtl_button[1].xLeft = sigwiring_image[3].xLeft + 64;
  wiringcrtl_button[1].yTop = sigwiring_image[3].yTop + 124;
  sigwiring_button[8] = wiringcrtl_button[1];

  for(i=0; i<9; i++)
    DisplayImageButton(&sigwiring_button[i], IMAGEUP, 0);
}
//---------------------------------------------------------------------------
void OperateSigWiringWindow(short wintype, int currwindow)    //�������ƴ��ڲ�����������
{
  int i, j;
  if(wintype < 9)
   {
    DisplayImageButton(&sigwiring_button[wintype], IMAGEDOWN, 0);
    usleep(DELAYTIME*1000);
    if(Local.CurrentWindow != currwindow)
      return;
    DisplayImageButton(&sigwiring_button[wintype], IMAGEUP, 0);
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
    //���ص�
    case 0:  //��
           //norder ָ��    TerType �豸����     TerNo �豸���    InfoType  �豸��Ϣ���    Info   �豸��Ϣ
           SendHostOrder(0x01, 0x01, 0x01, 0x01, 0x01);
           break;
    case 1:  //��
           //norder ָ��    TerType �豸����     TerNo �豸���    InfoType  �豸��Ϣ���    Info   �豸��Ϣ
           SendHostOrder(0x02, 0x01, 0x01, 0x01, 0x02);
           break;
    //�����
    case 2:  //��
           //norder ָ��    TerType �豸����     TerNo �豸���    InfoType  �豸��Ϣ���    Info   �豸��Ϣ
           SendHostOrder(0x04, 0x02, 0x01, 0x02, 10);
           break;
    case 3:  //��
           //norder ָ��    TerType �豸����     TerNo �豸���    InfoType  �豸��Ϣ���    Info   �豸��Ϣ
           SendHostOrder(0x05, 0x02, 0x01, 0x02, 10);
           break;
    //����
    case 4:  //��
           //norder ָ��    TerType �豸����     TerNo �豸���    InfoType  �豸��Ϣ���    Info   �豸��Ϣ
           SendHostOrder(0x01, 0x03, 0x01, 0x01, 0x01);
           break;
    case 5:  //ͣ
           //norder ָ��    TerType �豸����     TerNo �豸���    InfoType  �豸��Ϣ���    Info   �豸��Ϣ
           SendHostOrder(0x01, 0x03, 0x01, 0x01, 0x01);
           break;
    case 6:  //��
           //norder ָ��    TerType �豸����     TerNo �豸���    InfoType  �豸��Ϣ���    Info   �豸��Ϣ
           SendHostOrder(0x02, 0x03, 0x01, 0x01, 0x02);
           break;
    //�յ�
    case 7:  //��
           //norder ָ��    TerType �豸����     TerNo �豸���    InfoType  �豸��Ϣ���    Info   �豸��Ϣ
           SendHostOrder(0x01, 0x04, 0x01, 0x01, 0x01);
           break;
    case 8:  //��
           //norder ָ��    TerType �豸����     TerNo �豸���    InfoType  �豸��Ϣ���    Info   �豸��Ϣ
           SendHostOrder(0x02, 0x04, 0x01, 0x01, 0x02);
           break;
    case 30:  //��ҳ
    case 31:  //����
    case 32:  //�ҵ�
    case 33:  //�Խ�
    case 34:  //��Ϣ
    case 35:  //����
           if(Local.Status == 0)
             DisplayMainWindow(wintype - 30);
           break;
   }
}
//---------------------------------------------------------------------------
void DisplaySceneWiringWindow(void)    //��ʾ����ģʽ���ڣ�������
{
  int xTop;
  int i;
  char jpgfilename[80];
  char infonum[20];
  char str[3];
  xTop = 0;//36;

   strcpy(jpgfilename, sPath);
   strcat(jpgfilename,"scene.jpg");
   DisplayJPG(0, 0, jpgfilename, 1, SCRWIDTH, SCRHEIGHT, 0);

  Local.PrevWindow = Local.CurrentWindow;
  Local.CurrentWindow = 92;
}
//---------------------------------------------------------------------------
void OperateSceneWiringWindow(short wintype, int currwindow)    //����ģʽ���ڲ�����������
{
  int i, j;
  if(wintype < 5)
   {
    DisplayImageButton(&scenewiring_button[wintype], IMAGEDOWN, 0);
    usleep(DELAYTIME*1000);
    if(Local.CurrentWindow != currwindow)
      return;
    DisplayImageButton(&scenewiring_button[wintype], IMAGEUP, 0);
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
    case 0:  //���ģʽ
    case 1:  //�Ͳ�ģʽ
    case 2:  //����ģʽ
    case 3:  //ȫ��ģʽ
    case 4:  //ȫ��ģʽ
           //norder ָ��    TerType �豸����     TerNo �豸���    InfoType  �豸��Ϣ���    Info   �豸��Ϣ
           SendHostOrder(0x10, 0x00, wintype + 1, 0x00, 0x01);
           break;
    case 30:  //��ҳ
    case 31:  //����
    case 32:  //�ҵ�
    case 33:  //�Խ�
    case 34:  //��Ϣ
    case 35:  //����
           if(Local.Status == 0)
             DisplayMainWindow(wintype - 30);
           break;
   }

}
//---------------------------------------------------------------------------

