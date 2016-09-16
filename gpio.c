#include <stdio.h>
#include   <time.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/ipc.h>

typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned char u8;

#include <net/if.h>
#include <linux/sockios.h>
#define ETHTOOL_GLINK		0x0000000a /* Get link status (ethtool_value) */
#define ETHTOOL_GSET		0x00000001 /* Get settings. */
#define ETHTOOL_SSET		0x00000002 /* Set settings, privileged. */
/* for passing single values */
struct ethtool_value {
	u32	cmd;
	u32	data;
};
struct ethtool_cmd {
	u32	cmd;
	u32	supported;	/* Features this interface supports */
	u32	advertising;	/* Features this interface advertises */
	u16	speed;		/* The forced speed, 10Mb, 100Mb, gigabit */
	u8	duplex;		/* Duplex, half or full */
	u8	port;		/* Which connector port */
	u8	phy_address;
	u8	transceiver;	/* Which tranceiver to use */
	u8	autoneg;	/* Enable or disable autonegotiation */
	u32	maxtxpkt;	/* Tx pkts before generating tx int */
	u32	maxrxpkt;	/* Rx pkts before generating rx int */
	u32	reserved[4];
};
#define CommonH
#include "common.h"

extern int vpbuf_n; // ��Ƶ���Ż��λ������е�Ԫ��������
extern int curr_video_timestamp;       //��ǰ�Ѳ�����Ƶ֡ʱ���
extern int curr_audio_timestamp;       //��ǰ�Ѳ�����Ƶ֡ʱ���
extern struct audiobuf1 recbuf;     //��Ƶ�ɼ����λ���
extern struct audiobuf1 playbuf;    //��Ƶ���Ż��λ���

//����ɨ��
unsigned char Kinput;  //��������
unsigned char Ktemp;   //�ϴ�����
unsigned char Kstore;
unsigned char Kready;  //ȷ������
int nokeynum = 0;      //û�а�������
int key1pressnum = 0;      //����1��������
void CheckGpio(void);    //���GPIO����
void get_key_value(int gpiovalue);

struct Defence1{
               int Count;  //������������
               int Flag;   //����������־
               }Defence[MAXCOUNT];
void CheckDefence(void);    //����������
void SendAlarmFunc(unsigned char SpecialByte, unsigned char AlarmByte); //������������
void SendCancelAlarmFunc(void); //ȡ����������

//�����ƵPlay Ring
void CheckPlayRing(void);
//��������ź�
void CheckEth(void);
int detect_ethtool(int skfd, char *ifname);
int get_ethtool_speed(int skfd, char *ifname);//��ȡ��������

//���IP��ַ��ͻ
void CheckIPConflict(void);

void OnlineCheckFunc(void); //����ȷ�ϼ�⺯��
void TimeReportStatusFunc(void); //�豸��ʱ����״̬����
void ClearTipFunc(void);    //��״̬��ʾ��Ϣ����
void CheckDefenceDelay_Func(void); //��鲼����ʱ����
void CheckDelayAlarm(void);  //�����ʱ��������
int CursorFlag;
int ShowCursor; //�Ƿ���ʾ���
void WriteCursor(struct TEdit *t_edit, int t_Flag, int t_show, int PageNo); //д��꺯��   t_Flag 0--���  1--д
void ShowCursorFunc(void);  //��ʾ�����꺯��
void TalkCtrlFunc(void);  //�Խ����ƣ���ʾͨ��ʱ����жϳ�ʱ
void TalkOsd(void);  //ͨ���ͼ���ʱOSD��ʾ

void DisplayClock(void); //��ʾʱ��

//���ػ����̲��ù����ڴ淽ʽͨ��
int shmid;
char *c_addr;
//---------------------------------------------------------------------------

void gpio_rcv_thread_func(void)
{
	char jpgfilename[80];

	int timenum;  //����
	int i;

	static int flag=0;


	key_t key;
	char* name = "/dev/shm";///myshm2";
	key = ftok(name,0);
	if(key==-1)
		perror("ftok error");
	//�����ڴ棬���ػ�����daemonͨ��
	if((shmid = shmget(key, 1024, IPC_CREAT)) == -1)
	{
		fprintf(stderr,"Create Share Memory Error:%s\n\a","strerror(errno)");
	}
	c_addr = shmat(shmid, 0, 0);

	nokeynum = 0;
	Kinput=Ktemp=Kready=Kstore=0;

	printf("����GPIO�����̣߳�\n" );

	timenum = 0;

	//���
	CursorFlag = 1;

	for(i=0; i<MAXCOUNT; i++)
	{
		Defence[i].Count = 0;
		Defence[i].Flag = 0;
	}
	while(gpio_rcv_flag)
	{
		//���GPIO
		//GPIO14�������ѡ�񰴼�ɨ�跽ʽ���͵�ƽ��Ч
		//GPIO15�������ѡ�����ɨ�����룬�͵�ƽ��Ч
#if 0		
		{
			ioctl(gpio_fd, IO_CLEAR, 14);
			ioctl(gpio_fd, IO_PUT, 15);
			CheckGpio();
		}
#endif
#if 0		
		{
			//������1����6  Gpio 8--13
			//GPIO14�������ѡ�񰴼�ɨ�跽ʽ���͵�ƽ��Ч
			//GPIO15�������ѡ�����ɨ�����룬�͵�ƽ��Ч
			ioctl(gpio_fd, IO_PUT, 14);
			ioctl(gpio_fd, IO_CLEAR, 15);
			CheckDefence();
		}
#endif
		//��Ϊ��ʼ״̬������λ���ɹ� 
		ioctl(gpio_fd, IO_PUT, 14);
		ioctl(gpio_fd, IO_PUT, 15);

		if((timenum % (INTRPERSEC*2))==0)
		{
			//�����ƵPlay Ring
			CheckPlayRing();
	    	//��������ź�
	    		CheckEth();
			
	   }

		if((timenum%INTRPERSEC) == 0)
		{
			RefreshLed();
		}
 		 //���IP��ַ��ͻ
  		CheckIPConflict();

		//����6��û���յ�����ȷ�ϣ�����Ϊ����
		 if(Local.OnlineFlag == 1)
		 {
		    	OnlineCheckFunc();
		 }

  		//��״̬��ʾ��Ϣ����
  		ClearTipFunc();


		//�豸��ʱ����״̬����
		if(LocalCfg.ReportTime != 0)
		{
			if(Local.ReportSend == 1)
			{
				if(Local.ReportTimeNum >= (LocalCfg.ReportTime*INTRPERSEC))
				{
					Local.RandReportTime = (int)(1.0*LocalCfg.ReportTime*rand()/(RAND_MAX+1.0)) + 1;
					Local.ReportSend = 0;
					Local.ReportTimeNum = 0;
					printf("LocalCfg.ReportTime = %d, Local.RandReportTime = %d,temp_video_n = %d,
						vpbuf_n = %d, TimeStamp.CurrVideo = %d,curr_video_timestamp = %d,curr_audio_timestamp = %d, 
						TimeStamp.CurrAudio = %d,temp_audio_n = %d, playbuf.n = %d, recbuf.n = %d\n",
					LocalCfg.ReportTime, Local.RandReportTime, temp_video_n, vpbuf_n, TimeStamp.CurrVideo,
					curr_video_timestamp, curr_audio_timestamp, TimeStamp.CurrAudio,temp_audio_n, playbuf.n, recbuf.n);
				}
			}
			if(Local.ReportSend == 0)
			{
				if(Local.ReportTimeNum >= (Local.RandReportTime*INTRPERSEC))
				{
					Local.ReportSend = 1;
					printf("send report!\n");
					TimeReportStatusFunc();
				}
			}
			Local.ReportTimeNum ++;
		}  

		//1S��ʾ��ǰʱ��
		if(Local.CurrentWindow == MainWindow)
		{
			if((timenum % (INTRPERSEC*60))==0)
			{
				DisplayClock();
			}
		}

   		//500ms��ʾ���
  	 	if((timenum % (INTRPERSEC/2))==0)
  		{
			#if 0
     		ShowCursorFunc();
			#endif
   	}

  	 	//CheckDefenceDelay_Func(); //��鲼����ʱ����
		#if 0
   	if(Local.MiniDoorOpenLockFlag == 1)    
    	{
	        Local.MiniDoorOpenLockTime ++;    
	        if(Local.MiniDoorOpenLockTime > 10)  
	        {
	           Local.MiniDoorOpenLockFlag = 0;    
	           Local.MiniDoorOpenLockTime = 0;    
	           ioctl(gpio_fd, IO_CLEAR, MINIDOOR_OPENLOCK_IO);    
	        }
    	}   
		#endif
   	//�����ʱ��������
   	//CheckDelayAlarm();

		if((timenum % (INTRPERSEC*10))==0)
		     //д�����ڴ浽�ػ�����
			c_addr[0] = 'R';

		   //clear watchdog
		if((timenum % (INTRPERSEC*30))==0)
		{
			if(Local.Save_File_Run_Flag != 1)
			{
				printf("save thread is dead!\n");
				system("reboot");
			}

			if(Local.Multi_Send_Run_Flag != 1)
			{
				printf("multi send thread is dead!\n");
				system("reboot");
			}

			if(Local.Multi_Comm_Send_Run_Flag != 1)
			{
				printf("multi comm thread is dead!\n");
				system("reboot");
			}       
			Local.Save_File_Run_Flag = 0;
			Local.Multi_Send_Run_Flag = 0;
			Local.Multi_Comm_Send_Run_Flag = 0;
			sem_post(&save_file_sem);
			sem_post(&multi_send_sem);
			sem_post(&multi_comm_send_sem);
		}

		timenum ++;
		if(timenum > 0xFFFFFF)
			timenum = 0;   
		usleep((INTRTIME-10)*1000);
	}
}
//---------------------------------------------------------------------------
void DisplayClock(void) //��ʾʱ��
{
	//ʱ��
	time_t t;
	int i;
	struct TImage clock_image_tmp[4];
	
	time(&t);
	curr_tm_t=localtime(&t);

	clock_image_tmp[0] = clock_num_image[curr_tm_t->tm_hour/10];
	clock_image_tmp[0].xLeft = 38;
	clock_image_tmp[0].yTop = 115;
	clock_image_tmp[1] = clock_num_image[curr_tm_t->tm_hour%10];
	clock_image_tmp[1].xLeft = 78;
	clock_image_tmp[1].yTop = 115;
	clock_image_tmp[2] = clock_num_image[curr_tm_t->tm_min/10];
	clock_image_tmp[2].xLeft = 122;
	clock_image_tmp[2].yTop = 115;
	clock_image_tmp[3] = clock_num_image[curr_tm_t->tm_min%10];
	clock_image_tmp[3].xLeft = 162;
	clock_image_tmp[3].yTop = 115;

	for(i=0;i<4;i++)
	{
		DisplayImage(&clock_image_tmp[i],0);
	}
}
//---------------------------------------------------------------------------
/*
//20081115  4������  ������  16��������   13��������    17��������    14����ͨ��
#define WATCH_KEY  16
#define CENTER_KEY  13
#define OPENLOCK_KEY  17
#define TALK_KEY  14
*/

//13 ����/����	14 �Խ�  15 ��������  16 ����  17 ����	18 ������У׼

#define WATCH_KEY     13  // ����/����
#define TALK_KEY      14  // ͨ�� o
#define CENTER_KEY    15  // ����
#define SETING_KEY    16  // ���� 
#define AlarmSet_KEY  17  // ���� o
#define TP_SET_KEY    18  // ������У׼

void CheckGpio(void)    //���GPIO����
{
  int nokeypressed;  //û�м�����
  unsigned int keyvalue;
  int    gpio_value,k, i;
  int tmpvalue;
  int tmpb;
  
     nokeypressed = 1;
       	//***** pin start to end *****
        //�а������£����밴��ɨ�����
     gpio_value = 0;

//     printf("gpio = 0x%X, ffff = 0x%X\n", read(gpio_fd, NULL, 0), (read(gpio_fd, NULL, 0) & 0x03F00));
     tmpvalue = read(gpio_fd, NULL, 0);
     if((tmpvalue & 0x03F00) != 0x03F00)// == 0)
      {
        for(k = 5; k <= 7; k++)
         {
            switch(k)
             {
              case 5:
                     ioctl(gpio_fd, IO_PUT, 5);
                     ioctl(gpio_fd, IO_CLEAR, 6);
                     ioctl(gpio_fd, IO_CLEAR, 7);
                     break;
              case 6:
                     ioctl(gpio_fd, IO_CLEAR, 5);
                     ioctl(gpio_fd, IO_PUT, 6);
                     ioctl(gpio_fd, IO_CLEAR, 7);
                     break;
              case 7:
                     ioctl(gpio_fd, IO_CLEAR, 5);
                     ioctl(gpio_fd, IO_CLEAR, 6);
                     ioctl(gpio_fd, IO_PUT, 7);
                     break;
             }
            if((read(gpio_fd, NULL, 0) & 0x03F00) == 0x03F00)
             {
              for(i=0; i<6; i++)
               {
                if((tmpvalue & 0x2000) == 0)
                  break;
                tmpvalue = tmpvalue << 1;
               }
              gpio_value = k - 4 + (5-i)*3;
              nokeypressed = 0;
              nokeynum = 0;
              get_key_value(gpio_value);
              break;
             }
        }
       //GPIO7-5 ��Ϊ��
       //ioctl(gpio_fd, IO_CLEAR, 5);
       ioctl(gpio_fd, IO_CLEAR, 6);
       ioctl(gpio_fd, IO_CLEAR, 7);
      }
	if((gpio_value == WATCH_KEY)||(gpio_value == TP_SET_KEY))	   //������
	key1pressnum ++;      //������������
	else
	key1pressnum = 0;

	//����3S����  
	if((key1pressnum > INTRPERSEC*3)&&(Local.CurrentWindow != 190)&&(Local.CurrentWindow != 191)&&(Local.Status == 0))
	{                                                        
	key1pressnum = 0;
	//DisplayCalibrateWindow();   //У׼������
	}

	if(nokeypressed == 1)
	{
		nokeynum ++;
		//���Σ�50*2ms���ж�Ϊ�������ͷ�
		if(nokeynum >=2)
		{
			nokeynum = 0;
			nokeypressed = 0;
			Kinput = 0;
			Ktemp = 0;
			Kready = 0;
		}
	}
}
//---------------------------------------------------------------------------
void get_key_value(int gpiovalue)
{
  int tmpvalue;
  int isAlarm;
  int i;
  char wavFile[80];
  char str[10];
  //��������
  Kinput = gpiovalue;
  if((Kinput == Ktemp)&&(Kinput != Kready))
   {
    Kready = Kinput;
    printf("keyvalue = %d \n", Kready);
    
    //30S�޲���, ���ص���������
    PicStatBuf.KeyPressTime = 0;
    Local.LcdLightTime = 0;
    if(Local.Status == 0)
     {
      sprintf(wavFile, "%ssound1.wav\0", wavPath);
      StartPlayWav(wavFile, 0);
     }
    if(Local.LcdLightFlag == 0)
     {
      //����LCD����
      OpenLCD();
      Local.LcdLightFlag = 1; //LCD�����־
      Local.LcdLightTime = 0; //ʱ��
     }
    else
     {
      tmpvalue = Kready;
      switch(tmpvalue)
       {
#if 0
		case 1:  //1
        case 2:  //2
        case 3:  //3
        case 4:  //4
        case 5:  //5
        case 6:  //6
        case 7:  //7
        case 8:  //8
        case 9:  //9
        case 10:  //*
        case 11:  //0
        case 12:  //#
               if(tmpvalue == 11)
                 tmpvalue = 0;
               if(tmpvalue == 12)
                 tmpvalue = 11;
               if(Local.Status == 0)
                switch(Local.CurrentWindow)
                 {
                  case 3:
                         OperateTalkWindow(tmpvalue);
                         break;
                  case 5:
                         OperateSetupWindow(tmpvalue);
                         break;
                  case 31:  //����
                         OperateCancelFortifyWindow(tmpvalue);
                         break;
                  case 35:  //ȡ������
                         OperateCancelAlarmWindow(tmpvalue);
                         break;
                  case 159:
                         OperateAddressWindow(tmpvalue);
                         break;
                  case 161:
                         OperateIPWindow(tmpvalue);
                         break;
                  case 156:
                         OperateModiPassWindow(tmpvalue);
                         break;
                 }
               break;
        case 19:  //����
                 switch(Local.CurrentWindow)
                  {
                   case 31: //��������
                   case 32: //��������
                   case 4://��Ϣ���ڣ�һ����
                   case 2: //�Ҿӿ��ƴ���
                   case 5://���ô��ڣ�һ����
                   case 151://�������ô��ڣ�������
                          Local.CurrentWindow = 0;
                          DisplayMainWindow(Local.CurrentWindow);
                          break;
                   case 3: //�Խ����ڣ�һ����
                   case 13: //���Ӵ��ڣ�������
                   case 16: //�Խ�ͼ��������ڣ�������
                          if(Local.Status == 0)
                           {
                             isAlarm = 0;
                             for(i=0; i<6; i++)
                              if(LocalCfg.DefenceInfo[i][3] != 0)
                               {
                                isAlarm = 1;
                                break;
                               }
                            if(isAlarm == 0)
                             {
                              Local.CurrentWindow = 0;
                              DisplayMainWindow(Local.CurrentWindow);
                             }
                            else
                              DisplayAlarmWindow();
                           }
                          else
                           {
                            #ifdef _DEBUG
                              printf("������æ,�޷�����\n");
                            #endif
                           }
                          break;
                   case 21: //��Ƭ
                          OperatePictureWindow(3);
                          break;
                   case 22: //��Ƭ����
                          OperatePicContent(3);
                          break;
                   case 77:  //��Ϣ����
                          OperateInfoContent(3);
                          break;
                   case 159:
                   case 161:
                          OperateAddressWindow(12);
                          break;
                   case 156:
                          OperateModiPassWindow(12);
                          break;
                  }

             break;
			 
			 case CENTER_KEY:  //����
					//���ڱ�����ȡ������������
					if((Local.CurrentWindow != 34)&&(Local.CurrentWindow != 35))
					 {
						if((Local.Status == 0)&&(Local.CurrentWindow != 13)&&(Local.CurrentWindow != 16))
						 {
						  DisplayTalkPicWindow();
						  CallCenter_Func();
						 }
						if((Local.CurrentWindow == 16)&&(Local.Status == 1))
						  OperateTalkPicWindow(2, 16);
					 }
					break;
					
			 case SETING_KEY:  //����	 
					 if((Local.Status == 0)&&(Local.CurrentWindow != 5)) 
					 {
						  DisplaySetupWindow(); 
					 }
					 
					 break;
			 
			 case AlarmSet_KEY: //����
					 if((Local.Status == 0)&&(Local.CurrentWindow != 1)) 
					 {
						 DisplayMainWindow(1);	  
					 }
					 break;

        case TALK_KEY:     //ͨ��
               //���ڱ�����ȡ������������
               if((Local.CurrentWindow != 34)&&(Local.CurrentWindow != 35))
               {
                 if((Local.CurrentWindow == 16)&&(Local.Status != 0))  //ͨ��
                  {
                   switch(Local.Status)
                    {
                     case 2: //���жԽ�
                             OperateTalkPicWindow(0, 16);
                             break;
                     case 5: //ͨ��
                     case 6: //ͨ��
                             //OperateTalkPicWindow(2, 16); //ͨ���а��¿���
                             OperateTalkPicWindow(3, 16);   //�Ҷ�
                             break;
                    }
                   Local.RecordPic = 0;   //��������Ƭ
                   Local.IFrameCount = 0; //I֡����
                   Local.IFrameNo = 0;    //���ڼ���I֡
                  }
               }
               break;
#endif
			default:
				break;
       }
     }  
   }
  Ktemp=Kinput;
}
//---------------------------------------------------------------------------
/*
void CheckDefence(void)    //����������
{
  int    i;
  int tmpvalue;
  unsigned char AlarmByte;
  unsigned char tmpbyte;
  char wavFile[80];

     tmpvalue = read(gpio_fd, NULL, 0);

     if((tmpvalue & 0x03F00) != 0x3F00)
      {
       //     printf("k = %d  gpio = 0x%X, ffff = 0x%X, tmpvalue = 0x%X\n", k, read(gpio_fd, NULL, 0), (read(gpio_fd, NULL, 0) & 0x03F00), tmpvalue);
              for(i=0; i<6; i++)
               {
         //       printf("i = %d, (tmpvalue & 0x2000) = 0x%X \n", i, (tmpvalue & 0x2000));
                if((tmpvalue & 0x0100) == 0)
                  Defence[i].Count ++;
                else
                 {
                  Defence[i].Count = 0;
                  Defence[i].Flag = 0;
                 }
                tmpvalue = tmpvalue >> 1;
               }
      }
     else
       for(i=0; i<MAXCOUNT; i++)
        {
         Defence[i].Count = 0;
         Defence[i].Flag = 0;
        }
   AlarmByte = 0x0;
   for(i=0; i<MAXCOUNT; i++)
     if((Defence[i].Count > 2)&&(Defence[i].Flag == 0))
      {
       Defence[i].Flag = 1;
       printf("Defnece i = %d\n", i + 1);
       tmpbyte = 0x01;
       switch(i+1)
        {
         case 1:   //������� ���� ��ʱ����
         case 2:
                if(LocalCfg.DefenceStatus == 1)
                  if(Local.AlarmDelayFlag[i] == 0)
                   {
                    Local.AlarmDelayFlag[i] = 1;     //������ʱ��־
                    Local.AlarmDelayTime[i] = 0;    //����
                   
                    sprintf(wavFile, "%salarmdelay.wav\0", wavPath);
                    StartPlayWav(wavFile, 1);
                   }
                break;
         case 3:   //�ڼҲ��� ���� ����ʱ
         case 4:
                if((LocalCfg.DefenceStatus == 1)||(LocalCfg.DefenceStatus == 2))
                 if(LocalCfg.DefenceInfo[i][3] != 2)
                  {
                   AlarmByte |= (tmpbyte << i);
                   LocalCfg.DefenceInfo[i][3] = 1;  //�б���
                  }
                break;
         case 5:   //24Сʱ����
                 if(LocalCfg.DefenceInfo[i][3] != 2)
                  {
                   AlarmByte |= (tmpbyte << i);
                   LocalCfg.DefenceInfo[i][3] = 1;  //�б���
                  } 
                break;
         case 6:  
                if((Local.Status == 21)||(Local.Status == 22))
                  MiniDoorTalkEnd_Func(); 
                else
                  MiniDoorCall();
                break;                                 
        }
      }
  if(AlarmByte != 0x0)
   {
    ioctl(gpio_fd, IO_PUT, 4);
    printf("AlarmByte = 0x%X\n", AlarmByte);
    SendAlarmFunc(0, AlarmByte);
    DisplayAlarmWindow();
   }
}
*/
//---------------------------------------------------------------------------
/*
void CheckDefenceDelay_Func(void) //��鲼����ʱ����
{
  char wavFile[80];
   if(Local.DefenceDelayFlag == 1)    //������ʱ��־
    {
     if((Local.DefenceDelayTime % INTRPERSEC) == 0)
      {
       if((Local.DefenceDelayTime % (INTRPERSEC*2)) == 0)
         ioctl(gpio_fd, IO_PUT, 2);
       else
         ioctl(gpio_fd, IO_CLEAR, 2);
      }

     Local.DefenceDelayTime ++;    //����
     if(Local.DefenceDelayTime > (LocalCfg.Out_DelayTime*INTRPERSEC))    //����
      {
       ioctl(gpio_fd, IO_PUT, 2);
       sprintf(wavFile, "%sfortify.wav\0", wavPath);
       StartPlayWav(wavFile, 0);
       switch(LocalCfg.DefenceStatus)
        {
         case 4://���������ʱ��
                LocalCfg.DefenceInfo[0][2] = 1;  // ����
                LocalCfg.DefenceInfo[0][3] = 0;  // �ޱ���
                LocalCfg.DefenceInfo[1][2] = 1;  // ����
                LocalCfg.DefenceInfo[1][3] = 0;  // �ޱ���
                LocalCfg.DefenceInfo[2][2] = 0;  // ����
                LocalCfg.DefenceInfo[2][3] = 0;  // �ޱ���
                LocalCfg.DefenceInfo[3][2] = 0;  // ����
                LocalCfg.DefenceInfo[3][3] = 0;  // �ޱ���
         case 5://�ڼҲ�����ʱ��
                LocalCfg.DefenceInfo[0][2] = 0;  // ����
                LocalCfg.DefenceInfo[0][3] = 0;  // �ޱ���
                LocalCfg.DefenceInfo[1][2] = 0;  // ����
                LocalCfg.DefenceInfo[1][3] = 0;  // �ޱ���
                LocalCfg.DefenceInfo[2][2] = 1;  // ����
                LocalCfg.DefenceInfo[2][3] = 0;  // �ޱ���
                LocalCfg.DefenceInfo[3][2] = 1;  // ����
                LocalCfg.DefenceInfo[3][3] = 0;  // �ޱ���

                LocalCfg.DefenceStatus = LocalCfg.DefenceStatus - 3;
                Local.DefenceDelayFlag = 0;
                Local.DefenceDelayTime = 0;
                //����״̬
                if(LocalCfg.DefenceStatus == 0)
                 {
                  if(Local.CurrentWindow == 0)
                    DisplayImage(&state_image[1], 0);
                 }
                else
                 {
                  if(Local.CurrentWindow == 0)
                    DisplayImage(&state_image[0], 0);
                 }
                  
                if(Local.CurrentWindow == 32)
                  DisplayCancelFortifyWindow();
                break;
        }
      }
    }
}
//---------------------------------------------------------------------------
void CheckDelayAlarm(void)  //�����ʱ��������
{
  int i;
  unsigned char AlarmByte;
  unsigned char tmpbyte;
  AlarmByte = 0x0;
  tmpbyte = 0x01;
   if(LocalCfg.DefenceStatus == 1)
    {
     for(i=0; i<2; i++)
      if(Local.AlarmDelayFlag[i] == 1)
       {
        if((Local.AlarmDelayTime[i] % INTRPERSEC) == 0)
         {
          if((Local.AlarmDelayTime[i] % (INTRPERSEC*2)) == 0)
           {
            //ioctl(gpio_fd, IO_PUT, 4);
           }
          else
           {
            //ioctl(gpio_fd, IO_CLEAR, 4);
           }
         }

        Local.AlarmDelayTime[i] ++;    //����
        if(Local.AlarmDelayTime[i] > (LocalCfg.Alarm_DelayTime*INTRPERSEC))    //����
         {
          StopPlayWavFile();  //�رձ�����ʱ��ʾ��
          usleep(200*1000);  //��ʱ��Ϊ�˵ȴ�����������ɣ������������

          Local.AlarmDelayFlag[i] = 0;     //������ʱ��־
          Local.AlarmDelayTime[i] = 0;
          if(LocalCfg.DefenceInfo[i][3] != 2)
           {
            AlarmByte |= (tmpbyte << i);
            LocalCfg.DefenceInfo[i][3] = 1;  //�б���
           } 
         }
       }
     if(AlarmByte != 0x0)
      {
       ioctl(gpio_fd, IO_PUT, 4);
       printf("AlarmByte = 0x%X\n", AlarmByte);
       SendAlarmFunc(0, AlarmByte);
       DisplayAlarmWindow();
      }
    }
}
//---------------------------------------------------------------------------
void SendAlarmFunc(unsigned char SpecialByte, unsigned char AlarmByte) //������������
{
  int i,j,k;
    //����������
    pthread_mutex_lock (&Local.udp_lock);
      //���ҿ��÷��ͻ��岢���
      for(i=0; i<UDPSENDMAX; i++)
       if(Multi_Udp_Buff[i].isValid == 0)
        {
         sprintf(Multi_Udp_Buff[i].RemoteHost, "%d.%d.%d.%d\0",LocalCfg.IP_Server[0],
                LocalCfg.IP_Server[1], LocalCfg.IP_Server[2], LocalCfg.IP_Server[3]);
         Multi_Udp_Buff[i].SendNum = 0;   //��෢6��
         Multi_Udp_Buff[i].m_Socket = m_DataSocket;
         Multi_Udp_Buff[i].CurrOrder = 0;
         //ͷ��
         memcpy(Multi_Udp_Buff[i].buf, UdpPackageHead, 6);
         //����
         Multi_Udp_Buff[i].buf[6] = ALARM;
         Multi_Udp_Buff[i].buf[7] = ASK;    //����

         memcpy(Multi_Udp_Buff[i].buf + 8, LocalCfg.Addr, 20);
         memcpy(Multi_Udp_Buff[i].buf + 28, LocalCfg.Mac_Addr, 6);
         Multi_Udp_Buff[i].buf[34] = LocalCfg.DefenceStatus;     //����״̬
         Multi_Udp_Buff[i].buf[35] = SpecialByte;                //���ⱨ��
         Multi_Udp_Buff[i].buf[36] = LocalCfg.DefenceNum;        //����ģ�����
         Multi_Udp_Buff[i].buf[37] = AlarmByte;
         Multi_Udp_Buff[i].buf[38] = 0x0;
         Multi_Udp_Buff[i].buf[39] = 0x0;
         Multi_Udp_Buff[i].buf[40] = 0x0;

         Multi_Udp_Buff[i].nlength = 41;
         Multi_Udp_Buff[i].DelayTime = 100;
         Multi_Udp_Buff[i].isValid = 1;
         sem_post(&multi_send_sem);
         break;
        }
     //�򿪻�����
     pthread_mutex_unlock (&Local.udp_lock);
}
//---------------------------------------------------------------------------
void SendCancelAlarmFunc(void) //ȡ����������
{
  int i,j,k;
  unsigned char AlarmBype;
    //����������
    pthread_mutex_lock (&Local.udp_lock);
      //���ҿ��÷��ͻ��岢���
      for(i=0; i<UDPSENDMAX; i++)
       if(Multi_Udp_Buff[i].isValid == 0)
        {
         sprintf(Multi_Udp_Buff[i].RemoteHost, "%d.%d.%d.%d\0",LocalCfg.IP_Server[0],
                LocalCfg.IP_Server[1], LocalCfg.IP_Server[2], LocalCfg.IP_Server[3]);
         Multi_Udp_Buff[i].SendNum = 0;   //��෢6��
         Multi_Udp_Buff[i].m_Socket = m_DataSocket;
         Multi_Udp_Buff[i].CurrOrder = 0;
         //ͷ��
         memcpy(Multi_Udp_Buff[i].buf, UdpPackageHead, 6);
         //����
         Multi_Udp_Buff[i].buf[6] = CANCELALARM;
         Multi_Udp_Buff[i].buf[7] = ASK;    //����

         memcpy(Multi_Udp_Buff[i].buf + 8, LocalCfg.Addr, 20);
         Multi_Udp_Buff[i].buf[28] = LocalCfg.DefenceStatus;     //����״̬
         Multi_Udp_Buff[i].buf[29] = LocalCfg.DefenceNum;        //����ģ�����

         Multi_Udp_Buff[i].nlength = 30;
         Multi_Udp_Buff[i].DelayTime = 100;
         Multi_Udp_Buff[i].isValid = 1;
         sem_post(&multi_send_sem);
         break;
        }
     //�򿪻�����
     pthread_mutex_unlock (&Local.udp_lock);
}
*/
//---------------------------------------------------------------------------
//��״̬��ʾ��Ϣ����
void ClearTipFunc(void)
{
  //�粻�ڿ������棬���ʱ��30S�޲����Զ�����
  //���ں��кͲ�������    �����û����ӽ���  ���ڱ������� ����ȡ����������
	if((Local.CurrentWindow != MainWindow)&&(Local.Status == 0))//&&(Local.CurrentWindow != 34)&&(Local.CurrentWindow != 35)
	{
		PicStatBuf.KeyPressTime ++;
		if(PicStatBuf.KeyPressTime > 30*INTRPERSEC)   //10*30
		{
			PicStatBuf.KeyPressTime = 0;
			Local.CurrentWindow = 0;
			Local.ShowHotkey = 0;
			DisplayMainWindow(Local.CurrentWindow);
		}
	}
	//���ڿ������棬60S�޲�����LCD
	if(Local.CurrentWindow == MainWindow)
	{
		if(Local.ShowHotkey == 0)
		{
			if(Local.LcdLightFlag == 1)
			{
				Local.LcdLightTime ++;
				if(Local.LcdLightTime > 60*INTRPERSEC)
				{
					if(Local.LcdLightFlag == 1)
					{
						//�ر�LCD
						CloseLCD();
						if(Local.CurrentWindow != MainWindow)
						{
							Local.CurrentWindow = MainWindow;
							DisplayMainWindow(Local.CurrentWindow);
						}
					}
					Local.LcdLightFlag = 0; //LCD�����־
					Local.LcdLightTime = 0; //ʱ��
				}
			}
		}
		else if(Local.ShowHotkey == 1)
		{
			Local.ShowHotkeyTime++;
			if(Local.ShowHotkeyTime > 10* INTRPERSEC)
			{
				Local.ShowHotkey = 0;
				Local.ShowHotkeyTime = 0;
				RestorePicBuf_Func(label_hotkey.xLeft,label_hotkey.yTop,371,80,label_hotkey.image,0);
			}
		}
	}
	else
	{
		if(Local.LcdLightFlag == 0)
		{
			//����LCD����
			OpenLCD();
		}
		Local.LcdLightFlag = 1; //LCD�����־
		Local.LcdLightTime = 0; //ʱ��
	}        

#if 0
  //��ʱ����ʾ��Ϣ��־
  if(PicStatBuf.Flag == 1)
   {
    PicStatBuf.Time ++;
    //1S
    if(PicStatBuf.Time >= (INTRPERSEC*1))
     {
      PicStatBuf.Flag = 0;
      PicStatBuf.Time = 0;
      switch(PicStatBuf.Type)
       {
        case 5:   //���ô��ڣ�һ����
                //�鿴�Ƿ��ǵ�ǰ����
                if(Local.CurrentWindow == PicStatBuf.Type)
                 {
                  //����ʾ
                  RestorePicBuf_Func(176, 159, 42, 48, Label_Setup1.image, 0);
                 }
                break;
        case 12:   //����ͨ��
                //�鿴�Ƿ��ǵ�ǰ����
                if(Local.CurrentWindow == PicStatBuf.Type)
                 {
                  //����ʾ
                  strcpy(Label_R2R.Text, "");
                  ShowLabel(&Label_R2R, REFRESH);
                 }
                break;
        case 13:   //����
                //�鿴�Ƿ��ǵ�ǰ����
                if(Local.CurrentWindow == PicStatBuf.Type)
                 {
                  if((Local.TmpWindow == 16)||(Local.TmpWindow == 190))
                    Local.TmpWindow = 0;
                  DisplayMainWindow(Local.TmpWindow);
                  Local.Status = 0;  //״̬Ϊ����
                 }
                break;
        case 16:   //��ʾ�Խ�ͼ���������
                //�鿴�Ƿ��ǵ�ǰ����
                if(Local.CurrentWindow == PicStatBuf.Type)
                 {
                  DisplayMainWindow(0);
                  Local.Status = 0;  //״̬Ϊ����
                 }
                break;
        case 156:   //�޸Ĺ������루������
        case 192:   //�޸�ʹ�������루������
                //�鿴�Ƿ��ǵ�ǰ����
                if(Local.CurrentWindow == PicStatBuf.Type)
                 {
                  //����ʾ
                  RestorePicBuf_Func(176, 160, 42, 48, Label_Setup1.image, 0);
                 }
                break;
        case 159:   //�������ã�������
                //�鿴�Ƿ��ǵ�ǰ����
                if(Local.CurrentWindow == PicStatBuf.Type)
                 {
                  RestorePicBuf_Func(411, 153, 42, 48, Label_Setup2.image, 0);
                 }
                break;
        case 161:   //IP��ַ���ô��ڣ�������
                //�鿴�Ƿ��ǵ�ǰ����
                if(Local.CurrentWindow == PicStatBuf.Type)
                 {
                  //����ʾ
                  RestorePicBuf_Func(411, 153, 42, 48, Label_Setup2.image, 0);
                 }
                break;
       }
      PicStatBuf.Type = 0;
      CloseOsd(); //�ر�OSD
     }
   }
#endif
}
//---------------------------------------------------------------------------
//д��꺯��   t_Flag 0--���  1--д
void WriteCursor(struct TEdit *t_edit, int t_Flag, int t_show, int PageNo)
{
 ShowCursor = t_show;
 if(t_Flag == 1)
   fb_line(t_edit->xLeft + t_edit->CursorX + t_edit->BoxLen*(t_edit->fWidth) +1,
           t_edit->yTop +t_edit->CursorY,
           t_edit->xLeft + t_edit->CursorX + t_edit->BoxLen*(t_edit->fWidth)+1,
           t_edit->yTop + t_edit->CursorY + t_edit->CursorHeight - 1, t_edit->CursorCorlor, 0);
  else
   RestorePicBuf_Func(t_edit->xLeft + t_edit->CursorX + t_edit->BoxLen*(t_edit->fWidth) +1,
           t_edit->yTop +t_edit->CursorY,
           1, t_edit->CursorHeight, t_edit->Cursor_H, 0);   //�ָ�һ����Ļ
}


void WriteCursorTalk(struct TEdit* t_edit,int t_Flag,int t_show,int PageNo)
{
	ShowCursor = t_show;

	if(t_edit->BoxLen < 4)
	{
		if(t_Flag == 1)
		{
			fb_line(230+t_edit->BoxLen*(t_edit->fWidth)+1,203,230+t_edit->BoxLen*(t_edit->fWidth)+1,
				203+t_edit->CursorHeight - 1,t_edit->CursorCorlor,0);
		}
		else
		{
			RestorePicBuf_Func(230 + t_edit->BoxLen*(t_edit->fWidth) +1,203,1, t_edit->CursorHeight, t_edit->Cursor_H, 0);
		}
	}
	else if((t_edit->BoxLen>=4)&&(t_edit->BoxLen<6))
	{
		if(t_Flag == 1)
		{
			fb_line(304+(t_edit->BoxLen-4)*(t_edit->fWidth)+1,203,304+(t_edit->BoxLen-4)*(t_edit->fWidth)+1,
				203+t_edit->CursorHeight - 1,t_edit->CursorCorlor,0);
		}
		else
		{
			RestorePicBuf_Func(304 + (t_edit->BoxLen-4)*(t_edit->fWidth) +1,203,1, t_edit->CursorHeight, t_edit->Cursor_H, 0);
		}
	}
	else if(t_edit->BoxLen >= 6)
	{
		if(t_Flag == 1)
		{
			fb_line(389+(t_edit->BoxLen-6)*(t_edit->fWidth)+1,203,389+(t_edit->BoxLen-6)*(t_edit->fWidth)+1,
				203+t_edit->CursorHeight - 1,t_edit->CursorCorlor,0);
		}
		else
		{
			RestorePicBuf_Func(389 + (t_edit->BoxLen-6)*(t_edit->fWidth) +1,203,1, t_edit->CursorHeight, t_edit->Cursor_H, 0);
		}
	}
}

//---------------------------------------------------------------------------
//��ʾ�����꺯��
void ShowCursorFunc(void)
{
  if(ShowCursor == 1)
   {
    switch(Local.CurrentWindow)
     {
#if 0
      case 31:  //�������ڲ�����������
      case 35:  //ȡ���������ڲ�����������
              WriteCursor(&cancelfortify_edit, CursorFlag, ShowCursor, 0);
              break;
      case 12:  //����ͨ���������(������
              WriteCursor(&r2r_edit[CurrBox], CursorFlag, ShowCursor, 0);
              break;
      case 3:  //�Խ����ڲ�����һ����
              WriteCursor(&r2r_edit[CurrBox], CursorFlag, ShowCursor, 0);
              break;
      case 5:  //���ô��ڲ�����һ����
              WriteCursor(&setup_pass_edit, CursorFlag, ShowCursor, 0);
              break;
      case 156:  //�޸Ĺ������������������
              WriteCursor(&modi_engi_edit[CurrBox], CursorFlag, ShowCursor, 0);
              break;
      case 159:  //�������ò�����������
              WriteCursor(&addr_edit[CurrBox], CursorFlag, ShowCursor, 0);
              break;
      case 161:  //IP��ַ���ò�����������
              WriteCursor(&ip_edit[CurrBox], CursorFlag, ShowCursor, 0);
              break;
#endif
		default:
			break;
     }
    if(CursorFlag == 1)
      CursorFlag = 0;
    else
      CursorFlag = 1;
   }
}
//---------------------------------------------------------------------------
void TalkCtrlFunc(void)  //�Խ����ƣ���ʾͨ��ʱ����жϳ�ʱ
{
	char strtime[30];
	int PrevStep = 0;
	int CurrStep = 0;
	int CallTimeOut;
	char OsdContent[20];
	//1S
	if((Local.TimeOut % INTRPERSEC)==0)
	switch(Local.Status)
	{
	case 1:  //���жԽ�
	case 2:  //���жԽ�
		CallTimeOut = CALLTIMEOUT;
		//�ڱ�����ʱ����������Ӱ���ܣ���������ʱֵ�ȸ�����3S
		if((LocalCfg.Addr[0] == 'S')||(LocalCfg.Addr[0] == 'B'))
		{
			if(Local.isHost == '0')
				CallTimeOut = CALLTIMEOUT;
			else
				CallTimeOut = CALLTIMEOUT + 3*INTRPERSEC;
		}
		if(Local.TimeOut >= CallTimeOut)
		{
			DropMultiGroup(m_VideoSocket, NULL);
			CallTimeOut_Func();
		}
		break;
	case 5:  //����ͨ��
	case 6:  //����ͨ��
		sprintf(OsdContent, "%02d:%02d\0", Local.TimeOut/INTRPERSEC/60,(Local.TimeOut/INTRPERSEC)%60);
		OsdContent[2] = ',';
		OsdContent[5] = '\0';
		ShowOsd(OsdContent);

		if(Local.TimeOut >= Local.TalkTimeOut)
		{
			Local.TalkTimeOut = TALKTIMEOUT; //ͨ���ʱ��
			TalkEnd_Func();
			Local.OnlineFlag = 0;
#ifdef _DEBUG
			printf("ͨ����ʱ\n");
#endif
		}
		break;
	}
	Local.TimeOut ++;       //���ӳ�ʱ,  ͨ����ʱ,  ���г�ʱ�����˽���
}
//---------------------------------------------------------------------------
void TalkOsd(void)  //ͨ���ͼ���ʱOSD��ʾ
{
	char OsdContent[20];
	sprintf(OsdContent, "%02d:%02d\0", Local.TimeOut/INTRPERSEC/60,
	     (Local.TimeOut/INTRPERSEC)%60);
	OsdContent[2] = ',';
	OsdContent[5] = '\0';
	ShowOsd(OsdContent);
}
//---------------------------------------------------------------------------
void OnlineCheckFunc(void) //����ȷ�ϼ�⺯��
{
 	unsigned char send_b[1520];
  	int sendlength;
  
	if((Local.Timer1Num >=(INTRPERSEC*6)))
	{
		if(Local.CallConfirmFlag == 0)
		{
			if((Local.Status == 1)||(Local.Status == 2)||(Local.Status == 5)||(Local.Status == 6)
				||(Local.Status == 7)||(Local.Status == 8)||(Local.Status == 9)||(Local.Status == 10)) //�Խ�
				TalkEnd_Func();
			Local.OnlineFlag = 0;

#ifdef _DEBUG
			printf("û���յ�����ȷ�ϣ�ǿ�ƽ���\n");
#endif
		}
		else
			Local.CallConfirmFlag = 0;

		Local.Timer1Num = 0;
	}
	else if((Local.Timer1Num %INTRPERSEC)==0)
	{
		//�Խ�ʱ���з���������ȷ�ϰ���ÿ��һ��
		//���ʱ���ط���������ȷ�ϰ���ÿ��һ��
		//  printf("Local.Status = %d\n", Local.Status);
		if((Local.Status == 2)||(Local.Status == 6)
		||(Local.Status == 8)||(Local.Status == 10)
		||(Local.Status == 3))
		{
			//ͷ��
			memcpy(send_b, UdpPackageHead, 6);
			//����
			if((Local.Status == 2)||(Local.Status == 6)||(Local.Status == 8)||(Local.Status == 10))  //�Խ�
				send_b[6] = VIDEOTALK;
			send_b[7]=ASK;        //����
			send_b[8]=CALLCONFIRM;//ͨ������ȷ��
			//������
			if(Local.Status == 3) //����ʱ������Ϊ���з�
			{
				memcpy(send_b+9, LocalCfg.Addr, 20);
				memcpy(send_b+29, LocalCfg.IP, 4);
				memcpy(send_b+33, Remote.Addr[0], 20);
				memcpy(send_b+53, Remote.IP[0], 4);
			}
			if((Local.Status == 2)||(Local.Status == 4)||(Local.Status == 6)
			||(Local.Status == 8)||(Local.Status == 10))  //�Խ�ʱ������Ϊ���з�
			{
				memcpy(send_b+9, Remote.Addr[0], 20);
				memcpy(send_b+29, Remote.IP[0], 4);
				memcpy(send_b+33, LocalCfg.Addr, 20);
				memcpy(send_b+53, LocalCfg.IP, 4);
			}
			//ȷ�����
			send_b[60] = (Local.OnlineNum & 0xFF000000) >> 24;
			send_b[59] = (Local.OnlineNum & 0x00FF0000) >> 16;
			send_b[58] = (Local.OnlineNum & 0x0000FF00) >> 8;
			send_b[57] = Local.OnlineNum & 0x000000FF;
			Local.OnlineNum ++;
			sendlength=61;
			sprintf(RemoteHost, "%d.%d.%d.%d\0",Remote.DenIP[0],Remote.DenIP[1],Remote.DenIP[2],Remote.DenIP[3]);
			UdpSendBuff(m_VideoSocket, RemoteHost, send_b , sendlength);
		}
	}
	Local.Timer1Num ++;
	//�Խ����ƣ���ʾͨ��ʱ����жϳ�ʱ
	TalkCtrlFunc();
}
//---------------------------------------------------------------------------
void TimeReportStatusFunc(void) //�豸��ʱ����״̬����
{
	int i,j,k;
	//����������
	pthread_mutex_lock (&Local.udp_lock);
	//���ҿ��÷��ͻ��岢���
	for(i=0; i<UDPSENDMAX; i++)
	if(Multi_Udp_Buff[i].isValid == 0)
	{
		sprintf(Multi_Udp_Buff[i].RemoteHost, "%d.%d.%d.%d\0",LocalCfg.IP_Server[0],
		LocalCfg.IP_Server[1], LocalCfg.IP_Server[2], LocalCfg.IP_Server[3]);
		printf("%d.%d.%d.%d\n",LocalCfg.IP_Server[0],
		LocalCfg.IP_Server[1], LocalCfg.IP_Server[2], LocalCfg.IP_Server[3]);
		Multi_Udp_Buff[i].SendNum = 0;   //��෢6��
		Multi_Udp_Buff[i].m_Socket = m_DataSocket;
		Multi_Udp_Buff[i].CurrOrder = 0;
		//ͷ��
		memcpy(Multi_Udp_Buff[i].buf, UdpPackageHead, 6);
		//memcpy(Multi_Udp_Buff[i].buf,"XXXCID", 6);
		//����
		Multi_Udp_Buff[i].buf[6] = REPORTSTATUS;
		Multi_Udp_Buff[i].buf[7] = ASK;    //����

		memcpy(Multi_Udp_Buff[i].buf + 8, LocalCfg.Addr, 20);
		memcpy(Multi_Udp_Buff[i].buf + 28, LocalCfg.Mac_Addr, 6);
		Multi_Udp_Buff[i].buf[34] = LocalCfg.DefenceStatus;
		Multi_Udp_Buff[i].buf[35] = LocalCfg.DefenceNum;
		for(k=0; k<(LocalCfg.DefenceNum*6); k++)
			memcpy(Multi_Udp_Buff[i].buf + 36 + 10*k, LocalCfg.DefenceInfo[k], 10);

		Multi_Udp_Buff[i].nlength = 36 + LocalCfg.DefenceNum*6*10;
		Multi_Udp_Buff[i].DelayTime = 100;
		Multi_Udp_Buff[i].isValid = 1;
		sem_post(&multi_send_sem);
		break;
	}
	//�򿪻�����
	pthread_mutex_unlock (&Local.udp_lock);
}
//---------------------------------------------------------------------------
//���IP��ַ��ͻ
void CheckIPConflict(void)
{
	int result;
	int i;
	unsigned char to_user[7];
	result = 0;
	result = ioctl(m_DataSocket, CONFLICTARP, to_user);
	if(result == CONFLICTARP)
	{
		if(to_user[0] == 1)    //����
		{
#ifdef _DEBUG
			printf("IP��ַ��ͻ,�Է�Ӳ����ַ%02X:%02X:%02X:%02X:%02X:%02X\n",
				to_user[1],to_user[2],to_user[3],to_user[4],to_user[5],to_user[6]);
#endif
		}
		if(to_user[0] == 2)   //Ӧ��
		{
			//����������
			pthread_mutex_lock (&Local.udp_lock);
			for(i=0; i<UDPSENDMAX; i++)
			if(Multi_Udp_Buff[i].isValid == 1)
			if(Multi_Udp_Buff[i].m_Socket == ARP_Socket)
			{
				Multi_Udp_Buff[i].isValid = 0;
#ifdef _DEBUG
				printf("IP��ַ��ͻ,�Է�Ӳ����ַ%02X:%02X:%02X:%02X:%02X:%02X\n",
				to_user[1],to_user[2],to_user[3],to_user[4],to_user[5],to_user[6]);
#endif
				break;
			}
			//�򿪻�����
			pthread_mutex_unlock (&Local.udp_lock);
		}
	}
}
//---------------------------------------------------------------------------
//��������ź�
void CheckEth(void)
{
	int retval;
	int NewNetSpeed;
	
	retval = detect_ethtool(m_EthSocket, "eth0");
	if(retval != Local.NetStatus)   //����״̬ 0 �Ͽ�  1 ��ͨ
	{
		if (retval == 0)
		{
			Local.NetStatus = retval;
			if(Local.CurrentWindow == MainWindow)
			{	
				DisplayImageButton(&state_image[4],IMAGEDOWN, 0);
			}
		}

		else if (retval == 1)
		{
			Local.NetStatus = retval;
			if(Local.CurrentWindow == MainWindow)
			{
				DisplayImageButton(&state_image[4],IMAGEUP, 0);
			}
			//�������ARP
			SendFreeArp();

			NewNetSpeed = get_ethtool_speed(m_EthSocket, "eth0");
			if(NewNetSpeed != Local.OldNetSpeed)
			{
				Local.OldNetSpeed = NewNetSpeed;
				system("ethtool -s eth0 autoneg off");
				usleep(1000*1000);
				system("ethtool -s eth0 autoneg on");
				printf("����������������\n");
			}
		}
	}
}
//---------------------------------------------------------------------------
int detect_ethtool(int skfd, char *ifname)
{
	struct ifreq ifr;
	struct ethtool_value edata;

	memset(&ifr, 0, sizeof(ifr));
	edata.cmd = ETHTOOL_GLINK;

	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name)-1);
	ifr.ifr_data = (char *) &edata;

	if (ioctl(skfd, SIOCETHTOOL, &ifr) == -1)
	{
	    printf("ETHTOOL_GLINK failed\n");
	    return 2;
	}

	return (edata.data);// ? 0 : 1);
}
//---------------------------------------------------------------------------
struct ethtool_cmd eth_data;
int get_ethtool_speed(int skfd, char *ifname)  //��ȡ��������
{
        struct ifreq ifr;
      //  struct ethtool_cmd eth_data;

        memset(&ifr, 0, sizeof(ifr));
        eth_data.cmd = ETHTOOL_GSET;

        strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name)-1);
        ifr.ifr_data=(char*)&eth_data;

        if (ioctl(skfd, SIOCETHTOOL, &ifr) == -1)
        {
                printf("ETHTOOL_GSET failed\n");
                return 2;
        }
        printf("eth_data.speed = %d, eth_data.duplex = %d, eth_data.autoneg = %d\n", eth_data.speed, eth_data.duplex, eth_data.autoneg);
        return (eth_data.speed);// ? 0 : 1);
}
//---------------------------------------------------------------------------
extern int audio_play_wav_flag;
//�����ƵPlay Ring, ������������������BUG����һ�ֲ�
void CheckPlayRing(void)
{
	int Ring_Status;
	if(gpio_fd > 0)
	{
		Ring_Status = ioctl(gpio_fd, CHECK_PLAY_RING, NULL);
		if(Ring_Status == 1)
			Local.ResetPlayRingFlag = 1;
		if(Local.ResetPlayRingFlag == 1)
		if((audio_play_wav_flag == 0)&&(Local.Status == 0))
		{
			Local.ResetPlayRingFlag = 0;
			//�ر���������
			CloseSnd(AUDIODSP1);
			//����������
			if(!OpenSnd(AUDIODSP1))
			{
				printf("Open play sound device error!\n");
				return;
			}
			printf("reset audio play ring\n");
		}
	}
}
//---------------------------------------------------------------------------
