#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>       //sem_t
#include <dirent.h>

#define CommonH
#include "common.h"

//���̺ʹ�����������
void KeyAndTouchFunc(int pressed);

extern int ShowCursor; //�Ƿ���ʾ���
//������ʾ����
void DisplayBackground(void);           //������
void DisplayBigMenuWindow(void);    //��ʾ��˵�����
void OperateBigMenuWindow(short wintype);    //��˵����ڲ���
void DisplayMainWindow(short wintype);  //����
void DisplayMenuBar(void);       //�˵���
void DisplayStateBar(void);      //״̬��

void DisplayFirstWindow(void);    //��ʾ�����ڣ�һ����
void OperateFirstWindow(short wintype, int currwindow);    //�����ڲ�����һ����

void RefreshInfoTipWindow(void);    //ˢ����Ϣ��ʾ���ڣ�һ����
void OperateInfoTipWindow(short wintype);    //��Ϣ��ʾ���ڲ���

void DisplayCommWindow(void);    //��ʾ��ҵ���ڣ�һ����
void OperateCommWindow(short wintype);    //��ҵ���ڲ�����һ����


//��ǰ����  0 -- ��������  1 -- ��������  2 -- ��Ϣ����  3 -- �Խ�����   4 -- ���ڽ���  5 -- ���ý���  


//          11 -- ��������
//          12 -- ����ͨ��
//          13 -- ����
//          16 -- �Խ�ͼ�񴰿�
//          21 -- ������Ƭ�б���
//          22 -- ������Ƭ��ʾ���ڵ���
//          23 -- ͨ����Ƭ�б���
//          24 -- ͨ����Ƭ��ʾ���ڵ���

//          31 -- ����
//          32 -- ����
//          34 -- ��������
//          35 -- ȡ����������

//          71 -- ��ͨ��Ϣ
//          75 -- ȫ��ɾ����Ϣ��ʾ����
//          76 -- ����ɾ����Ϣ��ʾ����
//          77 -- ��Ϣ������ʾ����

//          151 -- �������ô���
// 	    156 -- ���Ĺ�������
// 	    159 -- ��������
//          160 -- ������ַ����
// 	    161 -- IP��ַ����
// 	    162 -- ������������
// 	    163 -- ��������
// 	    164 -- ����������
// 	    165 -- ϵͳ��Ϣ

//          190 -- У׼����������
//          191 -- У׼��������ʾ����

void ButtonFirstPageInit(void);    //��ť��ʼ��,Ϊ�ӿ��ٶȣ��ȳ�ʼ����ҳ��ʾ��Ҫ�İ�ť
void MainPageInit(void);    //��ҳ��ʼ��
void ButtonInit(void);    //��ť��ʼ��
void ButtonUnInit(void);    //��ť�ͷ�
void GifInit(void);    //GifͼƬ��ʼ��
void GifUnInit(void);    //GifͼƬ�ͷ�
void EditInit(void);    //�ı����ʼ��
void EditUnInit(void);    //�ı����ͷ�
void ImageInit(void);    //Image��ʼ��
void ImageUnInit(void);    //Image�ͷ�
void PopupWinInit(void);    //PopupWin��ʼ��
void PopupWinUnInit(void);    //PopupWin�ͷ�

void BuffUninit(void);   //Label��״̬���ͷ�


extern unsigned int    screensize;
extern unsigned char  *fbmem;

void ShowLabel(struct TLabel *t_label, int refreshflag);
//��ʾClock
void ShowClock(struct TLabel *t_label, int refreshflag);
//��ʾ����Ԥ��
void ShowWeather(void);
//��ʾ״̬��ʾ����
void ShowStatusText(int x,int y,int wd,int clr,int mx,int my,char s[128],int pass);

//����Ϣ�ṹ
extern struct Info1 Info[INFOTYPENUM];
//---------------------------------------------------------------------------
void DisplayStateBar(void)
{
	//����״̬
	if((LocalCfg.DefenceStatus == 1)||(LocalCfg.DefenceStatus == 2))
		DisplayImageButton(&state_image[1], IMAGEUP,0);
	else
		DisplayImageButton(&state_image[1],IMAGEDOWN, 0);

	printf("Local.NetStatus == %d\n",Local.NetStatus);
	if(Local.NetStatus == 0)   //����״̬ 0 �Ͽ�  1 ��ͨ
	{
		DisplayImageButton(&state_image[4],IMAGEDOWN, 0);
	}
	else
	{
		DisplayImageButton(&state_image[4],IMAGEUP, 0);
	}
	//miss call num
	if(LocalCfg.misscall_num >= 10)
		DisplayImage(&misscall_image[9],0);
	else if(LocalCfg.misscall_num > 0)
		DisplayImage(&misscall_image[LocalCfg.misscall_num-1],0);
	else
		DisplayImage(&misscall_image[10],0);

	if(Info[0].NoReadedNum >= 10)
		DisplayImage(&missmessage_image[9],0);
	else if(Info[0].NoReadedNum > 0)
		DisplayImage(&missmessage_image[Info[0].NoReadedNum -1],0);
	else if((Info[0].TotalNum == 0)||(Info[0].NoReadedNum== 0))
		DisplayImage(&missmessage_image[10],0);

	if(LocalCfg.news_num >= 10)
		DisplayImage(&news_image[9],0);
	else if(LocalCfg.news_num > 0)
		DisplayImage(&news_image[LocalCfg.missmessage_num-1],0);
	else
		DisplayImage(&news_image[10],0);

	DisplayClock(); //��ʾʱ��
	ShowWeather();  //��ʾ����
}
//---------------------------------------------------------------------------
void DisplayMainWindow(short wintype)
{
	//�ȹرչ�� 
	if(ShowCursor == 1)
		ShowCursor = 0;
	switch(wintype)
	{
	case MainWindow: //������
		DisplayFirstWindow();
		break;
	case TalkMenuWindow:
		DisplayTalkWindow();
		break;
	case 4:
		//DisplaySetupMainWindow();
		//DisplayLanSetWindow();
		//DisplayWlanSetWindow();
		//DisplayRoomnoSetWindow();
		//DisplayScreenContrastWindow();
		//DisplayScreenSaveWindow();
		//DisplayScreenCalibrateWindow();
		//DisplayVoiceSetWindow();
		//DisplayTimeSetWindow();
		break;
	default:
		DisplayFirstWindow();
		break;

	}

}
//---------------------------------------------------------------------------
void DisplayFirstWindow(void)    //��ʾ�����ڣ�һ����
{
	Local.CurrentWindow = MainWindow;
	if(Local.CurrFbPage != 0)
	{
		Local.CurrFbPage = 0;
		fb_setpage(fbdev,Local.CurrFbPage);
	}
	DisplayImage(&main_image,0);
	Local.ShowHotkey = 0;
	DisplayStateBar();
	/// hotkey menu store
	DisplayImage(&hotkey_image[6],0);
	if(label_hotkey.image == NULL)
	{
		label_hotkey.width = 371;
		label_hotkey.height = 80;
		label_hotkey.xLeft = 429;
		label_hotkey.yTop = 200;
		label_hotkey.image = (unsigned char*)malloc(371*80*3/2);
		SavePicBuf_Func(label_hotkey.xLeft,label_hotkey.yTop,371,80,label_hotkey.image,0);
	}
}
void DisplayHotKeyWindow(int wintype)
{
	printf("show hotkey windows\n");
	///Local.ShowHotkey = 0;
	switch(wintype)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		break;
	case 4:
		DisplaySetupMainWindow();
		break;
	}
}
//---------------------------------------------------------------------------
void OperateFirstWindow(short wintype, int currwindow)    //�����ڲ�����һ����
{
	int i;

	if(Local.CurrentWindow != 0)
		return;

	if(wintype < 5)
	{
		DisplayImageButton(&menu_button[wintype], IMAGEDOWN, 0);
		usleep(DELAYTIME*1000);
		DisplayImageButton(&menu_button[wintype], IMAGEUP, 0);
		DisplayMainWindow(wintype + 1);
	}
	else if((wintype >= 5)&&(wintype <11))
	{
		if(wintype >= 10)
		{
			RestorePicBuf_Func(label_hotkey.xLeft,label_hotkey.yTop,371,80,label_hotkey.image,0);
			Local.ShowHotkey = 0;
		}
		else
		{
			DisplayHotKeyWindow(wintype-5);
		}
		
	}
	else
	{
		if(wintype == 11)
		{
			Local.ShowHotkey = 1;
			printf("show hot key!\n");
			DisplayImage(&main_hotkey_image,0);
		}
		else if((wintype >= 12)&&(wintype<15))
		{
			printf("display misscall and others window!\n");
			//DisplayMissWindow();
		}
	}
}
//---------------------------------------------------------------------------
//���̺ʹ�����������
void KeyAndTouchFunc(int pressed)
{
	char wavFile[80];
	if(CheckTouchDelayTime() == 0)  //����������ʱ����ӳ�
	{
		printf("touch is too quick XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
		return;
	}    
    if(Local.Status == 0)
    {
		sprintf(wavFile, "%ssound1.wav\0", wavPath);
		StartPlayWav(wavFile, 0);
    }
    switch(pressed)
    {
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g':
        case 'h':
        case 'i':
        case 'j':
        case 'k':        
        case 'l':
        case 'm':
        case 'n':
        case 'o':
        case 'p':  //14
        case 'q':
        case 'r':
        case 's':
        case 't':
        case 'u':
        case 'v':
        case 'w':
        case 'x':
        case 'y':
        case 'z':  //���������µİ�ť
        case 'a' + 30:
        case 'a' + 31:
        case 'a' + 32:
        case 'a' + 33:
        case 'a' + 34:
        case 'a' + 35:
		case 'a' + 36:
		case 'a' + 37:
		case 'a' + 38:
		case 'a' + 39:
		case 'a' + 40:
		case 'a' + 41:
		case 'a' + 42:
		case 'a' + 43:
		case 'a' + 44:
		case 'a' + 45:
		case 'a' + 46:
		case 'a' + 47:
		case 'a' + 48:
		case 'a' + 49:
		case 'a' + 50:
		case 'a' + 51:
		case 'a' + 52:
		case 'a' + 53:
		case 'a' + 54:
		case 'a' + 55:
            if((Local.CurrentWindow >= 0) &&(Local.CurrentWindow <=300))
            switch(Local.CurrentWindow)
            {
               	case MainWindow:  //�����������һ����
                     OperateFirstWindow(pressed - 'a', Local.CurrentWindow);
                     break;
			   	case TalkMenuWindow:
			   		OperateTalkWindow( pressed-'a',Local.CurrentWindow);
			   		break;
				case TalkAreaMessageWindow:
					OperateAreaMessageWindow(pressed-'a',Local.CurrentWindow);
					break;
				case TalkInfoContentWindow:
					OperateInfoContent(pressed-'a',Local.CurrentWindow);
					break;
				case TalkPicWindow:
					OperateTalkPicWindow( pressed -'a',Local.CurrentWindow);
					break;
				case SetupMainWindow:
					OperateSetupMainWindow(pressed-'a',Local.CurrentWindow);
					break;
				case TalkLocalMessageWindow:
					OperateLocalMessageWindow(pressed-'a',Local.CurrentWindow);
					break;
				case TalkPushMessageWindow:
					OperatePushMessageWindow(pressed-'a',Local.CurrentWindow);
					break;
				case TalkCustomMessageWindow:
					OperateCustomMessageWindow(pressed-'a',Local.CurrentWindow);
					break;
				case TalkMisscallWindow:
					OperateMisscallWindow(pressed-'a',Local.CurrentWindow);
					break;
				case TalkCalledWindow:
					OperateCalledWindow(pressed-'a',Local.CurrentWindow);
					break;
				case TalkCallWindow:
					OperateCallWindow(pressed-'a',Local.CurrentWindow);
					break;
				case TalkPhonebookWindow:
					break;
				case TalkBlacklistWindow:
					break;

				case LanSetWindow:
					OperateLanSetWindow(pressed-'a',Local.CurrentWindow);
					break;
				case WlanSetWindow:
					OperateWlanSetWindow(pressed-'a',Local.CurrentWindow);
					break;
				case RoomSetWindow:
					OperateRoomnoSetWindow(pressed-'a',Local.CurrentWindow);
					break;
				case ScreenContrastWindow:
					OperateScreenContrastWindow(pressed-'a',Local.CurrentWindow);
					break;
				case ScreenSavingWindow:
					OperateScreenSaveWindow(pressed-'a',Local.CurrentWindow);
					break;
				case ScreenCalibrateWindow:
					OperateScreenCalibrateWindow(pressed-'a',Local.CurrentWindow);
					break;
				case VoiceSetWindow:
					OperateVoiceSetWindow(pressed-'a',Local.CurrentWindow);
					break;
				case TimeSetWindow:
					OperateTimeSetWindow(pressed-'a',Local.CurrentWindow);
					break;
				case PassSetWindow:
					OperatePasswordSetWindow(pressed-'a',Local.CurrentWindow);
					break;
				case LangSetWindow:
					OperateLangSetWindow(pressed-'a',Local.CurrentWindow);
					break;
				case UpgradeSDWindow:
					OperateUpgradeSDWindow(pressed-'a',Local.CurrentWindow);
					break;
				case UpgradeRemoteWindow:
					OperateUpgradeRemoteWindow(pressed-'a',Local.CurrentWindow);
					break;
				case SystemInfoWindow:
					OperateSysteminfoWindow(pressed-'a',Local.CurrentWindow);
					break;
              }
            break;
     }
}
// main page init
void MainPageInit(void)
{
	int i, j;
	DIR *dirp;
	char jpgfilename[80];
	char str[3];
	int hotkey_image_x[7] = {492,554,613,674,734,429,741};
	int menu_button_x[5] = {38,188,337,486,635};
	int state_image_x[5] = {566,610,654,698,742};
	int bigmenu_x[6] = {5,451,518,586,654,723};
	int bigmenu_y[6] = {16,524,524,524,524,524};
	int keyboard_num_x[32] = {17,55,93,131,169,207,245,283,321,359,36,74,112,150,188,226,264,302,340,74,112,150,188,226,264,302,20,340,20,115,210,305};
	int keyboard_num_y[32] = {13,13,13,13,13,13,13,13,13,13,69,69,69,69,69,69,69,69,69,125,125,125,125,125,125,125,125,125,181,181,181,181};

	if((dirp=opendir("/usr/pic")) == NULL)
	{
		strcpy(currpath,  sPath);
	}
	if(dirp != NULL)
	{
		strcpy(currpath,  "/usr/pic/");
		closedir(dirp);
		dirp = NULL;
	}

	//has no hotkey
	sprintf(jpgfilename,"%smain_1.jpg\0",currpath);
	ImageLoadFromFile(jpgfilename,&main_image);
	main_image.xLeft = 0;
	main_image.yTop = 0;



   //hotkey main
	sprintf(jpgfilename,"%smain_4.jpg\0",currpath);
   	ImageLoadFromFile(jpgfilename,&main_hotkey_image);
  	main_hotkey_image.xLeft = 429;
   	main_hotkey_image.yTop = 200;

	//hotkey
	for(i=0;i<7;i++)
	{
		sprintf(jpgfilename,"%shotkeys_%d.jpg\0",currpath,i+1);
   		ImageLoadFromFile(jpgfilename,&hotkey_image[i]);
   		hotkey_image[i].xLeft = hotkey_image_x[i];
   		hotkey_image[i].yTop = 200;
	}
	//menu 
	for(i=0;i<5;i++)
	{
		sprintf(jpgfilename,"%smenu%d_down.jpg\0",currpath,i+1);
		ImageButtonLoadFromFile(jpgfilename,&menu_button[i],IMAGEDOWN);
		sprintf(jpgfilename,"%smenu%d_up.jpg\0",currpath,i+1);
		ImageButtonLoadFromFile(jpgfilename,&menu_button[i],IMAGEUP);
		menu_button[i].xLeft= menu_button_x[i] ;
		menu_button[i].yTop = 341 ;
	}

	for(i=0;i<5;i++)
	{
		sprintf(jpgfilename,"%sstate%d.jpg\0",currpath,2*i+1);
		ImageButtonLoadFromFile(jpgfilename,&state_image[i],IMAGEDOWN);
		sprintf(jpgfilename,"%sstate%d.jpg\0",currpath,2*(i+1));
		ImageButtonLoadFromFile(jpgfilename,&state_image[i],IMAGEUP);
		state_image[i].xLeft= state_image_x[i] ;
		state_image[i].yTop = 25 ;
	}

	//weather
	for(i=0;i<5;i++)
	{
		sprintf(jpgfilename,"%sweather%d.jpg\0",currpath,i+1);
		ImageLoadFromFile(jpgfilename,&weather_image[i]);
		weather_image[i].xLeft = 26 ;
		weather_image[i].yTop = 182 ;
	}

	//misscall missmessage news bar
	for(i=0;i<11;i++)
	{
		sprintf(jpgfilename,"%smisscall_%d.jpg",currpath,i+1);
		ImageLoadFromFile(jpgfilename,&misscall_image[i]);
		misscall_image[i].xLeft = 252;
		misscall_image[i].yTop = 151;

		sprintf(jpgfilename,"%smissmessage_%d.jpg",currpath,i+1);
		ImageLoadFromFile(jpgfilename,&missmessage_image[i]);
		missmessage_image[i].xLeft = 252;
		missmessage_image[i].yTop = 190 ;

		sprintf(jpgfilename,"%snews_%d.jpg",currpath,i+1);
		ImageLoadFromFile(jpgfilename,&news_image[i]);
		news_image[i].xLeft = 252 ;
		news_image[i].yTop = 229;
	}

	//clock num!
	for(i=0;i<10;i++)
	{
		sprintf(jpgfilename,"%snum_%d.jpg\0",currpath,i);
		ImageLoadFromFile(jpgfilename,&clock_num_image[i]);
	}

	for(i=0;i<6;i++)
	{
		sprintf(jpgfilename,"%sbigmenu%d_down.jpg\0",currpath,i+1);
		ImageButtonLoadFromFile(jpgfilename,&bigmenu_button[i],IMAGEDOWN);
		sprintf(jpgfilename,"%sbigmenu%d_up.jpg\0",currpath,i+1);
		ImageButtonLoadFromFile(jpgfilename,&bigmenu_button[i],IMAGEUP);
		bigmenu_button[i].xLeft = bigmenu_x[i];
		bigmenu_button[i].yTop = bigmenu_y[i];
	}
		

	for(i=0;i<2;i++)
	{
		sprintf(jpgfilename,"%skeyboard%d_up.jpg\0",currpath,i+1);
		ImageLoadFromFile(jpgfilename,&keypad_image[i]);
		keypad_image[i].xLeft = 0;
		keypad_image[i].yTop = 368;
	}
	///���ְ���
	for(i=0;i<32;i++)
	{
		sprintf(jpgfilename,"%skb1_numbtn%d_down.jpg\0",currpath,i+1);
		ImageButtonLoadFromFile(jpgfilename,&keyboard_num[i],IMAGEDOWN);
		sprintf(jpgfilename,"%skb1_numbtn%d_up.jpg\0",currpath,i+1);
		ImageButtonLoadFromFile(jpgfilename,&keyboard_num[i],IMAGEUP);
		keyboard_num[i].xLeft = keyboard_num_x[i];
		keyboard_num[i].yTop = keyboard_num_y[i]+368;
	}
}
//---------------------------------------------------------------------------
void ButtonInit(void)    //��ť��ʼ��
{
	
	
}
//---------------------------------------------------------------------------
void ButtonUnInit(void)    //��ť�ͷ�
{
	int i;

	for(i=0;i<5;i++)
	{
		FreeImageButton(&talk_menu_button[i]);
	}
	for(i=0;i<12;i++)
	{
		FreeImageButton(&talk_keynum_button[i]);
	}
	for(i=0;i<3;i++)
	{
		FreeImageButton(&talk_hotkey_button[i]);
	}
}
//---------------------------------------------------------------------------
void GifInit(void)    //GifͼƬ��ʼ��
{
  char jpgfilename[80];
}
//---------------------------------------------------------------------------
void GifUnInit(void)    //GifͼƬ�ͷ�
{
  int i;

}
//---------------------------------------------------------------------------
void EditInit(void)    //�ı����ʼ��
{
  int i;
  char jpgfilename[80];


	
}
//---------------------------------------------------------------------------
void EditUnInit(void)    //�ı����ͷ�
{
  int i;

	FreeEdit(&roomaddr_edit);
	if(roomaddr_edit.Cursor_H != NULL)
		free(roomaddr_edit.Cursor_H);
}
//---------------------------------------------------------------------------
void ImageInit(void)    //Image��ʼ��
{
  char jpgfilename[80];
  int i;





}
//---------------------------------------------------------------------------
void ImageUnInit(void)    //Image�ͷ�
{

}
//---------------------------------------------------------------------------
void PopupWinInit(void)    //PopupWin��ʼ��
{
  int i;

#if 0
  //��Ϣ���ݵ�������
  InfoWin.image[0] = infowin_image.image;
  InfoWin.width =  infowin_image.width;
  InfoWin.height =  infowin_image.height;
  InfoWin.image[1] = (unsigned char *)malloc(InfoWin.width*InfoWin.height*3/2);
  InfoWin.xLeft = 120;
  InfoWin.yTop = 111;
  InfoWin.Visible = 0;

  //��Ϣ���ݵ�������
  CancelFortifyWin.image[0] = CancelFortify_Image.image;
  CancelFortifyWin.width =  CancelFortify_Image.width;
  CancelFortifyWin.height =  CancelFortify_Image.height;
  CancelFortifyWin.image[1] = (unsigned char *)malloc(CancelFortifyWin.width*CancelFortifyWin.height*3/2);
  CancelFortifyWin.xLeft = 105;
  CancelFortifyWin.yTop = 115;
  CancelFortifyWin.Visible = 0;
#endif
}
//---------------------------------------------------------------------------
void PopupWinUnInit(void)    //PopupWin�ͷ�
{
//  if(InfoWin.image[1] != NULL)
//    free(InfoWin.image[1]);
}
//---------------------------------------------------------------------------

void ShowLabel(struct TLabel *t_label, int refreshflag)
{
  if(refreshflag == REFRESH)
     RestorePicBuf_Func(t_label->xLeft - 2, t_label->yTop - 2, t_label->width, t_label->height,
                        t_label->image, 0);
  if(refreshflag == HILIGHT)
   {
     printf("t_label->xLeft=%d,t_label->yTop=%d,t_label->width=%d,t_label->height=%d \n" ,
       t_label->xLeft, t_label->yTop, t_label->width, t_label->height);
     RestorePicBuf_Func(t_label->xLeft - 2, t_label->yTop -2, t_label->width, t_label->height,
                        t_label->image_h, 0);
   }
  outxy24(t_label->xLeft, t_label->yTop, 3, cWhite, 1, 1, t_label->Text, 0, 0);
}
//---------------------------------------------------------------------------
//��ʾClock
char OldClock[20]="####################";
void ShowClock(struct TLabel *t_label, int refreshflag)
{
  if(refreshflag == REFRESH)
     RestorePicBuf_Func(t_label->xLeft, t_label->yTop, t_label->width, t_label->height,
                        t_label->image, 0);
  outxy16(t_label->xLeft + 15, t_label->yTop + 2, 1, cBlack, 1, 1, t_label->Text, 0, 0);
}
//---------------------------------------------------------------------------
//��ʾ����Ԥ��
void ShowWeather(void)
{
	int i;

	if((Local.Weather[0] >= 0)&&(Local.Weather[0] <= 6))
		DisplayImage(&weather_image[Local.Weather[0]],0);
#if 0
	int i;
	int tindex;
	char tmptext[10];
	char weathertext[2][10];
	if((Local.Weather[0] >=0)&&(Local.Weather[0] <=6))
		DisplayImage(&weather_image[Local.Weather[0]], 0);

	if(Local.Weather[1] <= 127)
	sprintf(weathertext[0], "%02d\0", Local.Weather[1]);
	else
	sprintf(weathertext[0], "-%02d\0", 256 - Local.Weather[1]);
	if(Local.Weather[2] <= 127)
	sprintf(weathertext[1], "%02d\0", Local.Weather[2]);
	else
	sprintf(weathertext[1], "-%02d\0", 256 - Local.Weather[2]);
	sprintf(Label_Weather.Text, "%s/%s\0", weathertext[0], weathertext[1]);

	RestorePicBuf_Func(Label_Weather.xLeft, Label_Weather.yTop, Label_Weather.width, Label_Weather.height,
	Label_Weather.image, 0);
	outxy16(Label_Weather.xLeft + 1, Label_Weather.yTop + 2, 0, cBlack, 1, 1, Label_Weather.Text, 0, 0);
#endif

}
//---------------------------------------------------------------------------
//��ʾ״̬��ʾ����
void ShowStatusText(int x,int y,int wd,int clr,int mx,int my,char s[128],int pass)
{
	switch(Local.CurrentWindow)
	{
	case TalkPicWindow:
		strcpy(label_call.Text,s);
		ShowLabel(&label_call,REFRESH);
		break;
	}
#if 0
  //�жϵ�ǰ����
  switch(Local.CurrentWindow)
   {
    case 0: //�Խ�����   ��������
           strcpy(Label_CCenter.Text, s);
           ShowLabel(&Label_CCenter, REFRESH);
           break;
    case 31: //�������ڣ�������
           strcpy(cancelfortify_label.Text, s);
           ShowLabel(&cancelfortify_label, REFRESH);
           break;
    case 32: //�������ڣ�������
           strcpy(fortify_label.Text, s);
           ShowLabel(&fortify_label, REFRESH);
           break;
    case 5: //���ô��ڣ�һ����
           strcpy(Label_Setup1.Text, s);
           ShowLabel(&Label_Setup1, REFRESH);
           break;
    case 156: //�޸Ĺ������봰�ڣ�������
    case 192: //�޸�ʹ�������봰�ڣ��ļ���    
           strcpy(modi_engi_label.Text, s);
           ShowLabel(&modi_engi_label, REFRESH);
           break;
    case 159: //�������ô��ڣ�������
           strcpy(addr_label.Text, s);
           ShowLabel(&addr_label, REFRESH);
           break;
    case 161: //IP��ַ���ô��ڣ�������
           strcpy(ip_label.Text, s);
           ShowLabel(&ip_label, REFRESH);
           break;
    case 12: //�Խ�����   ����ͨ��
           strcpy(Label_R2R.Text, s);
           ShowLabel(&Label_R2R, REFRESH);
           break;
    case 13: //����
           strcpy(Label_Watch.Text, s);
           ShowLabel(&Label_Watch, REFRESH);
           break;
    case 16: //�Խ�����   ����
           strcpy(Label_Call.Text, s);
           ShowLabel(&Label_Call, REFRESH);
           break;
   }
//  outxy24(x, y , wd, clr, mx, my,s, pass);
#endif
}
//---------------------------------------------------------------------------
void BuffUninit(void)
{

}
//---------------------------------------------------------------------------
