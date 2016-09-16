#include <inttypes.h>
#include <signal.h>
#include <semaphore.h>       //sem_t
#include <sys/stat.h>
#include <pthread.h>
#include "sndtools.h"
#include "./include/image.h"

#define MainWindow				0
#define TalkMenuWindow			3
#define TalkPicWindow			31
#define TalkAreaMessageWindow 	32
#define TalkLocalMessageWindow	33
#define TalkPushMessageWindow	34
#define TalkCustomMessageWindow	35
#define TalkMisscallWindow		36
#define TalkCalledWindow		37
#define TalkCallWindow			38
#define TalkPhonebookWindow		39
#define TalkBlacklistWindow		40

#define SetupMainWindow			200

#define LanSetWindow			41
#define WlanSetWindow			42
#define RoomSetWindow			43
#define ScreenContrastWindow	44
#define ScreenSavingWindow		45
#define ScreenCalibrateWindow	46
#define VoiceSetWindow			47
#define TimeSetWindow			48
#define PassSetWindow			49
#define LangSetWindow			50
#define UpgradeSDWindow			51
#define UpgradeRemoteWindow		52
#define SystemInfoWindow		53
#define TalkInfoContentWindow   54






#define cfg_name "/mnt/mtd/config/cfg"
#define picinfo_name "/mnt/mtd/config/picinfo"
#define info_name "/mnt/mtd/config/info"
#define picini_name1 "/mnt/mtd/picture/picini1"
#define picini_name2 "/mnt/mtd/picture/picini2"
#define mtdexe_name "/mnt/mtd/sound70"
//#define UdpPackageHead  "XXXCID"
#define FLAGTEXT   "hikdsdkkkkdfdsIMAGE"
#define FLAGTEXT43 "hikdsdkkkkdfdsPIC70"

//#define _ZHUHAIJINZHEN      //�麣����  1������ʱ�ɿ���  2��ͨ��ʱ����2���Ҷ� 20081127
#define _DEBUG           //����ģʽ
//#define _R2RVIDEO        //�����Խ���Ƶ
#define CIF_X    128
#define CIF_Y    124
#define MINIDOOR_OPENLOCK_IO          2   //С�ſڻ��������ߵ�ƽ��Ч��
#define MINIDOOR_POWER_IO             3   //С�ſڻ���Դ���͵�ƽ��Ч��
#define MINIDOOR_AUDIO_IO             4   //С�ſڻ���Ƶ�л����͵�ƽ��Ч��

//#define _TESTNSSERVER        //���Է���������ģʽ
//#define _TESTTRANS           //������Ƶ��תģʽ

#define HARDWAREVER "S-HW VER 1.0"    //Ӳ���汾
#define SOFTWAREVER "S-SW VER 1.0"    //����汾
#define SERIALNUM "ET20080816"    //��Ʒ���к�

#define NSMULTIADDR  "238.9.9.1"  //NS�鲥��ַ

#define ZOOMMAXTIME 2000   //�Ŵ���С�ӳٴ���ʱ��
#define TOUCHMAXTIME 300   //�����������ӳٴ���ʱ��

#define INTRTIME 50       //�߳�50ms
#define INTRPERSEC 20       //ÿ��20���߳�
#define BUFFER_SIZE 1024 
#define FRAMEBUFFERMAX  4
#define COMMMAX 1024     //���ڻ��������ֵ
#define SCRWIDTH  800
#define SCRHEIGHT  600//600//480
#define REFRESH  1
#define NOREFRESH 0
#define SHOW  0
#define HIDE  1
#define HILIGHT  2
#define IMAGEUP  0
#define IMAGEDOWN  1
#define CIF_W    352
#define CIF_H    240
#define D1_W    720
#define D1_H    480
#define TIPW     320     //��ʾ�����
#define TIPH     28      //��ʾ���߶�
#define DIRW     322     //��ʾ�����
#define DIRH     20      //��ʾ���߶�
#define TIPX     320     //��ʾ��X
#define TIPY     24      //��ʾ��Y
#define CLOCKW     75     //ʱ�ӿ��
#define CLOCKH     20      //ʱ�Ӹ߶�
#define CLOCKX     514     //ʱ��X
#define CLOCKY     30       //ʱ��Y
#define WEATHERW     43     //�������
#define WEATHERH     20      //�����߶�
#define WEATHERX     665     //����X
#define WEATHERY     30       //����Y
#define STATEX     0     //״̬��ͼ��X
#define STATEY     454     //״̬��ͼ��Y

#define MAXCOUNT   6      //�����������

#define CALLCENTERX     150     //��������X
#define CALLCENTERY     224    //��������Y

#define R2RX     150             //����ͨ��X
#define R2RY     330            //����ͨ��Y

#define WATCHX     150     //����X
#define WATCHY     224    //����Y

#define CALLX     150     //����X
#define CALLY     224    //����Y

#define SETUP1X     50     //���ô���1X
#define SETUP1Y     50    //���ô���1Y

#define INFOROWLEN   32    //��Ϣÿ�г���
#define MAXROW  12          //�������
#define PAGEPERROW  3          //ҳ����

#define WATCHTIMEOUT  30*(1000/INTRTIME)    //�����ʱ��
#define CALLTIMEOUT  25*(1000/INTRTIME)     //�����ʱ��
#define TALKTIMEOUT  130*(1000/INTRTIME)//30*20     //ͨ���ʱ��
#define PREPARETIMEOUT  10*(1000/INTRTIME)     //��Ӱ����Ԥ���ʱ��
#define RECORDTIMEOUT  30*(1000/INTRTIME)     //��Ӱ�����ʱ��

//���� ��������
#define ALARM         1    //����
#define CANCELALARM   2    //ȡ������
#define SENDMESSAGE   3   //������Ϣ
#define REPORTSTATUS  4   //�豸��ʱ����״̬
#define QUERYSTATUS   5   //�������Ĳ�ѯ�豸״̬
#define REMOTEDEFENCE   20   //Զ�̲���
#define RESETPASS       30   //��λ����
#define WRITEADDRESS   40   //д��ַ����
#define READADDRESS    41   //����ַ����
#define WRITEROOMSETUP     44   //д���ڻ���������
#define READROOMSETUP      45   //�����ڻ���������
#define WRITESETUP     52   //д��Ԫ�ſڻ���Χǽ��������Ϣ
#define READSETUP      53   //����Ԫ�ſڻ���Χǽ��������Ϣ
//�Խ�
#define VIDEOTALK      150 //���������ӶԽ�
#define VIDEOTALKTRANS 151 //���������ӶԽ���ת����
#define VIDEOWATCH     152 //���������
#define VIDEOWATCHTRANS   153 //�����������ת����
#define NSORDER        154 //�����������������ڹ㲥��
#define NSSERVERORDER  155 //����������(NS������)
#define FINDEQUIP      170 //�����豸

#define ASK              1     //�������� ����
#define REPLY            2     //�������� Ӧ��

#define CALL             1     //����
#define LINEUSE          2     //ռ��
#define QUERYFAIL        3      //ͨ��ʧ��
#define CALLANSWER       4     //����Ӧ��
#define CALLSTART        6     //��ʼͨ��

#define CALLUP           7     //ͨ������1�����з�->���з���
#define CALLDOWN         8     //ͨ������2�����з�->���з���
#define CALLCONFIRM      9     //ͨ������ȷ�ϣ����շ����ͣ��Ա㷢�ͷ�ȷ�����ߣ�
#define REMOTEOPENLOCK   10     //Զ�̿���
#define FORCEIFRAME      11     //ǿ��I֡����
#define ZOOMOUT          15     //�Ŵ�(720*480)
#define ZOOMIN           16     //��С(352*288)

#define CALLEND          30     //ͨ������

#define DOWNLOADFILE  224    //����Ӧ�ó���
#define DOWNLOADIMAGE  225    //����ϵͳӳ��
#define STARTDOWN  1       //��ʼ����
#define DOWN       2       //����
#define DOWNFINISHONE       3  //�������һ��
#define STOPDOWN       10      //ֹͣ����
#define DOWNFINISHALL       20 //ȫ���������
#define DOWNFAIL         21 //����ʧ��  �豸������������

#define ERASEFLASH  31    //����ɾ��Flash
#define WRITEFLASH  32    //����дFlash
#define CHECKFLASH  33    //����У��Flash
#define ENDFLASH  34      //���дImage
#define ERRORFLASH  35      //����Imageʧ��


#define MAINPICNUM  24      //��ҳͼƬ����
#define MAINLABELNUM  2     //��ҳLabel����
#define DOWNLOAD  220      //����
#define ASK  1
#define REPLY  2

#define SAVEMAX  50     //FLASH�洢�������ֵ
#define UDPSENDMAX  50  //UDP��η��ͻ������ֵ
#define COMMSENDMAX  10  //COMM��η��ͻ������ֵ
#define MAXSENDNUM  6  //����ʹ���

//��ťѹ��ʱ��
#define DELAYTIME  200
//��ť����
#define InfoButtonMax  16
//����Ϣ
#define INFOTYPENUM 4 //4    //����Ϣ����
#define INFOMAXITEM  50 //200    //����Ϣ�������
#define INFOMAXSIZE  400 //����Ϣ�����������
#define INFONUMPERPAGE 9  //һҳ��ʾ��Ϣ��
//��Ƭ
#define PICNUM   20

//��Ƶ����
#define cWhite  1
#define cYellow 2
#define cCyan   3
#define cGreen  4
#define cMagenta  5
#define cRed      6
#define cBlue     7
#define cBlack    8
#define	FB_DEV	"/dev/fb0"

#define VIDEO_PICTURE_QUEUE_SIZE_MAX 20

#define CONFLICTARP  0x8950
#define FLCD_GET_DATA_SEP   0x46db
#define FLCD_GET_DATA       0x46dc
#define FLCD_SET_FB_NUM     0x46dd


#define FLCD_SWITCH_MODE    0x46de
#define FLCD_CLOSE_PANEL    0x46df
#define FLCD_BYPASS    0x46e0
#define FLCD_OPEN	0x46fa
#define FLCD_CLOSE	0x46fb

#define DEVICE_GPIO                "/dev/gpio"
#define IO_PUT                 0
#define IO_CLEAR               3
#define IO_READ         4
#define IO_SETINOUT     5
#define IO_TRIGGERMODE  6
#define IO_EDGE         7
#define IO_SETSCANVALUE     8
#define IO_SETVALUE     9

//20080401 ����PMU���رղ��õ�ʱ��
#define CLOSE_PMU1  0x2255
#define CLOSE_PMU2  0x2256
//20080802 ���Audio play ring stoped
#define CHECK_PLAY_RING  0x2257

#define NMAX 512*64  //AUDIOBLK*64  //��Ƶ���λ�������С
#define G711NUM  64*512/AUDIOBLK       //��Ƶ���ջ��������� δ����   10

#define VIDEOMAX 720*480
#define VNUM  3         //��Ƶ�ɼ���������С
#define VPLAYNUM  10         //��Ƶ���Ż�������С         6
#define MP4VNUM  20         //��Ƶ���ջ��������� δ����   10
#define PACKDATALEN  1200   //���ݰ���С
#define MAXPACKNUM  100     //֡������ݰ�����

struct TimeStamp1{
    unsigned int OldCurrVideo;     //��һ�ε�ǰ��Ƶʱ��
    unsigned int CurrVideo;
    unsigned int OldCurrAudio;     //��һ�ε�ǰ��Ƶʱ��
    unsigned int CurrAudio;
   };
//��Ƶ�ɼ�����
struct videobuf1
 {
  int iput; // ���λ������ĵ�ǰ����λ��
  int iget; // �������ĵ�ǰȡ��λ��
  int n; // ���λ������е�Ԫ��������
  uint32_t timestamp[VNUM]; //ʱ���

  uint32_t frameno[VNUM];   //֡���
  unsigned char *buffer_y[VNUM];//[VIDEOMAX];
  unsigned char *buffer_u[VNUM];//[VIDEOMAX/4];
  unsigned char *buffer_v[VNUM];//[VIDEOMAX/4];
 };
//��Ƶ���ջ���  δ����
struct tempvideobuf1
 {
//  int iput;                     // ���λ������ĵ�ǰ����λ��
//  int iget;                     // �������ĵ�ǰȡ��λ��
//  int n;                        // ���λ������е�Ԫ��������
  uint32_t timestamp;  //ʱ���
  uint32_t frameno;       //֡���
  short TotalPackage;     //�ܰ���
  uint8_t CurrPackage[MAXPACKNUM]; //��ǰ��   1 �ѽ���  0 δ����
  int Len;                //֡���ݳ���
  uint8_t isFull;                  //��֡�ѽ�����ȫ
  unsigned char *buffer;//[VIDEOMAX];
  unsigned char frame_flag;             //֡��־ ��Ƶ֡ I֡ P֡  
 };                            //     [MP4VNUM]
//��Ƶ���ջ��� ����
typedef struct node2{
               struct tempvideobuf1 Content;
               struct node2 *llink, *rlink;
}TempVideoNode1;
//��Ƶ���Ż���
struct videoplaybuf1
 {
  uint8_t isUse;     //��֡�ѽ���δ����,������������
  uint32_t timestamp; //ʱ���
  uint32_t frameno;   //֡���
  unsigned char *buffer;//[VIDEOMAX];
  unsigned char frame_flag;             //֡��־ ��Ƶ֡ I֡ P֡
 };
//ͬ�����Žṹ
struct _SYNC
 {
  pthread_cond_t cond;       //ͬ���߳���������
  pthread_condattr_t cond_attr;
  pthread_mutex_t lock;      //������
  pthread_mutex_t audio_rec_lock;//[VPLAYNUM];//��Ƶ¼�ƻ�����
  pthread_mutex_t audio_play_lock;//[VPLAYNUM];//��Ƶ���Ż�����
  pthread_mutex_t video_rec_lock;//[VPLAYNUM];//��Ƶ¼�ƻ�����
  pthread_mutex_t video_play_lock;//[VPLAYNUM];//��Ƶ���Ż�����
  unsigned int count;        //����
  uint8_t isDecodeVideo;     //��Ƶ�ѽ���һ֡  �����߳�-->ͬ���߳�
  uint8_t isPlayVideo;       //��Ƶ�Ѳ���һ֡  �����߳�-->ͬ���߳�
  uint8_t isDecodeAudio;     //��Ƶ�ѽ���һ֡  �����߳�-->ͬ���߳�
  uint8_t isPlayAudio;       //��Ƶ�Ѳ���һ֡  �����߳�-->ͬ���߳�
 };

//�ӻ�����? 
struct audiobuf1
 {
  int iput; // ���λ������ĵ�ǰ����λ��
  int iget; // �������ĵ�ǰȡ��λ�� 
  int n; // ���λ������е�Ԫ��������
  uint32_t timestamp[NMAX/AUDIOBLK]; //ʱ���
  uint32_t frameno[NMAX/AUDIOBLK];   //֡���
  unsigned char buffer[NMAX];
 };

//��Ƶ���ջ���  δ����
struct tempaudiobuf1
 {
  uint32_t timestamp;  //ʱ���
  uint32_t frameno;       //֡���
  short TotalPackage;     //�ܰ���
  uint8_t CurrPackage[MAXPACKNUM]; //��ǰ��   1 �ѽ���  0 δ����
  int Len;                //֡���ݳ���
  uint8_t isFull;                  //��֡�ѽ�����ȫ
  unsigned char *buffer;//[AUDIOBLK];
  unsigned char frame_flag;             //֡��־ ��Ƶ֡ I֡ P֡
 };                            //     [MP4VNUM]
//��Ƶ���ջ��� ����
typedef struct node3{
               struct tempaudiobuf1 Content;
               struct node3 *llink, *rlink;
}TempAudioNode1;

//��Ƶ���Ż���
struct audioplaybuf1
 {
  uint8_t isUse;     //��֡�ѽ���δ����,������������
  uint32_t timestamp; //ʱ���
  uint32_t frameno;   //֡���
  unsigned char *buffer;//[VIDEOMAX];
 };

//��ͥ���Ի���
struct wavbuf1
 {
  int iput; // ���λ������ĵ�ǰ����λ��
  int iget; // �������ĵ�ǰȡ��λ��
  int n; // ���λ������е�Ԫ��������
  unsigned char *buffer;
 };

struct WaveFileHeader
{
 char chRIFF[4];
 uint32_t dwRIFFLen;
 char chWAVE[4];

 char chFMT[4];
 uint32_t dwFMTLen;
 uint16_t wFormatTag;
 uint16_t nChannels;
 uint32_t nSamplesPerSec;
 uint32_t nAvgBytesPerSec;
 uint16_t nBlockAlign;
 uint16_t wBitsPerSample;

 char chFACT[4];
 uint32_t dwFACTLen;

 char chDATA[4];
 uint32_t dwDATALen;
};

/*typedef */struct fcap_frame_buff
{
    unsigned int phyAddr;
    unsigned int mmapAddr;   //length per dma buffer
    unsigned int frame_no;
};
struct Local1{
               int Status;
		 int ShowHotkey;
			   int ShowHotkeyTime;
		int ShowRecPic;
               //״̬ 0 ���� 1 ���жԽ�  2 ���жԽ�  3 ����  4 ������  5 ����ͨ��
               //6 ����ͨ��
               //21 С�ſڻ����жԽ�  22 С�ſڻ�����ͨ�� 23 С�ſڻ�����
               int RecordPic;  //����Ƭ  0 ����  1 ��������Ƭ  2 ͨ������Ƭ
               int IFrameCount; //I֡����
               int IFrameNo;    //���ڼ���I֡
               unsigned char yuv[2][CIF_W*CIF_H*3/2];
               int HavePicRecorded;  //����Ƭ��¼��
               int HavePicRecorded_flag;
               struct tm *recpic_tm_t; //����Ƭʱ��               

               struct tm *call_tm_t; //������ʱ��
               
               int CallConfirmFlag; //���߱�־
               int Timer1Num;  //��ʱ��1����
               int OnlineFlag; //��������ȷ��
               int OnlineNum;  //����ȷ�����
               int TimeOut;    //���ӳ�ʱ,  ͨ����ʱ,  ���г�ʱ�����˽���
               int TalkTimeOut; //ͨ���ʱ��
               int RecPicSize;  //��Ƶ��С  1  352*288   2  720*480
               int PlayPicSize;  //��Ƶ��С  1  352*288   2  720*480
               pthread_mutex_t save_lock;//������
               pthread_mutex_t udp_lock;//������
               pthread_mutex_t comm_lock;//������
               int PrevWindow;      //��һ�����ڱ��
               int TmpWindow;       //�ݴ洰�ڱ�� ���ڵ�������ʱ
               int CurrentWindow;   //��ǰ���ڱ��
               int DefenceDelayFlag;    //������ʱ��־
               int DefenceDelayTime;   //����
               int PassLen;            //���볤��
               int AlarmDelayFlag[2];    //������ʱ��־
               int AlarmDelayTime[2];   //����

               int ForceIFrame;    //1 ǿ��I֡
               int CalibratePos;   //У׼������ʮ��λ�� 0 1 2 3
               int CalibrateSucc;  //У׼�ɹ�
               int CurrFbPage; //��ǰFbҳ
               unsigned char IP_Group[4];  //�鲥��ַ
               unsigned char Weather[3];   //����Ԥ��

               int AddrLen;          //��ַ����  S 12  B 6 M 8 H 6                 

               int isHost;           //'0' ���� '1' ���� '2' ...
               int ConnToHost;       //�������������� 1 ���� 0 ������
               unsigned char HostIP[4]; //����IP
               unsigned char HostAddr[21]; //����Addr
               int DenNum;             //Ŀ������  ����
               unsigned char DenIP[10][4];    //����IP
               char DenAddr[10][21];         //����Addr

               int NetStatus;   //����״̬ 1 �Ͽ�  0 ��ͨ
               int OldNetSpeed;  //�����ٶ�                
               int NoBreak;     //����״̬ 1 ����  0 ����

               int ReportSend;  //�豸��ʱ����״̬�ѷ���
               int RandReportTime; //�豸��ʱ����״̬���ʱ��
               int ReportTimeNum;  //��ʱ
                                 //��GPIO�߳��в�ѯ���߳��Ƿ�����
               int Key_Press_Run_Flag;
               int Save_File_Run_Flag;
               int Dispart_Send_Run_Flag;
               int Multi_Send_Run_Flag;
               int Multi_Comm_Send_Run_Flag;

               int MenuIndex;     //��ǰ��ť����
               int MaxIndex;      //�������������
               int MainMenuIndex;     //�����水ť����

               int OsdOpened;  //OSD�򿪱�־

               int LcdLightFlag; //LCD�����־
               int LcdLightTime; //ʱ��

	     int newInfoNum;
               int ResetPlayRingFlag;  //��λAudio Play flag

               int nowvideoframeno;   //��ǰ��Ƶ֡���
               int nowaudioframeno;   //��ǰ��Ƶ֡���

               int ForceEndWatch;  //�к���ʱ��ǿ�ƹؼ���
               int ZoomInOutFlag;  //���ڷŴ���С��
               uint32_t newzoomtime;
               uint32_t oldzoomtime;
               uint32_t newtouchtime;
               uint32_t oldtouchtime;    //��һ�δ���������ʱ��

               int MiniDoorOpenLockFlag;    //С�ſڻ�������־
               int MiniDoorOpenLockTime;    //С�ſڻ�������ʱ����          

		int keypad_type;	//// 0 number keypad   1 character keypad
		int keypad_show;
		int AlarmStatus;
			   
              };

struct LocalCfg1{
               char Addr[20];             //��ַ����
               unsigned char Mac_Addr[6]; //������ַ
               unsigned char IP[4];       //IP��ַ
               unsigned char IP_Mask[4];  //��������
               unsigned char IP_Gate[4];  //���ص�ַ
               unsigned char IP_NS[4];    //NS�����ƽ�������������ַ
               unsigned char IP_Server[4];  //����������ַ����NS��������Ϊͬһ����
               unsigned char IP_Broadcast[4];  //�㲥��ַ

               int ReportTime;      //�豸��ʱ����״̬ʱ��
               unsigned char DefenceStatus;       //����״̬
               unsigned char DefenceNum;          //����ģ�����
               unsigned char DefenceInfo[32][10]; //������Ϣ

               char EngineerPass[10];             //��������
               
               int In_DelayTime;                //������ʱ
               int Out_DelayTime;               //�����ʱ
               int Alarm_DelayTime;               //������ʱ

               int Ts_X0;                   //������
               int Ts_Y0;
               int Ts_deltaX;
               int Ts_deltaY;

	 int misscall_num;
	   int missmessage_num;
	   int news_num;
};
//���ڽ��ջ�����
struct commbuf1
 {
  int iput; // ���λ������ĵ�ǰ����λ��
  int iget; // �������ĵ�ǰȡ��λ��
  int n; // ���λ������е�Ԫ��������
  unsigned char buffer[COMMMAX];
 };
//״̬��ʾ��Ϣ����
//Type
//          11 -- ��������
//          12 -- ����ͨ��
//          13 -- ����
//          16 -- �Խ�ͼ�񴰿�
struct PicStatBuf1{
               int Flag;                      //��ʱ����ʾ��Ϣ��־
               int Type;                      //����
               int Time;
               int MaxTime;                   //�ʱ��

               int KeyPressTime;               

               unsigned char *InfoContent;      //��Ϣ��ʾ�� , �����Ϸ��·�
               unsigned char *InfoContent1;      //��Ϣ��ʾ�� , �����Ϸ��·�
               unsigned char *InfoButton;       //��Ϣ��ť����
               unsigned char *InfoAllDel;       //��Ϣȫ��ɾ����ť

               unsigned char *PicContent;     //��Ƭ
               
               unsigned char *InfoNum_N[4];       //��Ϣ����Ϣ��δ����Ϣ
               unsigned char *InfoNum_H[4];       //��Ϣ����Ϣ��δ����Ϣ
               unsigned char *Info_Row_N;     //������Ϣ��������ʾ
               unsigned char *Info_Row_H;     //������Ϣ��������ʾ
              };
struct Remote1{
               int DenNum;             //Ŀ������  ����+����
               unsigned char DenIP[4]; //�Է�IP����Ƶ������IP
               unsigned char GroupIP[4]; //GroupIP
               unsigned char IP[10][4];    //�Է�IP
               int Added[10];                //�Ѽ�����
               char Addr[10][21];         //�Է�Addr
               int isDirect;       //�Ƿ�ֱͨ  0 ֱͨ  1 ��ת
              };

struct Info1{
               int MaxNum;   //�����Ϣ��
               int TotalNum; //��Ϣ����
               int NoReadedNum; //δ����Ϣ����
               int TotalInfoPage;   //����Ϣҳ��
               int CurrentInfoPage; //��ǰ��Ϣҳ
               int CurrNo;    //��ǰ��Ϣ���
               int CurrPlayNo;  //��ǰ�������
               int TimeNum;    //����
              };

//������Ϣ���ݽṹ��
struct InfoContent1{
               uint8_t isValid;  //��Ч��δɾ����־   1
               uint8_t isReaded; //�Ѷ���־    1
               uint8_t isLocked; //������־    1
               char Time[32];    //����ʱ��    32
               uint8_t Type;     //����        1    ��Ϣ���ͻ��¼�����
               uint32_t Sn;      //���        4
               int Length;       //����        4
               char Content[INFOMAXSIZE];//����  400  ���ݻ��¼�����
               char Event[20];         //�¼�
              };                               //�ڴ����Ϊ444
//��ǰ��Ϣ����״̬              
struct InfoStatus1{
                  int CurrType;  //��ǰ��Ϣ����
                  int CurrWin;   //��ǰ��Ϣ����  0 ��Ϣ�б�  1  ��Ϣ����
                  int CurrNo;    //��ǰ��Ϣ���
                 };
//��Ϣ����
typedef struct node{
               struct InfoContent1 Content;
               struct node *llink, *rlink;
}InfoNode1;

//�洢�ļ���FLASH���� ���ݽṹ ���ڴ洢FLASH�ٶȽ��� ���߳�������
struct Save_File_Buff1{
               int isValid; //�Ƿ���Ч
               int Type;    //�洢���� 1��һ����Ϣ  2��������Ϣ  3����ҵ����  4����������
               int InfoType;   //��Ϣ����
               int InfoNo;     //��Ϣλ��
               InfoNode1 *Info_Node; //��Ϣ���
              };

//UDP�����������ݷ��ͽṹ
struct Multi_Udp_Buff1{
               int isValid; //�Ƿ���Ч
               int SendNum; //��ǰ���ʹ���
               int CurrOrder;//��ǰ����״̬,VIDEOTALK VIDEOTALKTRANS VIDEOWATCH VIDEOWATCHTRANS
                             //��Ҫ���������ʱ���絥��������Ϊ0
               int m_Socket;
               char RemoteHost[20];
               unsigned char buf[1500];
               int DelayTime;  //�ȴ�ʱ��
               int nlength;
              };
              
//COMM�����������ݷ��ͽṹ
struct Multi_Comm_Buff1{
               int isValid; //�Ƿ���Ч
               int SendNum; //��ǰ���ʹ���
               int m_Comm;
               unsigned char buf[1500];
               int nlength;
              };

//ͨ�����ݽṹ
struct talkdata1
  {
   char HostAddr[20];       //���з���ַ
   unsigned char HostIP[4]; //���з�IP��ַ
   char AssiAddr[20];       //���з���ַ
   unsigned char AssiIP[4]; //���з�IP��ַ
   unsigned int timestamp;  //ʱ���
   unsigned short DataType;          //��������
   unsigned short Frameno;           //֡���
   unsigned int Framelen;            //֡���ݳ���    
   unsigned short TotalPackage;      //�ܰ���
   unsigned short CurrPackage;       //��ǰ����
   unsigned short Datalen;           //���ݳ���
   unsigned short PackLen;       //���ݰ���С
  }__attribute__ ((packed));
//��Ϣ���ݽṹ
struct infodata1
  {
   char Addr[20];       //��ַ����
   unsigned short Type; //����
   unsigned int  Sn;         //���
   unsigned short Length;   //���ݳ���
  }__attribute__ ((packed));
struct downfile1
  {
   char FlagText[20];     //��־�ַ���
   char FileName[20];
   unsigned int Filelen;            //�ļ���С
   unsigned short TotalPackage;      //�ܰ���
   unsigned short CurrPackage;       //��ǰ����
   unsigned short Datalen;           //���ݳ���
  }__attribute__ ((packed));

struct PicInfo1{
                int Width;
                int Height;
                int X;
                int Y;
                };
struct LabelInfo1{
                int Width;
                int Height;
                int X;
                int Y;
                };
#ifndef CommonH
#define CommonH
  int DebugMode;           //����ģʽ
  struct flcd_data f_data;
  int             fbdev;
  unsigned char  *fbmem;
  unsigned int    fb_uvoffset;
  unsigned int    fb_width;
  unsigned int    fb_height;
  unsigned int    fb_depth;
  unsigned int    screensize;

  int DeltaLen;  //���ݰ���Ч����ƫ����

  struct PicInfo1 PicInfo[MAINPICNUM];      //��ҳͼ��
  struct PicInfo1 TmpPicInfo[MAINPICNUM];      //��ҳͼ��
  struct LabelInfo1 LabelInfo[MAINLABELNUM]; //��ҳLabel
  struct LabelInfo1 TmpLabelInfo[MAINLABELNUM]; //��ҳLabel
  //��ҳ�ļ���
  char nPic_Name[MAINPICNUM][20] = {"main.jpg", "logo.jpg", "menu1_up.jpg", "menu2_up.jpg", "menu3_up.jpg", "menu4_up.jpg", "menu5_up.jpg", "menu1_down.jpg",
                                       "menu2_down.jpg", "menu3_down.jpg", "menu4_down.jpg", "menu5_down.jpg", "weather1.jpg", "weather2.jpg", "weather3.jpg", "weather4.jpg", "weather5.jpg",
                                       "state1.jpg", "state2.jpg", "state3.jpg", "state4.jpg", "state5.jpg", "state6.jpg", "state7.jpg"
                                       };

  struct tm *curr_tm_t;
  struct TimeStamp1 TimeStamp;  //����ʱ���벥��ʱ�䣬ͬ����

  int temp_video_n;      //��Ƶ���ջ������
  TempVideoNode1 *TempVideoNode_h;    //��Ƶ���ջ����б�
  int temp_audio_n;      //��Ƶ���ջ������
  TempAudioNode1 *TempAudioNode_h;    //��Ƶ���ջ����б�

  //ϵͳ��ʼ����־
  int InitSuccFlag;
  //����״̬����
  struct Local1 Local;
  struct LocalCfg1 LocalCfg;

  //Զ�˵�ַ
  struct Remote1 Remote;
  char NullAddr[21];   //���ַ���
  //COMM
  int Comm2fd;  //����2���
  int Comm3fd;  //����3���
  int Comm4fd;  //����4���
  //���ARP
  int ARP_Socket;
  //�����������
  int m_EthSocket;
  //UDP
  int m_DataSocket;
  int m_VideoSocket;
  int LocalDataPort;   //�������UDP�˿�
  int LocalVideoPort;  //����ƵUDP�˿�
  int RemoteDataPort;
  int RemoteVideoPort;
  char RemoteHost[20];
  char sPath[80];
  char currpath[80];   //�Զ���·��  
  char wavPath[80];
  char UdpPackageHead[15];


  char CallListAddr[50];
  //״̬��ʾ��Ϣ����
  struct PicStatBuf1 PicStatBuf;
  //��������Ч����һ�̴߳���
  int key_press_flag;
  short key_press;
  //FLASH�洢�߳�
  int save_file_flag;
  pthread_t save_file_thread;
  void save_file_thread_func(void);
  sem_t save_file_sem;
  struct Save_File_Buff1 Save_File_Buff[SAVEMAX]; //FLASH�洢�������ֵ

  //�����������ݷ����̣߳��ն����������������ʱһ��û�յ���Ӧ�����η���
  //����UDP��Commͨ��
  int multi_send_flag;
  pthread_t multi_send_thread;
  void multi_send_thread_func(void);
  sem_t multi_send_sem;
  struct Multi_Udp_Buff1 Multi_Udp_Buff[UDPSENDMAX]; //10��UDP�������ͻ���

  //�����������ݷ����̣߳��ն����������������ʱһ��û�յ���Ӧ�����η���
  //����UDP��Commͨ��
  int multi_comm_send_flag;
  pthread_t multi_comm_send_thread;
  void multi_comm_send_thread_func(void);
  sem_t multi_comm_send_sem;
  struct Multi_Comm_Buff1 Multi_Comm_Buff[COMMSENDMAX]; //10��COMM�������ͻ���

  //watchdog
  int watchdog_fd;
  int InitParam;

  //gpio ����
  int gpio_fd;
  int gpio_rcv_flag;
  pthread_t gpio_rcv_thread;
  void gpio_rcv_thread_func(void);

	int CurrBox;
	struct TImage main_image;    //��ҳ����ͼ��  
	struct TImage main_hotkey_image;
	struct TImage hotkey_image[10];
	struct TImageButton menu_button[5]; //�˵�����ť
	struct TImageButton state_image[6];    //״̬��ͼ��
	struct TImage weather_image[12];
	struct TImage misscall_image[20];
	struct TImage missmessage_image[20];
	struct TImage news_image[20];
	struct TLabel label_hotkey;
	struct TImage clock_num_image[10];
	struct TImageButton bigmenu_button[6];
	//�Խ�����
	struct TImageButton talk_menu_button[5];
	struct TImageButton talk_keynum_button[12];
	struct TImageButton talk_hotkey_button[3];
	struct TImageButton talkpic_button[10];
	struct TEdit roomaddr_edit;
	struct TEdit talkaddr_edit;
	struct TImage talk_main_image;

	struct TImage talk_message_image[16];
	struct TImageButton talk_message_button[5];
	struct TImage talk_comkey[20];
	struct TImage talk_banner[10];
	struct TImage talk_bg[8];
	struct TImageButton talk_callrec_button[4];
	struct TImageButton talk_info_button[9];
	struct TImage talk_infocontent_comkey[4];
	struct TLabel label_talkinfo;
 
 	struct TImage AreaMessageComKey[5];
	struct TImage LocalMessageComKey[6];
	struct TImage PushMessageComKey[5];
	struct TImage CustomMessageComKey[5];
	struct TImage MisscallComKey[6];
	struct TImage CalledComKey[6];
	struct TImage CallComKey[6];
 
	struct TImageButton setup_menu_button[12];
	struct TImage setup_comkey_image[5];
	struct TImage setup_bg[12];
	struct TImageButton setup_screen_menu_button[3];
	struct TImageButton setup_upgrade_menu_button[2];
	struct TImageButton setup_keynum_button[15];
 
	struct TEdit setup_edit[4];
	struct TEdit SetupPass_edit;
	struct TEdit LanSet_edit[5];
	struct TEdit WlanSet_edit[8];
	struct TEdit RoomSet_edit[6];
	struct TEdit VoiceSet_edit;
	struct TEdit TimeSet_edit[3];
	struct TEdit PassSet_edit[3];
	struct TImage blank_image;

	struct TImage keypad_image[2];
	struct TImageButton keyboard_num[32];
	struct TImageButton keyboard_en[32];
	struct TLabel label_keypad;

	struct TLabel label_call;
 
#else
  extern int DebugMode;           //����ģʽ
  extern struct flcd_data f_data;
  extern int             fbdev;
  extern unsigned char  *fbmem;
  extern unsigned int    fb_uvoffset;
  extern unsigned int    fb_width;
  extern unsigned int    fb_height;
  extern unsigned int    fb_depth;
  extern unsigned int    screensize;

  extern int DeltaLen;  //���ݰ���Ч����ƫ����

  extern struct PicInfo1 PicInfo[MAINPICNUM];      //��ҳͼ��
  extern struct PicInfo1 TmpPicInfo[MAINPICNUM];      //��ҳͼ��
  extern struct LabelInfo1 LabelInfo[MAINLABELNUM]; //��ҳLabel
  extern struct LabelInfo1 TmpLabelInfo[MAINLABELNUM]; //��ҳLabel
  //��ҳ�ļ���
  extern char nPic_Name[MAINPICNUM][20];  

  extern struct tm *curr_tm_t;
  extern struct TimeStamp1 TimeStamp;  //����ʱ���벥��ʱ�䣬ͬ����  

  extern int temp_video_n;      //��Ƶ���ջ������
  extern TempVideoNode1 *TempVideoNode_h;    //��Ƶ���ջ����б�
  extern int temp_audio_n;      //��Ƶ���ջ������
  extern TempAudioNode1 *TempAudioNode_h;    //��Ƶ���ջ����б�

  //ϵͳ��ʼ����־
  extern int InitSuccFlag;  
  //����״̬����
  extern struct Local1 Local;
  extern struct LocalCfg1 LocalCfg;

  //Զ�˵�ַ
  extern struct Remote1 Remote;
  extern char NullAddr[21];   //���ַ���
  //COMM
  extern int Comm2fd;  //����2���
  extern int Comm3fd;  //����3���
  extern int Comm4fd;  //����4���
  //���ARP
  extern int ARP_Socket;
  //�����������
  int m_EthSocket;   
  //UDP
  extern int m_DataSocket;
  extern int m_VideoSocket;
  extern int LocalDataPort;   //�������UDP�˿�
  extern int LocalVideoPort;  //����ƵUDP�˿�
  extern int RemoteDataPort;
  extern int RemoteVideoPort;
  extern char RemoteHost[20];
  extern char sPath[80];
  extern char currpath[80];   //�Զ���·��
  extern char wavPath[80];
 extern char UdpPackageHead[15];
extern  char CallListAddr[50];
  //״̬��ʾ��Ϣ����
  extern struct PicStatBuf1 PicStatBuf;
  //��������Ч����һ�̴߳���
  extern int key_press_flag;
  extern short key_press;
  //FLASH�洢�߳�
  extern int save_file_flag;
  extern pthread_t save_file_thread;
  extern void save_file_thread_func(void);
  extern sem_t save_file_sem;
  extern struct Save_File_Buff1 Save_File_Buff[SAVEMAX]; //FLASH�洢�������ֵ

  //�����������ݷ����̣߳��ն����������������ʱһ��û�յ���Ӧ�����η���
  //����UDP��Commͨ��
  extern int multi_send_flag;
  extern pthread_t multi_send_thread;
  extern void multi_send_thread_func(void);
  extern sem_t multi_send_sem;
  extern struct Multi_Udp_Buff1 Multi_Udp_Buff[UDPSENDMAX]; //10��UDP�������ͻ���
  //�����������ݷ����̣߳��ն����������������ʱһ��û�յ���Ӧ�����η���
  //����UDP��Commͨ��
  extern int multi_comm_send_flag;
  extern pthread_t multi_comm_send_thread;
  extern void multi_comm_send_thread_func(void);
  extern sem_t multi_comm_send_sem;
  extern struct Multi_Comm_Buff1 Multi_Comm_Buff[COMMSENDMAX]; //10��COMM�������ͻ���

  //watchdog
  extern int watchdog_fd;  
 extern  int InitParam;

  //gpio ����
  	extern int gpio_fd;
  	extern int gpio_rcv_flag;
  	extern pthread_t gpio_rcv_thread;
  	extern void gpio_rcv_thread_func(void);

	extern int CurrBox;
  
  	extern struct TImage main_image;    //��ҳ����ͼ��  
	extern struct TImage main_hotkey_image;
	extern struct TImage hotkey_image[10];
	extern struct TImageButton menu_button[5]; //�˵�����ť
	extern struct TImageButton state_image[6];    //״̬��ͼ��
	extern struct TImage weather_image[12];
	extern struct TImage misscall_image[20];
	extern struct TImage missmessage_image[20];
	extern struct TImage news_image[20];
	extern struct TLabel label_hotkey;
	extern   struct TImage clock_num_image[10];
    	extern struct TImageButton bigmenu_button[6];
	  //�Խ�����
	extern   struct TImageButton talk_menu_button[5];
	extern   struct TImageButton talk_keynum_button[12];
	extern   struct TImageButton talk_hotkey_button[3];
	extern   struct TImageButton talkpic_button[10];
	extern   struct TEdit roomaddr_edit;
	extern   struct TEdit talkaddr_edit;
	extern   struct TImage talk_main_image;
	
	extern struct TImage talk_message_image[16];
	extern struct TImageButton talk_message_button[5];
	extern struct TImage talk_comkey[20];
	extern struct TImage talk_banner[10];
	extern struct TImage talk_bg[8];
	extern struct TImageButton talk_callrec_button[4];
	extern struct TImageButton talk_info_button[9];
	extern struct TImage talk_infocontent_comkey[4];

	extern struct TImage AreaMessageComKey[5];
	extern struct TImage LocalMessageComKey[6];
	extern struct TImage PushMessageComKey[5];
	extern struct TImage CustomMessageComKey[5];
	extern struct TImage MisscallComKey[6];
	extern struct TImage CalledComKey[6];
	extern struct TImage CallComKey[6];

	extern struct TImageButton setup_menu_button[12];
	extern struct TImage setup_comkey_image[5];
	extern struct TImage setup_bg[12];
	extern struct TImageButton setup_screen_menu_button[3];
	extern struct TImageButton setup_upgrade_menu_button[2];
	extern struct TImageButton setup_keynum_button[15];

	extern struct TEdit setup_edit[4];
	extern struct TEdit SetupPass_edit;
	extern struct TEdit LanSet_edit[5];
	extern struct TEdit WlanSet_edit[8];
	extern struct TEdit RoomSet_edit[6];
	extern struct TEdit VoiceSet_edit;
	extern struct TEdit TimeSet_edit[3];
	extern struct TEdit PassSet_edit[3];
	extern struct TLabel label_talkinfo;
	extern struct TImage blank_image;

	extern struct TImage keypad_image[2];
	extern struct TImageButton keyboard_num[32];
	extern struct TImageButton keyboard_en[32];
	extern struct TLabel label_keypad;
	extern struct TLabel label_call;
#endif
