//UDP
#include <stdio.h>
#include   <time.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <semaphore.h>       //sem_t
#include <dirent.h>

#define CommonH
#include "common.h"

enum BOOL {FALSE = 0,TRUE = !FALSE};
//UDP
int SndBufLen=1024*128;
int RcvBufLen=1024*128;

short UdpRecvFlag;
pthread_t udpdatarcvid;
pthread_t udpvideorcvid;
int InitUdpSocket(short lPort);
void CloseUdpSocket(void);
int UdpSendBuff(int m_Socket, char *RemoteHost, unsigned char *buf,int nlength);
void CreateUdpDataRcvThread(void);
void CreateUdpVideoRcvThread(void);
void UdpDataRcvThread(void);  //UDP���ݽ����̺߳���
void UdpVideoRcvThread(void);  //UDP����Ƶ�����̺߳���
void AddMultiGroup(int m_Socket, char *McastAddr);  //�����鲥��
void DropMultiGroup(int m_Socket, char *McastAddr);  //�˳��鲥��
void DropNsMultiGroup(int m_Socket, char *McastAddr);  //�˳�NS�鲥��
void RefreshNetSetup(int cType); //ˢ����������  0 -- δ����  1 -- ������

extern sem_t audiorec2playsem;

extern sem_t videorec2playsem;

int AudioMuteFlag;   //������־

extern struct _SYNC sync_s;

//�����߳��ڲ�������
//����
void RecvAlarm_Func(unsigned char *recv_buf, char *cFromIP, int length, int m_Socket);
//ȡ������
void RecvCancelAlarm_Func(unsigned char *recv_buf, char *cFromIP, int length, int m_Socket);
//��Ϣ
void RecvMessage_Func(unsigned char *recv_buf, char *cFromIP, int length, int m_Socket);
//�豸��ʱ����״̬
void RecvReportStatus_Func(unsigned char *recv_buf, char *cFromIP, int length, int m_Socket);
//�������Ĳ�ѯ�豸״̬
void RecvQueryStatus_Func(unsigned char *recv_buf, char *cFromIP, int length, int m_Socket);
//Զ�̲���
void RecvRemoteDefence_Func(unsigned char *recv_buf, char *cFromIP, int length, int m_Socket);
//��λ����
void RecvResetPass_Func(unsigned char *recv_buf, char *cFromIP, int length, int m_Socket);
//д��ַ����
void RecvWriteAddress_Func(unsigned char *recv_buf, char *cFromIP, int length, int m_Socket);
//����ַ����
void RecvReadAddress_Func(unsigned char *recv_buf, char *cFromIP, int length, int m_Socket);
//д���ڻ���������
void RecvWriteRoomSetup_Func(unsigned char *recv_buf, char *cFromIP, int length, int m_Socket);
//�����ڻ���������
void RecvReadRoomSetup_Func(unsigned char *recv_buf, char *cFromIP, int length, int m_Socket);

//����
void RecvNSAsk_Func(unsigned char *recv_buf, char *cFromIP, int m_Socket);  //��������
void RecvNSReply_Func(unsigned char *recv_buf, char *cFromIP, int m_Socket);//����Ӧ��
//����
void RecvWatchCall_Func(unsigned char *recv_buf, char *cFromIP);  //���Ӻ���
void RecvWatchLineUse_Func(unsigned char *recv_buf, char *cFromIP);  //����ռ��Ӧ��
void RecvWatchCallAnswer_Func(unsigned char *recv_buf, char *cFromIP);  //���Ӻ���Ӧ��
void RecvWatchCallConfirm_Func(unsigned char *recv_buf, char *cFromIP);  //ͨ������ȷ��
void RecvWatchCallEnd_Func(unsigned char *recv_buf, char *cFromIP);  //���Ӻ��н���
void RecvWatchZoomOut_Func(unsigned char *recv_buf, char *cFromIP);  //�Ŵ�(720*480)
void RecvWatchZoomIn_Func(unsigned char *recv_buf, char *cFromIP);  //��С(352*240)
void RecvWatchCallUpDown_Func(unsigned char *recv_buf, char *cFromIP, int length);  //��������
//�Խ�
void RecvTalkCall_Func(unsigned char *recv_buf, char *cFromIP);  //�Խ�����
void ExitGroup(unsigned char *buf);      //���������з��˳��鲥������
void RecvTalkLineUse_Func(unsigned char *recv_buf, char *cFromIP);  //�Խ�ռ��Ӧ��
void RecvTalkCallAnswer_Func(unsigned char *recv_buf, char *cFromIP);  //�Խ�����Ӧ��
void RecvTalkCallConfirm_Func(unsigned char *recv_buf, char *cFromIP); //�Խ�����ȷ��
void RecvTalkCallAsk_Func(unsigned char *recv_buf, char *cFromIP);  //�Խ���ʼͨ��ѯ��
void RecvTalkCallStart_Func(unsigned char *recv_buf, char *cFromIP);  //�Խ���ʼͨ��
void RecvTalkCallEnd_Func(unsigned char *recv_buf, char *cFromIP);  //�Խ����н���
void RecvTalkRemoteOpenLock_Func(unsigned char *recv_buf, char *cFromIP);  //Զ�̿���
void RecvTalkZoomOut_Func(unsigned char *recv_buf, char *cFromIP);  //�Ŵ�(720*480)
void RecvTalkZoomIn_Func(unsigned char *recv_buf, char *cFromIP);  //��С(352*240)
void RecvTalkCallUpDown_Func(unsigned char *recv_buf, char *cFromIP, int length);  //�Խ�����
void TalkEnd_ClearStatus(void); //�Խ���������״̬�͹ر�����Ƶ
void ForceIFrame_Func(void);  //ǿ��I֡
//�����븱��
void RecvSyncSub_Func(unsigned char *recv_buf, char *cFromIP);  //Ϊ��������ʱ�븱��ͬ��״̬
//�����豸
void RecvFindEquip_Func(unsigned char *recv_buf, char *cFromIP, int length, int m_Socket);

//��Ϣ����
// *h�����ͷ����ָ�룬*pָ��ǰ����ǰһ����㣬*sָ��ǰ���
extern InfoNode1 *InfoNode_h[INFOTYPENUM];
//����Ϣ�����ṹ
extern struct Info1 Info[INFOTYPENUM];
//��ǰ��Ϣ����״̬
extern struct InfoStatus1 InfoStatus;

int downbuflen;
char downip[20];
unsigned char *downbuf;
int download_image_flag;
pthread_t download_image_thread;      //����ϵͳӳ���߳�
void download_image_thread_func(void);
int downloaded_flag[2000]; //�����ر�־
int OldPackage;
//����Ӧ�ó���
void RecvDownLoadFile_Func(unsigned char *recv_buf, char *cFromIP, int length, int m_Socket);
//����ϵͳӳ��
void RecvDownLoadImage_Func(unsigned char *recv_buf, char *cFromIP, int length, int m_Socket);
//---------------------------------------------------------------------------
int InitUdpSocket(short lPort)
{
  struct sockaddr_in s_addr;
  int  nZero=0;
  int  iLen;
  int m_Socket;
  int  nYes;
  int ret;

  /* ���� socket , �ؼ�������� SOCK_DGRAM */
  if ((m_Socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
   {
    perror("Create socket error\r\n");
    return 0;
   }
  else
    printf("create socket.\n\r");

  if(m_EthSocket == 0)
    m_EthSocket = socket(AF_INET, SOCK_DGRAM, 0);

  memset(&s_addr, 0, sizeof(struct sockaddr_in));
  /* ���õ�ַ�Ͷ˿���Ϣ */
  s_addr.sin_family = AF_INET;
  s_addr.sin_port = htons(lPort);
  s_addr.sin_addr.s_addr = INADDR_ANY;//inet_addr(LocalIP);//INADDR_ANY;

  iLen=sizeof(nZero);           //  SO_SNDBUF
  nZero=SndBufLen;       //128K
  setsockopt(m_Socket,SOL_SOCKET,SO_SNDBUF,(char*)&nZero,sizeof((char*)&nZero));
  nZero=RcvBufLen;       //128K
  setsockopt(m_Socket,SOL_SOCKET,SO_RCVBUF,(char*)&nZero,sizeof((char*)&nZero));

  //����Ƶ�˿ڣ������ͺͽ��չ㲥
  if(lPort == LocalVideoPort)
   {
    nYes = 1;
    if (setsockopt(m_Socket, SOL_SOCKET, SO_BROADCAST, (char *)&nYes, sizeof((char *)&nYes))== -1)
     {
      printf("set broadcast error.\n\r");
      return 0;
     }
   }

  /* �󶨵�ַ�Ͷ˿���Ϣ */
  if ((bind(m_Socket, (struct sockaddr *) &s_addr, sizeof(s_addr))) == -1)
   {
    perror("bind error");
    return 0;
   }
  else
    printf("bind address to socket.\n\r");
  if(lPort == LocalDataPort)
   {
    m_DataSocket = m_Socket;
    //����UDP�����߳�
    CreateUdpDataRcvThread();
   }
  if(lPort == LocalVideoPort)
   {
    m_VideoSocket = m_Socket;
    //����UDP�����߳�
    CreateUdpVideoRcvThread();
   }
  return 1;
}
//---------------------------------------------------------------------------
void CloseUdpSocket(void)
{
  UdpRecvFlag = 0;
  pthread_cancel(udpdatarcvid);
  pthread_cancel(udpvideorcvid);
  close(m_DataSocket);
  close(m_VideoSocket);
}
//---------------------------------------------------------------------------
int UdpSendBuff(int m_Socket, char *RemoteHost, unsigned char *buf,int nlength)
{
  struct sockaddr_in To;
  int nSize;
  To.sin_family=AF_INET;
  if(m_Socket == m_DataSocket)
    To.sin_port=htons(RemoteDataPort);
  if(m_Socket == m_VideoSocket)
    To.sin_port=htons(RemoteVideoPort);
  To.sin_addr.s_addr = inet_addr(RemoteHost);
  nSize=sendto(m_Socket,buf,nlength,0,(struct sockaddr*)&To,sizeof(struct sockaddr));
  return nSize;
}
//---------------------------------------------------------------------------
void CreateUdpDataRcvThread()
{
  int i,ret;
  pthread_attr_t attr;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  ret=pthread_create(&udpdatarcvid, &attr, (void *)UdpDataRcvThread, NULL);
  pthread_attr_destroy(&attr);  
  #ifdef _DEBUG
    printf ("Create UDP data pthread!\n");
  #endif
  if(ret!=0){
    printf ("Create data pthread error!\n");
  }
}
//---------------------------------------------------------------------------
void CreateUdpVideoRcvThread()
{
  pthread_attr_t attr;
  int i,ret;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  ret=pthread_create(&udpvideorcvid, &attr, (void *)UdpVideoRcvThread, NULL);
  pthread_attr_destroy(&attr);
  #ifdef _DEBUG
    printf ("Create UDP video pthread!\n");
  #endif
  if(ret!=0){
    printf ("Create video pthread error!\n");
  }
}
//---------------------------------------------------------------------------
void AddMultiGroup(int m_Socket, char *McastAddr)  //�����鲥��
{
// Winsock1.0
  struct ip_mreq mcast; // Winsock1.0
  mcast.imr_multiaddr.s_addr = inet_addr(McastAddr);
  mcast.imr_interface.s_addr = INADDR_ANY;
  if( setsockopt(m_Socket,IPPROTO_IP,IP_ADD_MEMBERSHIP,(char*)&mcast,sizeof(mcast)) == -1)
  {
      printf("set multicast error.\n\r");
      return;
  }
}
//---------------------------------------------------------------------------
void DropMultiGroup(int m_Socket, char *McastAddr)  //�˳��鲥��
{
// Winsock1.0
/*  struct ip_mreq mcast; // Winsock1.0
  char IP_Group[20];
  //�鿴�Ƿ��������鲥����
  if(Local.IP_Group[0] != 0)
   {
    sprintf(IP_Group, "%d.%d.%d.%d\0",
            Local.IP_Group[0],Local.IP_Group[1],Local.IP_Group[2],Local.IP_Group[3]);
    Local.IP_Group[0] = 0; //�鲥��ַ
    Local.IP_Group[1] = 0;
    Local.IP_Group[2] = 0;
    Local.IP_Group[3] = 0;
    //  memset(&mcast, 0, sizeof(struct ip_mreq));
    mcast.imr_multiaddr.s_addr = inet_addr(IP_Group);
    mcast.imr_interface.s_addr = INADDR_ANY;
    if( setsockopt(m_Socket,IPPROTO_IP,IP_DROP_MEMBERSHIP,(char*)&mcast,sizeof(mcast)) == -1)
     {
      printf("drop multicast error.\n\r");
      return;
     }
   }    */

}
//---------------------------------------------------------------------------
void DropNsMultiGroup(int m_Socket, char *McastAddr)  //�˳�NS�鲥��
{
// Winsock1.0
  struct ip_mreq mcast; // Winsock1.0

    //  memset(&mcast, 0, sizeof(struct ip_mreq));
    mcast.imr_multiaddr.s_addr = inet_addr(McastAddr);
    mcast.imr_interface.s_addr = INADDR_ANY;
    if( setsockopt(m_Socket,IPPROTO_IP,IP_DROP_MEMBERSHIP,(char*)&mcast,sizeof(mcast)) == -1)
     {
      printf("drop multicast error.\n\r");
      return;
     }

}
//---------------------------------------------------------------------------
void RefreshNetSetup(int cType) //ˢ����������  0 -- δ����  1 -- ������
{

  char SystemOrder[100];
  #if 1
  //����������
  pthread_mutex_lock (&Local.udp_lock);
  //�˳�NS�鲥��
  if(cType == 1)
   {
    DropNsMultiGroup(m_VideoSocket, NSMULTIADDR);
   }
  //����MAC��ַ
  system("ifconfig eth0 down");
  sprintf(SystemOrder, "ifconfig eth0 hw ether %02X:%02X:%02X:%02X:%02X:%02X\0",
                 LocalCfg.Mac_Addr[0], LocalCfg.Mac_Addr[1], LocalCfg.Mac_Addr[2],
                 LocalCfg.Mac_Addr[3], LocalCfg.Mac_Addr[4], LocalCfg.Mac_Addr[5]);
  system(SystemOrder);
  system("ifconfig eth0 up");
  //����IP��ַ����������
  sprintf(SystemOrder, "ifconfig eth0 %d.%d.%d.%d netmask %d.%d.%d.%d\0",
                 LocalCfg.IP[0], LocalCfg.IP[1], LocalCfg.IP[2], LocalCfg.IP[3],
                 LocalCfg.IP_Mask[0], LocalCfg.IP_Mask[1], LocalCfg.IP_Mask[2], LocalCfg.IP_Mask[3]);
  system(SystemOrder);
  //��������
  sprintf(SystemOrder, "route add default gw %d.%d.%d.%d\0",
                 LocalCfg.IP_Gate[0], LocalCfg.IP_Gate[1], LocalCfg.IP_Gate[2], LocalCfg.IP_Gate[3]);
  system(SystemOrder);
  //����NS�鲥��
  if(cType == 1)
   {
    AddMultiGroup(m_VideoSocket, NSMULTIADDR);
   }
  //�򿪻�����
  pthread_mutex_unlock (&Local.udp_lock);
#endif
}
//---------------------------------------------------------------------------
void UdpDataRcvThread(void)  //UDP�����̺߳���
{
  /* ѭ���������� */
//  int oldframeno=0;
  unsigned char send_b[1520];
  int sendlength;
  char FromIP[20];
  int newframeno;
  int currpackage;
  int i,j;
  int sub;
  short PackIsExist; //���ݰ��ѽ��ձ�־
  short FrameIsNew;  //���ݰ��Ƿ�����֡�Ŀ�ʼ
  struct sockaddr_in c_addr;
  socklen_t addr_len;
  int len;
  int tmp;
  unsigned char buff[8096];

  char tmpAddr[21];
  int isAddrOK;
  #ifdef _DEBUG
    printf("This is udp pthread.\n");
  #endif
  UdpRecvFlag = 1;

  addr_len = sizeof(c_addr);
  while (UdpRecvFlag == 1)
   {
    len = recvfrom(m_DataSocket, buff, sizeof(buff) - 1, 0,
     (struct sockaddr *) &c_addr, &addr_len);
    if (len < 0)
     {
      perror("recvfrom");
     }
    buff[len] = '\0';
    strcpy(FromIP, inet_ntoa(c_addr.sin_addr));
    if((buff[0]==UdpPackageHead[0])&&(buff[1]==UdpPackageHead[1])&&(buff[2]==UdpPackageHead[2])
      &&(buff[3]==UdpPackageHead[3])&&(buff[4]==UdpPackageHead[4])&&(buff[5]==UdpPackageHead[5]))
     {
      switch(buff[6])
       {
        case ALARM:   //����
                if(len == 41)
                 {
                  RecvAlarm_Func(buff, FromIP, len, m_DataSocket);
                 }
                else
                 {
                  #ifdef _DEBUG
                    printf("����Ӧ�𳤶��쳣\n");
                  #endif
                 }
                break;
        case CANCELALARM:   //ȡ������
                if(len == 30)
                 {
                  RecvCancelAlarm_Func(buff, FromIP, len, m_DataSocket);
                 }
                else
                 {
                  #ifdef _DEBUG
                    printf("ȡ������Ӧ�𳤶��쳣\n");
                  #endif
                 }
                break;
        case SENDMESSAGE: //��Ϣ
                RecvMessage_Func(buff, FromIP, len, m_DataSocket);
                break;
        case REPORTSTATUS:   //�豸��ʱ����״̬
                if(len == 40)
                 {
                  RecvReportStatus_Func(buff, FromIP, len, m_DataSocket);
                 }
                else
                 {
                  #ifdef _DEBUG
                    printf("�豸��ʱ����״̬Ӧ�𳤶��쳣\n");
                  #endif
                 }
                break;
        case QUERYSTATUS:   //�������Ĳ�ѯ�豸״̬
                if(len == 40)
                 {
                  RecvQueryStatus_Func(buff, FromIP, len, m_DataSocket);
                 }
                else
                 {
                  #ifdef _DEBUG
                    printf("�������Ĳ�ѯ�豸״̬�����쳣\n");
                  #endif
                 }
                break;                
        case REMOTEDEFENCE:   //Զ�̲���
                if(len == 29)
                 {
                  RecvRemoteDefence_Func(buff, FromIP, len, m_DataSocket);
                 }
                else
                 {
                  #ifdef _DEBUG
                    printf("Զ�̲��������쳣\n");
                  #endif
                 }
                break;
        case RESETPASS:   //��λ����
                if(len == 29)
                 {
                  RecvResetPass_Func(buff, FromIP, len, m_DataSocket);
                 }
                else
                 {
                  #ifdef _DEBUG
                    printf("��λ���볤���쳣\n");
                  #endif
                 }
                break;
        case WRITEADDRESS:   //д��ַ����
                if(len == 72)
                 {
                  RecvWriteAddress_Func(buff, FromIP, len, m_DataSocket);
                 }
                else
                 {
                  #ifdef _DEBUG
                    printf("д��ַ���ó����쳣\n");
                  #endif
                 }
                break;
        case READADDRESS:   //����ַ����
                if(len == 28)
                 {
                  RecvReadAddress_Func(buff, FromIP, len, m_DataSocket);
                 }
                else
                 {
                  #ifdef _DEBUG
                    printf("����ַ���ó����쳣\n");
                  #endif
                 }
                break;
        case WRITEROOMSETUP:   //д���ڻ���������
                break;
        case READROOMSETUP:   //�����ڻ���������
                break;
        //        case NSOrder:   //�����������������ڹ㲥��
        case NSSERVERORDER:  //����������(NS������)
                switch(buff[7])
                 {
                  case 1://����
                         if(len == 56)
                          {
                           RecvNSAsk_Func(buff, FromIP, m_DataSocket);
                          }
                         else
                          {
                           #ifdef _DEBUG
                             printf("�����������ݳ����쳣\n");
                           #endif
                          }
                         break;
                  case 2://������Ӧ
                         if(len >= 57)
                          {
                           RecvNSReply_Func(buff, FromIP, m_DataSocket);
                          }
                         else
                          {
                           #ifdef _DEBUG
                             printf("����Ӧ�����ݳ����쳣\n");
                           #endif
                          }
                         break;
                 }
                break;
        case DOWNLOADFILE:   //����Ӧ�ó���
                if(len >= (9 + sizeof(struct downfile1)))
                 {
                  RecvDownLoadFile_Func(buff, FromIP, len, m_DataSocket);
                 }
                else
                 {
                  #ifdef _DEBUG
                    printf("����Ӧ�ó��򳤶��쳣\n");
                  #endif
                 }
                break;
        case DOWNLOADIMAGE:   //����ϵͳӳ��
                if(len >= (9 + sizeof(struct downfile1)))
                 {
                  RecvDownLoadImage_Func(buff, FromIP, len, m_DataSocket);
                 }
                else
                 {
                  #ifdef _DEBUG
                    printf("����ϵͳӳ�񳤶��쳣\n");
                  #endif
                 }
                break;
        }
      }
    if(strcmp(buff,"exit")==0)
     {
      printf("recvfrom888888888");
      UdpRecvFlag=0;
     }
   }
}
//---------------------------------------------------------------------------
//����
void RecvAlarm_Func(unsigned char *recv_buf, char *cFromIP, int length, int m_Socket)
{
  int i,j;
  int newlength;
  int isAddrOK;
  unsigned char send_b[1520];
  int sendlength;
  unsigned char AlarmByte;
  unsigned char tmp_p;
  //ʱ��
  time_t t;

  i = 0;
  isAddrOK = 1;
  for(j=8; j<8+Local.AddrLen; j++)
   if(LocalCfg.Addr[j-8] != recv_buf[j])
    {
     isAddrOK = 0;
     break;
    }
  //��ַƥ��
  if(isAddrOK == 1)
   {
    //����������
    pthread_mutex_lock (&Local.udp_lock);
    if(recv_buf[7] == REPLY)   //Ӧ��
     for(i=0; i<UDPSENDMAX; i++)
      if(Multi_Udp_Buff[i].isValid == 1)
       if(Multi_Udp_Buff[i].m_Socket == m_DataSocket)
        if(Multi_Udp_Buff[i].SendNum  < MAXSENDNUM)
         if(Multi_Udp_Buff[i].buf[6] == ALARM)
          if(Multi_Udp_Buff[i].buf[7] == ASK)
            if(strcmp(Multi_Udp_Buff[i].RemoteHost, cFromIP) == 0)
             {
              Multi_Udp_Buff[i].isValid = 0;
              AlarmByte = (recv_buf[37] & 0x3F);
              tmp_p = 0x01;
              for(j=0; j<6; j++)
               {
                if((AlarmByte & tmp_p) == tmp_p)
                  LocalCfg.DefenceInfo[j][3] = 2; //�����ѽ���
                tmp_p = (tmp_p << 1);
               }
              #ifdef _DEBUG
                printf("�յ�����״̬Ӧ��\n");
              #endif
              break;
             }
    //�򿪻�����
    pthread_mutex_unlock (&Local.udp_lock);
   }
}
//---------------------------------------------------------------------------
//ȡ������
void RecvCancelAlarm_Func(unsigned char *recv_buf, char *cFromIP, int length, int m_Socket)
{
  int i,j;
  int newlength;
  int isAddrOK;
  unsigned char send_b[1520];
  int sendlength;
  //ʱ��
  time_t t;

  i = 0;
  isAddrOK = 1;
  for(j=8; j<8+Local.AddrLen; j++)
   if(LocalCfg.Addr[j-8] != recv_buf[j])
    {
     isAddrOK = 0;
     break;
    }
  //��ַƥ��
  if(isAddrOK == 1)
   {
    //����������
    pthread_mutex_lock (&Local.udp_lock);
    if(recv_buf[7] == REPLY)   //Ӧ��
     for(i=0; i<UDPSENDMAX; i++)
      if(Multi_Udp_Buff[i].isValid == 1)
       if(Multi_Udp_Buff[i].m_Socket == m_DataSocket)
        if(Multi_Udp_Buff[i].SendNum  < MAXSENDNUM)
         if(Multi_Udp_Buff[i].buf[6] == CANCELALARM)
          if(Multi_Udp_Buff[i].buf[7] == ASK)
            if(strcmp(Multi_Udp_Buff[i].RemoteHost, cFromIP) == 0)
             {
              Multi_Udp_Buff[i].isValid = 0;

              #ifdef _DEBUG
                printf("�յ�����״̬Ӧ��\n");
              #endif
              break;
             }
    //�򿪻�����
    pthread_mutex_unlock (&Local.udp_lock);
   }
}
//---------------------------------------------------------------------------
//��Ϣ
void RecvMessage_Func(unsigned char *recv_buf, char *cFromIP, int length, int m_Socket)
{
	struct InfoContent1 InitInfoCon;
	int i,j;
	int newlength;
	int isAddrOK;
	InfoNode1 *tmp_node;
	//ʱ��
	time_t t;
	struct tm *tm_t;
	unsigned char send_b[1520];
	int sendlength;
	//��Ϣ���ݽṹ
	struct infodata1 infodata;

	printf("recv data\n");
	i = 0;
	isAddrOK = 1;

	for(j=8; j<8+Local.AddrLen; j++)
	if(LocalCfg.Addr[j-8] != recv_buf[j])
	{
		isAddrOK = 0;
		break;
	}
	//��ַƥ��
	if(isAddrOK == 1)
	{
		memcpy(send_b, recv_buf, length);
		send_b[7] = REPLY;    //Ӧ��
		sendlength = length;
		UdpSendBuff(m_Socket, cFromIP, send_b , sendlength);

		memcpy(&infodata, recv_buf + 8, sizeof(infodata));
		newlength = infodata.Length + 36;

		//ֻ������ͨ��Ϣ  
		if((length == newlength)&&(infodata.Type == 1))
		{
			InitInfoCon.isValid = 1;
			InitInfoCon.isReaded = 0;
			InitInfoCon.isLocked = 0;
			time(&t);
			tm_t=localtime(&t);
			sprintf(InitInfoCon.Time, "%04d-%02d-%02d %02d:%02d:%02d\0", tm_t->tm_year + 1900, tm_t->tm_mon+1,
				tm_t->tm_mday, tm_t->tm_hour, tm_t->tm_min, tm_t->tm_sec);
			InitInfoCon.Type = infodata.Type;
			InitInfoCon.Sn = infodata.Sn;
			InitInfoCon.Length = infodata.Length;
			memcpy(InitInfoCon.Content, recv_buf + 36, InitInfoCon.Length);
			InitInfoCon.Content[InitInfoCon.Length] = '\0';
			printf("InitInfoCon.Type = %d\n", InitInfoCon.Type);
	//����ϢΪ0 ��Ϊ���

		if(Info[InitInfoCon.Type-1].TotalNum >= Info[InitInfoCon.Type-1].MaxNum)//ɾ��δ���������һ������ͷ������һ��
		{
			for(j = Info[InitInfoCon.Type-1].TotalNum; j >= 1; j --)
			{
				tmp_node=locate_infonode(InfoNode_h[InitInfoCon.Type-1], j);
				if(tmp_node->Content.isLocked == 0)
				{
					delete_infonode(tmp_node);
					Info[InitInfoCon.Type-1].TotalNum --;
					break;
				}
			}
		}

		//����Ϣδ���������������뵽ͷ��
		if(Info[InitInfoCon.Type-1].TotalNum < Info[InitInfoCon.Type-1].MaxNum)
		{
			tmp_node=locate_infonode(InfoNode_h[InitInfoCon.Type-1], 1);
			insert_infonode(InfoNode_h[InitInfoCon.Type-1], tmp_node, InitInfoCon);
			Info[InitInfoCon.Type-1].TotalNum ++;
		}
		//��������
		Info[InitInfoCon.Type-1].TotalNum = length_infonode(InfoNode_h[InitInfoCon.Type-1]);
		Info[InitInfoCon.Type-1].NoReadedNum = length_infonoreaded(InfoNode_h[InitInfoCon.Type-1]);

		//�統ǰΪ��Ϣ���ڣ�ˢ����Ļ
		if(Local.CurrentWindow == TalkAreaMessageWindow)
		{
			printf("��Ҫˢ�±�����!\n");
			ShowInfoList(InitInfoCon.Type-1);
			//     ShowInfoNum(InitInfoCon.Type-1);  //��ʾ��Ϣ������δ����Ϣ����
			//     if((InfoStatus.CurrType == (InitInfoCon.Type - 1))&&(InfoStatus.CurrWin == 0))
			//ShowInfoList(InitInfoCon.Type-1);
		}
		//����������
		pthread_mutex_lock (&Local.save_lock);
		//���ҿ��ô洢���岢���
		for(i=0; i<SAVEMAX; i++)
		if(Save_File_Buff[i].isValid == 0)
		{
			Save_File_Buff[i].Type = 1;
			Save_File_Buff[i].InfoType = InitInfoCon.Type-1;
			Save_File_Buff[i].isValid = 1;
			sem_post(&save_file_sem);
			break;
		}

		//�򿪻�����
		pthread_mutex_unlock (&Local.save_lock);
		//  WriteInfoFile(InitInfoCon.Type-1);
		
		if(Local.CurrentWindow == MainWindow)
		{
			if(Info[0].NoReadedNum >= 10)
				DisplayImage(&missmessage_image[9],0);
			else if(Info[0].NoReadedNum > 0)
				DisplayImage(&missmessage_image[Info[0].NoReadedNum -1],0);
			else
				DisplayImage(&missmessage_image[10],0);
		}
	}
	else
	{
#ifdef _DEBUG
	printf("��Ϣ���ݳ��ȴ�������Ͳ���\n");
#endif
	}
	}
}
//---------------------------------------------------------------------------
//�豸��ʱ����״̬
void RecvReportStatus_Func(unsigned char *recv_buf, char *cFromIP, int length, int m_Socket)
{
	int i,j;
	int newlength;
	int isAddrOK;
	unsigned char send_b[1520];
	int sendlength;
	//ʱ��
	time_t t;

	i = 0;
	isAddrOK = 1;
	for(j=8; j<8+Local.AddrLen; j++)
	if(LocalCfg.Addr[j-8] != recv_buf[j])
	{
		isAddrOK = 0;
		break;
	}
	//��ַƥ��
	if(isAddrOK == 1)
	{
		//����������
		pthread_mutex_lock (&Local.udp_lock);
		if(recv_buf[7] == REPLY)   //Ӧ��
		for(i=0; i<UDPSENDMAX; i++)
		if(Multi_Udp_Buff[i].isValid == 1)
		if(Multi_Udp_Buff[i].m_Socket == m_DataSocket)
		if(Multi_Udp_Buff[i].SendNum  < MAXSENDNUM)
		if(Multi_Udp_Buff[i].buf[6] == REPORTSTATUS)
		if(Multi_Udp_Buff[i].buf[7] == ASK)
		if(strcmp(Multi_Udp_Buff[i].RemoteHost, cFromIP) == 0)
		{
			Multi_Udp_Buff[i].isValid = 0;
			if(((recv_buf[29] << 8) + recv_buf[28]) != LocalCfg.ReportTime)
			{
				LocalCfg.ReportTime = (recv_buf[29] << 8) + recv_buf[28];
				SaveToFlash(4);    //��Flash�д洢�ļ�
			}
			//У׼ʱ��
			#if 0
			if(((curr_tm_t->tm_year + 1900) != ((recv_buf[30] << 8) + recv_buf[31]))
				||((curr_tm_t->tm_mon+1) != recv_buf[32])
				||(curr_tm_t->tm_mday != recv_buf[33])
				||(curr_tm_t->tm_hour != recv_buf[34])
				||(curr_tm_t->tm_min != recv_buf[35]))
			{
				curr_tm_t->tm_year   = (recv_buf[30] << 8) + recv_buf[31] - 1900;
				curr_tm_t->tm_mon   =   recv_buf[32] - 1;
				curr_tm_t->tm_mday   =   recv_buf[33];
				curr_tm_t->tm_hour   =   recv_buf[34];
				curr_tm_t->tm_min   = recv_buf[35];
				curr_tm_t->tm_sec   =   recv_buf[36];
				t=mktime(curr_tm_t);
				stime((long*)&t);
				sprintf(Label_Clock.Text, "%02d:%02d\0",  curr_tm_t->tm_hour, curr_tm_t->tm_min);
				if(Local.CurrentWindow == 0)
				ShowClock(&Label_Clock, REFRESH);
			}
			#endif
		//����Ԥ��
			if((Local.Weather[0] != recv_buf[37])||
				(Local.Weather[1] != recv_buf[38])||
				(Local.Weather[2] != recv_buf[39]))
			{
				Local.Weather[0] = recv_buf[37];
				Local.Weather[1] = recv_buf[38];
				Local.Weather[2] = recv_buf[39];
				//����Ԥ��
				if(Local.CurrentWindow == MainWindow)
					ShowWeather();
			}
			break;
		}
		//�򿪻�����
		pthread_mutex_unlock (&Local.udp_lock);
	}
}
//---------------------------------------------------------------------------
//�������Ĳ�ѯ�豸״̬
void RecvQueryStatus_Func(unsigned char *recv_buf, char *cFromIP, int length, int m_Socket)
{
	int i,j,k;
	int newlength;
	int isAddrOK;
	unsigned char send_b[1520];
	int sendlength;
	//ʱ��
	time_t t;

	i = 0;
	isAddrOK = 1;
	for(j=8; j<8+Local.AddrLen; j++)
	if(LocalCfg.Addr[j-8] != recv_buf[j])
	{
		isAddrOK = 0;
		break;
	}
	//��ַƥ��
	if(isAddrOK == 1)
	{
		//ͷ��
		memcpy(send_b, UdpPackageHead, 6);
		//����
		send_b[6] = QUERYSTATUS;
		send_b[7] = ASK;    //����
		memcpy(send_b + 8, LocalCfg.Addr, 20);
		memcpy(send_b + 28, LocalCfg.Mac_Addr, 6);
		send_b[34] = LocalCfg.DefenceStatus;
		send_b[35] = LocalCfg.DefenceNum;
		for(k=0; k<(LocalCfg.DefenceNum*6); k++)
		memcpy(send_b + 36 + 10*k, LocalCfg.DefenceInfo[k], 10);
		sendlength = 36 + LocalCfg.DefenceNum*6*10;
		UdpSendBuff(m_Socket, cFromIP, send_b , sendlength);

		if(((recv_buf[29] << 8) + recv_buf[28]) != LocalCfg.ReportTime)
		{
			LocalCfg.ReportTime = (recv_buf[29] << 8) + recv_buf[28];
			SaveToFlash(4);    //��Flash�д洢�ļ�
		}
		#if 0
		//У׼ʱ��
		if(((curr_tm_t->tm_year + 1900) != ((recv_buf[30] << 8) + recv_buf[31]))
		||((curr_tm_t->tm_mon+1) != recv_buf[32])
		||(curr_tm_t->tm_mday != recv_buf[33])
		||(curr_tm_t->tm_hour != recv_buf[34])
		||(curr_tm_t->tm_min != recv_buf[35]))
		//          ||(curr_tm_t->tm_sec != recv_buf[36]))
		{
		curr_tm_t->tm_year   = (recv_buf[30] << 8) + recv_buf[31] - 1900;
		curr_tm_t->tm_mon   =   recv_buf[32] - 1;
		curr_tm_t->tm_mday   =   recv_buf[33];
		curr_tm_t->tm_hour   =   recv_buf[34];
		curr_tm_t->tm_min   = recv_buf[35];
		curr_tm_t->tm_sec   =   recv_buf[36];
		t=mktime(curr_tm_t);
		stime((long*)&t);
		sprintf(Label_Clock.Text, "%02d:%02d\0",  curr_tm_t->tm_hour, curr_tm_t->tm_min);
		ShowClock(&Label_Clock, REFRESH);
		}
		#endif
		//����Ԥ��
		if((Local.Weather[0] != recv_buf[37])||
		(Local.Weather[1] != recv_buf[38])||
		(Local.Weather[2] != recv_buf[39]))
		{
			Local.Weather[0] = recv_buf[37];
			Local.Weather[1] = recv_buf[38];
			Local.Weather[2] = recv_buf[39];
			if(Local.CurrentWindow == 0)
				ShowWeather();
		}
	}

}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//Զ�̲���
void RecvRemoteDefence_Func(unsigned char *recv_buf, char *cFromIP, int length, int m_Socket)
{
  int i,j;
  int newlength;
  int isAddrOK;
  unsigned char send_b[1520];
  int sendlength;

  i = 0;
  isAddrOK = 1;
  for(j=8; j<8+Local.AddrLen; j++)
   if(LocalCfg.Addr[j-8] != recv_buf[j])
    {
     isAddrOK = 0;
     break;
    }
  //��ַƥ��
  if(isAddrOK == 1)
   {
    memcpy(send_b, recv_buf, length);
    send_b[7] = REPLY;    //Ӧ��
    sendlength = length;
    UdpSendBuff(m_Socket, cFromIP, send_b , sendlength);
    //Զ�̲���
    LocalCfg.DefenceStatus = recv_buf[28];
  }
}
//---------------------------------------------------------------------------
//��λ����
void RecvResetPass_Func(unsigned char *recv_buf, char *cFromIP, int length, int m_Socket)
{
  int i,j;
  int newlength;
  int isAddrOK;
  unsigned char send_b[1520];
  int sendlength;

  i = 0;
  isAddrOK = 1;
  for(j=8; j<8+Local.AddrLen; j++)
   if(LocalCfg.Addr[j-8] != recv_buf[j])
    {
     isAddrOK = 0;
     break;
    }
  //��ַƥ��
  if(isAddrOK == 1)
   {
    memcpy(send_b, recv_buf, length);
    send_b[7] = REPLY;    //Ӧ��
    sendlength = length;
    UdpSendBuff(m_Socket, cFromIP, send_b , sendlength);
    //��λ����
    switch(recv_buf[28])
     {
      case 1: //��λ��������
             if(strcmp(LocalCfg.EngineerPass, "1234") != 0)
              {
               strcpy(LocalCfg.EngineerPass, "1234");
               SaveToFlash(4);    //��Flash�д洢�ļ�
              }
             break;
     }
  }
}
//---------------------------------------------------------------------------
//д��ַ����
void RecvWriteAddress_Func(unsigned char *recv_buf, char *cFromIP, int length, int m_Socket)
{
  int i,j;
  int newlength;
  int isAddrOK;
  unsigned char send_b[1520];
  int sendlength;

  i = 0;
  isAddrOK = 1;
  for(j=8; j<8+Local.AddrLen; j++)
   if(LocalCfg.Addr[j-8] != recv_buf[j])
    {
     isAddrOK = 0;
     break;
    }
  //��ַƥ��
  if(isAddrOK == 1)
   {
    memcpy(send_b, recv_buf, length);
    send_b[7] = REPLY;    //Ӧ��
    sendlength = length;
    UdpSendBuff(m_Socket, cFromIP, send_b , sendlength);

    //д��ַ����
    if(recv_buf[28] & 0x01) //��ַ����
      memcpy(LocalCfg.Addr, recv_buf + 30, 20);
    if(recv_buf[28] & 0x02) //������ַ
     {
      memcpy(LocalCfg.Mac_Addr, recv_buf + 50, 6);
     }
    if(recv_buf[28] & 0x04) //IP��ַ
     {
      memcpy(LocalCfg.IP, recv_buf + 56, 4);
     }
    if(recv_buf[28] & 0x08) //��������
     {
      memcpy(LocalCfg.IP_Mask, recv_buf + 60, 4);
     }
    if(recv_buf[28] & 0x10) //���ص�ַ
     {
      memcpy(LocalCfg.IP_Gate, recv_buf + 64, 4);
     }
    if(recv_buf[28] & 0x20) //��������ַ
      memcpy(LocalCfg.IP_Server, recv_buf + 68, 4);

    SaveToFlash(4);    //��Flash�д洢�ļ�

    RefreshNetSetup(1); //ˢ����������
  }
}
//---------------------------------------------------------------------------
//����ַ����
void RecvReadAddress_Func(unsigned char *recv_buf, char *cFromIP, int length, int m_Socket)
{
  int i,j;
  int newlength;
  int isAddrOK;
  unsigned char send_b[1520];
  int sendlength;

  i = 0;
  isAddrOK = 1;
  for(j=8; j<8+Local.AddrLen; j++)
   if(LocalCfg.Addr[j-8] != recv_buf[j])
    {
     isAddrOK = 0;
     break;
    }
  //��ַƥ��
  if(isAddrOK == 1)
   {
    memcpy(send_b, recv_buf, length);
    send_b[7] = REPLY;    //Ӧ��

    send_b[28] = 0;
    send_b[29] = 0;

    //��ַ����
    memcpy(send_b + 30, LocalCfg.Addr, 20);
    //������ַ
    memcpy(send_b + 50, LocalCfg.Mac_Addr, 6);
    //IP��ַ
    memcpy(send_b + 56, LocalCfg.IP, 4);
    //��������
    memcpy(send_b + 60, LocalCfg.IP_Mask, 4);
    //���ص�ַ
    memcpy(send_b + 64, LocalCfg.IP_Gate, 4);
    //��������ַ
    memcpy(send_b + 68, LocalCfg.IP_Server, 4);

    sendlength = 72;
    UdpSendBuff(m_Socket, cFromIP, send_b , sendlength);
  }
}
//---------------------------------------------------------------------------
//����Ӧ�ó���
void RecvDownLoadFile_Func(unsigned char *recv_buf, char *cFromIP, int length, int m_Socket)
{
  int i;
  unsigned char send_b[8096];
  int PackDataLen = 8000;  
  int sendlength;
  FILE *pic_fd;
  DIR *dirp;
  char picname[80];
  char filename[20] = "sound70";
  struct downfile1 DownData;
  char systemtext[50];
  int DownOK;
  memcpy(&DownData, recv_buf + 9, sizeof(struct downfile1));
  if((strcmp(DownData.FlagText, FLAGTEXT) == 0)&&(strcmp(DownData.FileName, filename) == 0))
   {
    switch(recv_buf[8])
     {
      case STARTDOWN:         //��ʼ����
                     memcpy(send_b, recv_buf, length);
                     send_b[7] = REPLY;    //Ӧ��
                     sendlength = length;
                     UdpSendBuff(m_Socket, cFromIP, send_b , sendlength);

                     printf("��ʼ����, DownData.Filelen = %d\n", DownData.Filelen);
                     if(downbuf == NULL)
                       downbuf = (unsigned char *)malloc(DownData.Filelen);
                     for(i=0; i<DownData.TotalPackage; i++)
                       downloaded_flag[i] = 0;
                     break;
      case DOWN:              //����
                     memcpy(downbuf + DownData.CurrPackage * PackDataLen, recv_buf  + 9 + sizeof(struct downfile1), DownData.Datalen);

                     downloaded_flag[DownData.CurrPackage] = 1;
                     /*if(DownData.CurrPackage != 0)
                      if(DownData.CurrPackage != (OldPackage + 1))
                        printf("CurrPackage = %d, package lost %d, length = %d\n", DownData.CurrPackage , OldPackage + 1, length);
                     if(OldPackage != DownData.CurrPackage)
                       OldPackage = DownData.CurrPackage;     */

                     memcpy(send_b, recv_buf, length);
                     send_b[7] = REPLY;    //Ӧ��
                     sendlength = 9 + sizeof(struct downfile1);
                     UdpSendBuff(m_Socket, cFromIP, send_b , sendlength);
                     break;
      case DOWNFINISHONE:         //�������һ��
                     memcpy(send_b, recv_buf, length);
                     send_b[7] = REPLY;    //Ӧ��
                     sendlength = length;
                     UdpSendBuff(m_Socket, cFromIP, send_b , sendlength);

                     printf("�������һ��\n");
                     DownOK = 1;
                     for(i=0; i<DownData.TotalPackage; i++)
                       if(downloaded_flag[i] == 0)
                        {
                         printf("��ʧ���ݰ���i = %d\n", i);
                         DownOK = 0;
                        }
                     if(DownOK == 1)
                      {
                       sprintf(picname, "/mnt/mtd/%s\0", DownData.FileName);
                       if((pic_fd = fopen(picname, "wb")) == NULL)
                         printf("�޷�����Ӧ�ó����ļ�\n");
                       else
                        {
                         fwrite(downbuf, DownData.Filelen, 1, pic_fd);
                         fclose(pic_fd);
                        }

                       sprintf(systemtext, "chmod 777 %s\0", picname);
                       system(systemtext);

                       free(downbuf);
                       downbuf = NULL;
                       usleep(200*1000);
                       sync();
                       system("reboot");
                      }
                     else
                      {
                       free(downbuf);
                       downbuf = NULL;
                       SendUdpOne(DOWNLOADIMAGE, DOWNFAIL, 0, cFromIP);  //ʧ��
                      }
                     break;
      case STOPDOWN:             //ֹͣ����
                     memcpy(send_b, recv_buf, length);
                     send_b[7] = REPLY;    //Ӧ��
                     sendlength = length;
                     UdpSendBuff(m_Socket, cFromIP, send_b , sendlength);

                     printf("ֹͣ����\n");
                     if(downbuf != NULL)
                      {
                       free(downbuf);
                       downbuf = NULL;
                      } 
                     break;
      case DOWNFINISHALL:        //ȫ���������
                     memcpy(send_b, recv_buf, length);
                     send_b[7] = REPLY;    //Ӧ��
                     sendlength = length;
                     UdpSendBuff(m_Socket, cFromIP, send_b , sendlength);
                     
                     printf("�������\n");
                     break;
     }
   }
}
//---------------------------------------------------------------------------
//����ϵͳӳ��
void RecvDownLoadImage_Func(unsigned char *recv_buf, char *cFromIP, int length, int m_Socket)
{
  int i;
  pthread_attr_t attr;
  unsigned char send_b[8096];
  int PackDataLen = 8000;  
  int sendlength;
  FILE *pic_fd;
  DIR *dirp;
  char picname[80];
  char filename[20] = "msound70pImage";
  struct downfile1 DownData;
  char systemtext[50];
  int DownOK;
  memcpy(&DownData, recv_buf + 9, sizeof(struct downfile1));
  if((strcmp(DownData.FlagText, FLAGTEXT) == 0)&&(strcmp(DownData.FileName, filename) == 0))
   {
    switch(recv_buf[8])
     {
      case STARTDOWN:         //��ʼ����
                     memcpy(send_b, recv_buf, length);
                     send_b[7] = REPLY;    //Ӧ��
                     sendlength = length;
                     UdpSendBuff(m_Socket, cFromIP, send_b , sendlength);

                     printf("��ʼ����, DownData.Filelen = %d\n", DownData.Filelen);
                     if(downbuf == NULL)
                       downbuf = (unsigned char *)malloc(DownData.Filelen);
                     for(i=0; i<DownData.TotalPackage; i++)
                       downloaded_flag[i] = 0;
                     break;
      case DOWN:              //����
                     memcpy(downbuf + DownData.CurrPackage * PackDataLen, recv_buf  + 9 + sizeof(struct downfile1), DownData.Datalen);

                     downloaded_flag[DownData.CurrPackage] = 1;
                     /*if(DownData.CurrPackage != 0)
                      if(DownData.CurrPackage != (OldPackage + 1))
                        printf("CurrPackage = %d, package lost %d, length = %d\n", DownData.CurrPackage , OldPackage + 1, length);
                     if(OldPackage != DownData.CurrPackage)
                       OldPackage = DownData.CurrPackage;  */
                     memcpy(send_b, recv_buf, length);
                     send_b[7] = REPLY;    //Ӧ��
                     sendlength = 9 + sizeof(struct downfile1);
                     UdpSendBuff(m_Socket, cFromIP, send_b , sendlength);
                     break;
      case DOWNFINISHONE:         //�������һ��
                     memcpy(send_b, recv_buf, length);
                     send_b[7] = REPLY;    //Ӧ��
                     sendlength = length;
                     UdpSendBuff(m_Socket, cFromIP, send_b , sendlength);

                     printf("�������һ��\n");
                     DownOK = 1;
                     for(i=0; i<DownData.TotalPackage; i++)
                       if(downloaded_flag[i] == 0)
                        {
                         printf("��ʧ���ݰ���i = %d\n", i);
                         DownOK = 0;
                        }
                     if(DownOK == 1)
                      {
                       downbuflen = DownData.Filelen;
                       strcpy(downip, cFromIP);
                       download_image_flag = 1;
                       pthread_attr_init(&attr);
                       pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
                       pthread_create(&download_image_thread,&attr,(void *)download_image_thread_func, NULL);
                       pthread_attr_destroy(&attr);
                       if(download_image_thread == 0 )
                        {
                          printf("�޷�ϵͳӳ�������߳�\n");
                          free(downbuf);
                          downbuf = NULL;
                        }
                      }
                     else
                      {
                       free(downbuf);
                       downbuf = NULL;
                       SendUdpOne(DOWNLOADIMAGE, DOWNFAIL, 0, cFromIP);  //ʧ��
                      }
                     break;
      case STOPDOWN:             //ֹͣ����
                     memcpy(send_b, recv_buf, length);
                     send_b[7] = REPLY;    //Ӧ��
                     sendlength = length;
                     UdpSendBuff(m_Socket, cFromIP, send_b , sendlength);

                     printf("ֹͣ����\n");
                     if(downbuf != NULL)
                      {
                       free(downbuf);
                       downbuf = NULL;
                      } 
                     break;
      case DOWNFINISHALL:        //ȫ���������
                     memcpy(send_b, recv_buf, length);
                     send_b[7] = REPLY;    //Ӧ��
                     sendlength = length;
                     UdpSendBuff(m_Socket, cFromIP, send_b , sendlength);
                     
                     printf("�������\n");
                     break;
     }
   }
}
//---------------------------------------------------------------------------
//ϵͳӳ�������߳�
void download_image_thread_func(void)
{
  #ifdef _DEBUG
    printf("����ϵͳӳ�������߳�\n" );
  #endif
  flashcp(downbuf, downbuflen, downip);
  free(downbuf);
  downbuf = NULL;

  #ifdef _DEBUG
    printf("����ϵͳӳ�������߳�\n" );
  #endif
}
//---------------------------------------------------------------------------
//д���ڻ���������
void RecvWriteRoomSetup_Func(unsigned char *recv_buf, char *cFromIP, int length, int m_Socket)
{
}
//---------------------------------------------------------------------------
//�����ڻ���������
void RecvReadRoomSetup_Func(unsigned char *recv_buf, char *cFromIP, int length, int m_Socket)
{
}
//---------------------------------------------------------------------------
void UdpVideoRcvThread(void)  //UDP�����̺߳���
{
  /* ѭ���������� */
//  int oldframeno=0;
  unsigned char send_b[1520];
  int sendlength;
  char FromIP[20];
  int newframeno;
  int currpackage;
  int i,j;
  int sub;
  short PackIsExist; //���ݰ��ѽ��ձ�־
  short FrameIsNew;  //���ݰ��Ƿ�����֡�Ŀ�ʼ
  struct sockaddr_in c_addr;
  socklen_t addr_len;
  int len;
  int tmp;
  unsigned char buff[8096];

//  char tmpAddr[21];
  int isAddrOK;
  #ifdef _DEBUG
    printf("This is udp video pthread.\n");
  #endif
  UdpRecvFlag = 1;

  addr_len = sizeof(c_addr);
  while (UdpRecvFlag == 1)
   {
    len = recvfrom(m_VideoSocket, buff, sizeof(buff) - 1, 0,
     (struct sockaddr *) &c_addr, &addr_len);
    if (len < 0)
     {
      perror("recvfrom");
     }
    buff[len] = '\0';
    strcpy(FromIP, inet_ntoa(c_addr.sin_addr));
    #ifdef _DEBUG
//      printf("FromIP is %s\n",FromIP);
    #endif
    if((buff[0]==UdpPackageHead[0])&&(buff[1]==UdpPackageHead[1])&&(buff[2]==UdpPackageHead[2])
      &&(buff[3]==UdpPackageHead[3])&&(buff[4]==UdpPackageHead[4])&&(buff[5]==UdpPackageHead[5]))
     {
      if(Local.CurrentWindow == 255)
        return;
      switch(buff[6])
       {
        case NSORDER:   //�����������������ڹ㲥��
    //    case NSSERVERORDER:  //����������(NS������)
                switch(buff[7])
                 {
                  case 1://����
                         if(len == 56)
                          {
                           RecvNSAsk_Func(buff, FromIP, m_VideoSocket);
                          }
                         else
                          {
                           #ifdef _DEBUG
                             printf("�����������ݳ����쳣\n");
                           #endif
                          }
                         break;
                  case 2://������Ӧ
                         if(len >= 57)
                          {
                           RecvNSReply_Func(buff, FromIP, m_VideoSocket);
                          }
                         else
                          {
                           #ifdef _DEBUG
                             printf("����Ӧ�����ݳ����쳣\n");
                           #endif
                          }
                         break;
                 }
                break;
        case VIDEOTALK:    //���������ӶԽ�
        case VIDEOTALKTRANS:  //���������ӶԽ���ת����
               switch(buff[8])
                {
                  case CALL:        //�Է�����Խ�
                            if(len == 62)
                             {
                              if(InitSuccFlag == 1)
                                RecvTalkCall_Func(buff, FromIP);
                              else
                                #ifdef _DEBUG
                                  printf("������ʼ��û�����\n");
                                #endif
                             }
                            else
                             {
                              #ifdef _DEBUG
                                printf("�Խ��������ݳ����쳣\n");
                              #endif
                             }
                            break;
                  case LINEUSE:        //�Է���æ
                            if(len == 57)
                             {
                              RecvTalkLineUse_Func(buff, FromIP);
                             }
                            else
                             {
                              #ifdef _DEBUG
                                printf("ռ��Ӧ�����ݳ����쳣\n");
                              #endif
                             }
                            break;
                  case CALLANSWER:  //����Ӧ��
                    //        printf("FromIP is %s\n",FromIP);
                    //        printf("Multi_Udp_Buff[i].RemoteHost is %s\n",Multi_Udp_Buff[i].RemoteHost);
                            if(len == 62)
                             {
                              RecvTalkCallAnswer_Func(buff, FromIP);
                             }
                            else
                             {
                              #ifdef _DEBUG
                                printf("�Խ�����Ӧ�����ݳ����쳣\n");
                              #endif
                             }
                            break;
                  case CALLSTART:  //���з���ʼͨ��
                            if(len == 57)
                             {
                              RecvTalkCallStart_Func(buff, FromIP);
                             }
                            else
                             {
                              #ifdef _DEBUG
                                printf("�Խ���ʼͨ�����ݳ����쳣\n");
                              #endif
                             }
                            break;
                  case CALLCONFIRM:     //ͨ������ȷ��
                            if(len == 61)
                             {
                              RecvTalkCallConfirm_Func(buff, FromIP);
                             }
                            else
                             {
                              #ifdef _DEBUG
                                printf("�Խ�ͨ������ȷ�����ݳ����쳣\n");
                              #endif
                             }
                            break;
                  case CALLEND:  //ͨ������
                            //����������Ƶ
                            //��Ϊ�Է���������Ӧ��
                            if(len == 57)
                             {
                              RecvTalkCallEnd_Func(buff, FromIP);
                             }
                            else
                             {
                              #ifdef _DEBUG
                                printf("�����Խ����ݳ����쳣\n");
                              #endif
                             }
                            break;
                  case REMOTEOPENLOCK:    //Զ�̿���
                            if(len == 57)
                             {
                              RecvTalkRemoteOpenLock_Func(buff, FromIP);
                             }
                            else
                             {
                              #ifdef _DEBUG
                                printf("Զ�̿������ݳ����쳣\n");
                              #endif
                             }
                            break;                            
                  case FORCEIFRAME:    //ǿ��I֡����
                            Local.ForceIFrame = 1;
                            break;
                  case ZOOMOUT:    //�Ŵ�(720*480)
                            if(len == 57)
                             {
                              RecvTalkZoomOut_Func(buff, FromIP);
                             }
                            else
                             {
                              #ifdef _DEBUG
                                 printf("�Ŵ�(720*480)���ݳ����쳣");
                              #endif
                             }
                            break;
                  case ZOOMIN:    //��С(352*240)
                            if(len == 57)
                             {
                              RecvTalkZoomIn_Func(buff, FromIP);
                             }
                            else
                             {
                              #ifdef _DEBUG
                                 printf("��С(352*240)���ݳ����쳣");
                              #endif
                             }
                            break;
                  case CALLUP: //ͨ������
                  case CALLDOWN: //ͨ������
                           RecvTalkCallUpDown_Func(buff, FromIP, len);
                           break;
                  }
                 break;
        case VIDEOWATCH:     //���������
        case VIDEOWATCHTRANS:  //�����������ת����
               switch(buff[8])
                {
                  case CALL:        //�Է��������  ���ڻ�����Ӧ����
                         /*   if(len == 57)
                             {
                              RecvWatchCall_Func(buff, FromIP);
                             }
                            else
                             {
                              #ifdef _DEBUG
                                printf("���Ӻ������ݳ����쳣\n");
                              #endif
                             }     */
                            break;
                  case LINEUSE:        //�Է���æ
                            if(len == 57)
                             {
                              RecvWatchLineUse_Func(buff, FromIP);
                             }
                            else
                             {
                              #ifdef _DEBUG
                                printf("ռ��Ӧ�����ݳ����쳣\n");
                              #endif
                             }
                            break;
                  case CALLANSWER:  //����Ӧ��
                    //        printf("FromIP is %s\n",FromIP);
                    //        printf("Multi_Udp_Buff[i].RemoteHost is %s\n",Multi_Udp_Buff[i].RemoteHost);
                            if(len == 57)
                             {
                              RecvWatchCallAnswer_Func(buff, FromIP);
                             }
                            else
                             {
                              #ifdef _DEBUG
                                printf("���Ӻ���Ӧ�����ݳ����쳣\n");
                              #endif
                             }
                            break;
                  case CALLCONFIRM:     //ͨ������ȷ��
                            if(len == 61)
                             {
                              RecvWatchCallConfirm_Func(buff, FromIP);
                             }
                            else
                             {
                              #ifdef _DEBUG
                                printf("����ͨ������ȷ�����ݳ����쳣\n");
                              #endif
                             }
                            break;
                  case CALLEND:  //ͨ������
                            //����������Ƶ
                            //��Ϊ�Է���������Ӧ��
                            if(len == 57)
                             {
                              RecvWatchCallEnd_Func(buff, FromIP);
                             }
                            else
                             {
                              #ifdef _DEBUG
                                printf("�����������ݳ����쳣\n");
                              #endif
                             }
                            break;
                  case FORCEIFRAME:    //ǿ��I֡����
                            Local.ForceIFrame = 1;
                            break;
                  case ZOOMOUT:    //�Ŵ�(720*480)
                            if(len == 57)
                             {
                              RecvWatchZoomOut_Func(buff, FromIP);
                             }
                            else
                             {
                              #ifdef _DEBUG
                                 printf("�Ŵ�(720*480)���ݳ����쳣");
                              #endif
                             }
                            break;
                  case ZOOMIN:    //��С(352*240)
                            if(len == 57)
                             {
                              RecvWatchZoomIn_Func(buff, FromIP);
                             }
                            else
                             {
                              #ifdef _DEBUG
                                 printf("��С(352*240)���ݳ����쳣");
                              #endif
                             }
                            break;
                  case CALLUP: //ͨ������
                  case CALLDOWN: //ͨ������
                           RecvWatchCallUpDown_Func(buff, FromIP, len);
                           break;
                }
               break;
        case FINDEQUIP:       //�����豸
               if(len == 56)
                {
                 RecvFindEquip_Func(buff, FromIP, len, m_VideoSocket);
                }
               else
                {
                 #ifdef _DEBUG
                   printf("�����豸���ݳ����쳣\n");
                 #endif
                }
               break;
       }
     }
   }
}
//-----------------------------------------------------------------------
//��������
void RecvNSAsk_Func(unsigned char *recv_buf, char *cFromIP, int m_Socket)
{
  int i,j;
  int isAddrOK;
  unsigned char send_b[1520];
  int sendlength;
  isAddrOK = 1;
  for(j=8; j<8+Local.AddrLen; j++)
   if(LocalCfg.Addr[j-8] != recv_buf[j])
   {
    isAddrOK = 0;
    break;
   }
  //���Ǳ�������
  if(isAddrOK == 0)
   {
    isAddrOK = 1;
    for(j=32; j<32+12; j++)
     if(LocalCfg.Addr[j-32] != recv_buf[j])
      {
       isAddrOK = 0;
       break;
      }

//printf("isAddrOK = %d\n", isAddrOK);
  //Ҫ��������Ǳ�����ַ
    if(isAddrOK == 1)
     {
      memcpy(send_b, recv_buf, 32);
      send_b[7] = REPLY;    //Ӧ��

      if(Local.isHost == '0')  //����
       {
        send_b[32] = Local.DenNum + 1;   //��ַ����

        memcpy(send_b + 33, LocalCfg.Addr, 20);
        memcpy(send_b + 53, LocalCfg.IP, 4);
        for(i=0; i<Local.DenNum; i++)
         {
          memcpy(send_b + 57 + 24*i, Local.DenAddr[i], 20);
          memcpy(send_b + 57 + 20 +24*i, Local.DenIP[i], 4);
         }
        sendlength = 57 + 24*Local.DenNum;
       }
      else                    //����
       {
        send_b[32] = 1;   //��ַ����

        memcpy(send_b + 33, LocalCfg.Addr, 20);
        memcpy(send_b + 53, LocalCfg.IP, 4);
        sendlength = 57;
       }
      UdpSendBuff(m_Socket, cFromIP, send_b , sendlength);
     }
  }
}
//-----------------------------------------------------------------------
//����Ӧ��
void RecvNSReply_Func(unsigned char *recv_buf, char *cFromIP, int m_Socket)
{
  int i,j, k;
  int CurrOrder;
  int isAddrOK;

  //��ʱ�رձ��ؽ������������������
  #ifdef _TESTNSSERVER
    if(m_Socket == m_VideoSocket)
      return;
  #endif
    
  //����������
  pthread_mutex_lock (&Local.udp_lock);

  for(i=0; i<UDPSENDMAX; i++)
   if(Multi_Udp_Buff[i].isValid == 1)
     if(Multi_Udp_Buff[i].SendNum  < MAXSENDNUM)
      if((Multi_Udp_Buff[i].buf[6] == NSORDER)||(Multi_Udp_Buff[i].buf[6] == NSSERVERORDER))
       if((Multi_Udp_Buff[i].buf[7] == ASK)&&(recv_buf[32] > 0))
        {
         //�ж�Ҫ�������ַ�Ƿ�ƥ��
         isAddrOK = 1;
         for(j=32; j<32+12; j++)
          if(Multi_Udp_Buff[i].buf[j] != recv_buf[j+1])
           {
            isAddrOK = 0;
            break;
           }
         CurrOrder = Multi_Udp_Buff[i].CurrOrder;
         Multi_Udp_Buff[i].isValid = 0;
         Multi_Udp_Buff[i].SendNum = 0;
         if(isAddrOK == 1)
           break;         
        }
  //�򿪻�����
  pthread_mutex_unlock (&Local.udp_lock);

  if(isAddrOK == 1)
   { //�յ���ȷ�Ľ�����Ӧ
    if(CurrOrder == 255) //�����򸱻�����
     {
      //�鿴�б����Ƿ����иø�����Ϣ
      isAddrOK = 0;
      for(i=0; i<Local.DenNum; i++)
       {
        isAddrOK = 1;
        for(j=0; j<12; j++)
         if(Local.DenAddr[i][j] != recv_buf[j+33])
          {
           isAddrOK = 0;
           break;
          }
        if(isAddrOK == 1)
          for(j=0; j<4; j++)
           if(Local.DenIP[i][j] != recv_buf[j+53])
            {
             isAddrOK = 0;
             break;
            }
        if(isAddrOK == 1)
          break;
       }
      if(isAddrOK == 0)
       {
        memcpy(Local.DenIP[Local.DenNum], recv_buf + 53, 4);
        memcpy(Local.DenAddr[Local.DenNum], recv_buf + 33, 20);
        Local.DenNum ++;
       }
     }
    else
     {
           Remote.DenNum = recv_buf[32];
           if((Remote.DenNum >= 1)&&(Remote.DenNum <= 10))
            {
              {
               for(j=0; j<Remote.DenNum; j++)
                {
                 Remote.IP[j][0] = recv_buf[53+24*j];
                 Remote.IP[j][1] = recv_buf[54+24*j];
                 Remote.IP[j][2] = recv_buf[55+24*j];
                 Remote.IP[j][3] = recv_buf[56+24*j];
                 Remote.DenIP[0] = Remote.IP[j][0];
                 Remote.DenIP[1] = Remote.IP[j][1];
                 Remote.DenIP[2] = Remote.IP[j][2];
                 Remote.DenIP[3] = Remote.IP[j][3];
                 for(k=0; k<20; k++)
                   Remote.Addr[j][k] = recv_buf[33+24*j+k];
                 Remote.GroupIP[0] = 236;
                 Remote.GroupIP[1] = LocalCfg.IP[1];
                 Remote.GroupIP[2] = LocalCfg.IP[2];
                 Remote.GroupIP[3] = LocalCfg.IP[3];

                 //����������
                 pthread_mutex_lock (&Local.udp_lock);

                 for(i=0; i<UDPSENDMAX; i++)
                  if(Multi_Udp_Buff[i].isValid == 0)
                   {
                    Multi_Udp_Buff[i].SendNum = 0;
                    Multi_Udp_Buff[i].m_Socket = m_VideoSocket;
                    sprintf(Multi_Udp_Buff[i].RemoteHost, "%d.%d.%d.%d\0",
                         Remote.DenIP[0],Remote.DenIP[1],Remote.DenIP[2],Remote.DenIP[3]);
                    #ifdef _DEBUG
                      printf("Multi_Udp_Buff[i].RemoteHost is %s\n",Multi_Udp_Buff[i].RemoteHost);
                      printf("������ַ�ɹ�,���ں���\n");
                    #endif
                    //ͷ��
                    memcpy(Multi_Udp_Buff[i].buf, UdpPackageHead, 6);
                    //����
                    Multi_Udp_Buff[i].buf[6] = CurrOrder;
                    Multi_Udp_Buff[i].buf[7] = ASK;    //����
                    // ������
                    Multi_Udp_Buff[i].buf[8] = CALL;

                    memcpy(Multi_Udp_Buff[i].buf+9,LocalCfg.Addr,20);
                    memcpy(Multi_Udp_Buff[i].buf+29,LocalCfg.IP,4);
                    memcpy(Multi_Udp_Buff[i].buf+33,Remote.Addr[j],20);
                    memcpy(Multi_Udp_Buff[i].buf+53,Remote.IP[j],4);

                    if(Remote.DenNum == 1)
                      Multi_Udp_Buff[i].buf[57] = 0; //����
                    else
                      Multi_Udp_Buff[i].buf[57] = 1; //�鲥
                    //�鲥��ַ
                    Multi_Udp_Buff[i].buf[58] = Remote.GroupIP[0];
                    Multi_Udp_Buff[i].buf[59] = Remote.GroupIP[1];
                    Multi_Udp_Buff[i].buf[60] = Remote.GroupIP[2];
                    Multi_Udp_Buff[i].buf[61] = Remote.GroupIP[3];

                    Multi_Udp_Buff[i].nlength = 62;
                    Multi_Udp_Buff[i].DelayTime = 800;
                    Multi_Udp_Buff[i].isValid = 1;
                    sem_post(&multi_send_sem);
                    break;
                   }
                 //�򿪻�����
                 pthread_mutex_unlock (&Local.udp_lock);
                }
              }
           }
      }
  }
}
//-----------------------------------------------------------------------
//���Ӻ���
void RecvWatchCall_Func(unsigned char *recv_buf, char *cFromIP)
{
  int i,j;
  int isAddrOK;
  unsigned char send_b[1520];
  int sendlength;
  uint32_t Ip_Int;
  //����״̬Ϊ����
  if(Local.Status == 0)
   {
    memcpy(send_b, recv_buf, 57);
    send_b[7]=ASK;    //����
    send_b[8]=CALLANSWER;//����Ӧ��
    sendlength=57;
    UdpSendBuff(m_VideoSocket, cFromIP, send_b , sendlength);

    //��ȡ�Է���ַ
    memcpy(Remote.Addr[0], recv_buf+9, 20);
    memcpy(Remote.IP[0], recv_buf+29, 4);

    Ip_Int=inet_addr(cFromIP);
    memcpy(Remote.DenIP, &Ip_Int,4);
    printf("Remote.DenIP, %d.%d.%d.%d\0",
            Remote.DenIP[0],Remote.DenIP[1],Remote.DenIP[2],Remote.DenIP[3]);
    if((Remote.DenIP[0] == Remote.IP[0][0]) && (Remote.DenIP[1] == Remote.IP[0][1])
      && (Remote.DenIP[2] == Remote.IP[0][2]) &&(Remote.DenIP[3] == Remote.IP[0][3]))
      {
       Remote.isDirect = 0;
       #ifdef _DEBUG
         printf("�Է�����ֱͨ���Ӻ���\n");
       #endif
      }
    else
      {
       Remote.isDirect = 1; 
       #ifdef _DEBUG
         printf("�Է�������ת���Ӻ���\n");
       #endif
      }

    Local.Status = 4;  //״̬Ϊ������
    //��ʼ¼����Ƶ
//    StartRecVideo(720, 480);
//    StartRecVideo(352, 288);
//    StartRecVideo(CIF_W, CIF_H);
    Local.CallConfirmFlag = 1; //�������߱�־
    Local.Timer1Num = 0;
    Local.TimeOut = 0;       //���ӳ�ʱ,  ͨ����ʱ,  ���г�ʱ�����˽���
    Local.OnlineNum = 0;     //����ȷ�����
    Local.OnlineFlag = 1;
   }
  //����Ϊæ
  else
   {
    memcpy(send_b, recv_buf, 57);
    send_b[7]=ASK;    //����
    send_b[8]=LINEUSE;//ռ��Ӧ��
    sendlength=57;
    UdpSendBuff(m_VideoSocket, cFromIP, send_b , sendlength);
    #ifdef _DEBUG
      printf("�Է�������Ӻ���\n");
    #endif
   }
}
//-----------------------------------------------------------------------
//����ռ��Ӧ��
void RecvWatchLineUse_Func(unsigned char *recv_buf, char *cFromIP)
{
  int i,j;
  int isAddrOK;

  //����������
  pthread_mutex_lock (&Local.udp_lock);
  if(recv_buf[7] == ASK)   //Ӧ��
   for(i=0; i<UDPSENDMAX; i++)
    if(Multi_Udp_Buff[i].isValid == 1)
     if(Multi_Udp_Buff[i].m_Socket == m_VideoSocket)
      if(Multi_Udp_Buff[i].SendNum  < MAXSENDNUM)
       if((Multi_Udp_Buff[i].buf[6] == VIDEOWATCH)||(Multi_Udp_Buff[i].buf[6] == VIDEOWATCHTRANS))
        if(Multi_Udp_Buff[i].buf[7] == ASK)
         if(Multi_Udp_Buff[i].buf[8] == CALL)
          if(strcmp(Multi_Udp_Buff[i].RemoteHost, cFromIP) == 0)
           {
            Multi_Udp_Buff[i].isValid = 0;
          //  Local.Status = 0;  //״̬��Ϊ����
            ShowStatusText(CALLX,CALLY, 3, cBlack, 1, 1, "�Է�ռ��", 0);
		  DisplayMainWindow(MainWindow);
#if 0
            //��ʱ����ʾ��Ϣ��־
            PicStatBuf.Type = 13;
            PicStatBuf.Time = 0;
            PicStatBuf.Flag = 1;
            #ifdef _DEBUG
              printf("�յ�����ռ��Ӧ��\n");
            #endif
#endif
            break;
           }
  //�򿪻�����
  pthread_mutex_unlock (&Local.udp_lock);
}
//-----------------------------------------------------------------------
//���Ӻ���Ӧ��
void RecvWatchCallAnswer_Func(unsigned char *recv_buf, char *cFromIP)
{
  int i,j;
  int isAddrOK;

  //����������
  pthread_mutex_lock (&Local.udp_lock);
  if(recv_buf[7] == ASK)   //Ӧ��
   for(i=0; i<UDPSENDMAX; i++)
    if(Multi_Udp_Buff[i].isValid == 1)
     if(Multi_Udp_Buff[i].m_Socket == m_VideoSocket)
      if(Multi_Udp_Buff[i].SendNum  < MAXSENDNUM)
       if((Multi_Udp_Buff[i].buf[6] == VIDEOWATCH)||(Multi_Udp_Buff[i].buf[6] == VIDEOWATCHTRANS))
        if(Multi_Udp_Buff[i].buf[7] == ASK)
         if(Multi_Udp_Buff[i].buf[8] == CALL)
          if(strcmp(Multi_Udp_Buff[i].RemoteHost, cFromIP) == 0)
           {
            Multi_Udp_Buff[i].isValid = 0;
            //��ʼ������Ƶ
            StartPlayVideo(CIF_W, CIF_H);
        //    StartPlayVideo(720, 480);

            Local.CallConfirmFlag = 1; //�������߱�־
            Local.Timer1Num = 0;
            Local.TimeOut = 0;       //���ӳ�ʱ,  ͨ����ʱ,  ���г�ʱ�����˽���
            Local.OnlineNum = 0;     //����ȷ�����
            Local.OnlineFlag = 1;

            Local.Status = 3;  //״̬Ϊ����
            #ifdef _DEBUG
              printf("�յ����Ӻ���Ӧ��\n");
            #endif
            OpenOsd();   //��OSD
            //sprintf(Label_W_Time.Text, "%02d:%02d\0", Local.TimeOut/INTRPERSEC/60,
            //       (Local.TimeOut/INTRPERSEC)%60);
            //Label_W_Time.Text[2] = ',';
           // Label_W_Time.Text[5] = '\0';
           // ShowOsd(Label_W_Time.Text);
    //        ShowStatusText(50, 380 , 3, cBlack, 1, 1, "���ڼ�����", 0);
            break;
           }
  //�򿪻�����
  pthread_mutex_unlock (&Local.udp_lock);
}
//-----------------------------------------------------------------------
//��������ȷ��
void RecvWatchCallConfirm_Func(unsigned char *recv_buf, char *cFromIP)
{
  int i,j;
  int isAddrOK;
  unsigned char send_b[1520];
  int sendlength;

  //��������    ������
  if((Local.Status == 4)&&(recv_buf[7] == ASK))
   {
    memcpy(send_b, recv_buf, 61);
    send_b[7]=REPLY;    //Ӧ��
    sendlength=61;
    UdpSendBuff(m_VideoSocket, cFromIP, send_b , sendlength);
    Local.CallConfirmFlag = 1;
    #ifdef _DEBUG
  //    printf("�յ���������ȷ��\n");
    #endif
   }
  else  //��������
   if(Local.Status == 3)
   {
    Local.CallConfirmFlag = 1;
    #ifdef _DEBUG
//      printf("�յ��Է�Ӧ�𱾻���������ȷ��\n");
    #endif
   }
}
//-----------------------------------------------------------------------
//���Ӻ��н���
void RecvWatchCallEnd_Func(unsigned char *recv_buf, char *cFromIP)
{
  int i,j;
  int isAddrOK;
  unsigned char send_b[1520];
  int sendlength;

  //��������    ������
  if(((Local.Status == 3)||(Local.Status == 4))&&(recv_buf[7] == ASK))
   {
    Local.OnlineFlag = 0;
    Local.CallConfirmFlag = 0; //�������߱�־
    memcpy(send_b, recv_buf, 57);
    send_b[7]=REPLY;    //Ӧ��
    sendlength=57;
    UdpSendBuff(m_VideoSocket, cFromIP, send_b , sendlength);

    switch(Local.Status)
     {
      case 3: //��������
             StopPlayVideo();
             //ShowVideoWindow();    //�ر���Ƶ����
             CloseOsd();
             break;
      case 4: //����������
             Local.Status = 0;  //״̬Ϊ����
             break;
     }
    CloseOsd();
    if(Local.CurrentWindow == 13)
     {
      ShowStatusText(50, 380 , 3, cBlack, 1, 1, "�Է���������", 0);
      //��ʱ����ʾ��Ϣ��־
      PicStatBuf.Type = 13;
      PicStatBuf.Time = 0;
      PicStatBuf.Flag = 1;
     }
    #ifdef _DEBUG
      printf("�Է���������\n");
    #endif
   }
  else  //��������
   if(Local.Status == 3)
   {
    Local.OnlineFlag = 0;
    Local.CallConfirmFlag = 0; //�������߱�־
    //����������
    pthread_mutex_lock (&Local.udp_lock);
    //��������
    if(recv_buf[7] == REPLY)
     for(i=0; i<UDPSENDMAX; i++)
      if(Multi_Udp_Buff[i].isValid == 1)
       if(Multi_Udp_Buff[i].m_Socket == m_VideoSocket)
        if(Multi_Udp_Buff[i].SendNum  < MAXSENDNUM)
         if((Multi_Udp_Buff[i].buf[6] == VIDEOWATCH)||(Multi_Udp_Buff[i].buf[6] == VIDEOWATCHTRANS))
          if(Multi_Udp_Buff[i].buf[7] == ASK)
           if(Multi_Udp_Buff[i].buf[8] == CALLEND)
            if(strcmp(Multi_Udp_Buff[i].RemoteHost, cFromIP) == 0)
             {
              Multi_Udp_Buff[i].isValid = 0;
              switch(Local.Status)
               {
                case 3: //��������
                       StopPlayVideo();
                       //ShowVideoWindow();    //�ر���Ƶ����
                       CloseOsd();

                       if(Local.ForceEndWatch == 0)  //�к���ʱ��ǿ�ƹؼ���
                        {
                         //��ʱ����ʾ��Ϣ��־
                         PicStatBuf.Type = 13;
                         PicStatBuf.Time = 0;
                         PicStatBuf.Flag = 1;
                        }
                       else
                        {
                         DisplayMainWindow(0);
                         Local.Status = 0;  //״̬Ϊ����
                         Local.ForceEndWatch = 0;  //�к���ʱ��ǿ�ƹؼ���
                        }                       
                       
       //                Local.Status = 0;  //״̬Ϊ����
                       break;
                case 4: //����������
                       Local.Status = 0;  //״̬Ϊ����
                 //      StopRecVideo();
                       break;
               }
              #ifdef _DEBUG
                printf("�Է�Ӧ�𱾻���������\n");
              #endif
              CloseOsd();
              if(Local.CurrentWindow == 13)
               {
                ShowStatusText(50, 380 , 3, cBlack, 1, 1, "�Է�Ӧ�𱾻���������", 0);
                //��ʱ����ʾ��Ϣ��־
                PicStatBuf.Type = 13;
                PicStatBuf.Time = 0;
                PicStatBuf.Flag = 1;
               }
              break;
             }
    //�򿪻�����
    pthread_mutex_unlock (&Local.udp_lock);
   }
}
//-----------------------------------------------------------------------
//�Ŵ�(720*480)
void RecvWatchZoomOut_Func(unsigned char *recv_buf, char *cFromIP)
{
  int i,j;
  int isAddrOK;
  unsigned char send_b[1520];
  int sendlength;

  //��������    ������
  if((Local.Status == 4)&&(recv_buf[7] == ASK))
   {
   }
  else  //��������
   if(Local.Status == 3)
   {
    //����������
    pthread_mutex_lock (&Local.udp_lock);
    //��������
    if(recv_buf[7] == REPLY)
     for(i=0; i<UDPSENDMAX; i++)
      if(Multi_Udp_Buff[i].isValid == 1)
       if(Multi_Udp_Buff[i].m_Socket == m_VideoSocket)
        if(Multi_Udp_Buff[i].SendNum  < MAXSENDNUM)
         if((Multi_Udp_Buff[i].buf[6] == VIDEOWATCH)||(Multi_Udp_Buff[i].buf[6] == VIDEOWATCHTRANS))
          if(Multi_Udp_Buff[i].buf[7] == ASK)
           if(Multi_Udp_Buff[i].buf[8] == ZOOMOUT)
            if(strcmp(Multi_Udp_Buff[i].RemoteHost, cFromIP) == 0)
             {
              Multi_Udp_Buff[i].isValid = 0;
              if(Local.PlayPicSize == 1)
               {
                Local.ZoomInOutFlag = 1;   //���ڷŴ���С��
                if(Local.Status != 0)
                  StopPlayVideo();   //352*240
                //����  �ڶ�ҳ  ��Ƶ720*480
                memset(fbmem + f_data.buf_len, 16, f_data.uv_offset);
                memset(fbmem + f_data.buf_len + f_data.uv_offset, 128, f_data.uv_offset/4);
                memset(fbmem + f_data.buf_len + f_data.uv_offset*5/4, 128, f_data.uv_offset/4);
                if(Local.Status != 0)
                  StartPlayVideo(D1_W, D1_H);  //720*480
                Local.ZoomInOutFlag = 0;   //���ڷŴ���С��
                #ifdef _DEBUG
                  printf("�Է�Ӧ�𱾻��Ŵ�ͼ��\n");
                #endif
               }
              break;
             }
    //�򿪻�����
    pthread_mutex_unlock (&Local.udp_lock);
   }
}
//-----------------------------------------------------------------------
//��С(352*240)
void RecvWatchZoomIn_Func(unsigned char *recv_buf, char *cFromIP)
{
  int i,j;
  int isAddrOK;
  unsigned char send_b[1520];
  int sendlength;

  //��������    ������
  if((Local.Status == 4)&&(recv_buf[7] == ASK))
   {
   }
  else  //��������
   if(Local.Status == 3)
   {
    //����������
    pthread_mutex_lock (&Local.udp_lock);
    //��������
    if(recv_buf[7] == REPLY)
     for(i=0; i<UDPSENDMAX; i++)
      if(Multi_Udp_Buff[i].isValid == 1)
       if(Multi_Udp_Buff[i].m_Socket == m_VideoSocket)
        if(Multi_Udp_Buff[i].SendNum  < MAXSENDNUM)
         if((Multi_Udp_Buff[i].buf[6] == VIDEOWATCH)||(Multi_Udp_Buff[i].buf[6] == VIDEOWATCHTRANS))
          if(Multi_Udp_Buff[i].buf[7] == ASK)
           if(Multi_Udp_Buff[i].buf[8] == ZOOMIN)
            if(strcmp(Multi_Udp_Buff[i].RemoteHost, cFromIP) == 0)
             {
              Multi_Udp_Buff[i].isValid = 0;
              if(Local.PlayPicSize == 2)
               {
                Local.ZoomInOutFlag = 1;   //���ڷŴ���С��
                //��ʾ��Ƶ����
                //ShowVideoWindow();
                if(Local.Status != 0)
                  StopPlayVideo();   //720*480
                if(Local.Status != 0)
                  StartPlayVideo(CIF_W, CIF_H);  //352*240
                Local.ZoomInOutFlag = 0;   //���ڷŴ���С��
                #ifdef _DEBUG
                  printf("�Է�Ӧ�𱾻���Сͼ��\n");
                #endif
               }
              break;
             }
    //�򿪻�����
    pthread_mutex_unlock (&Local.udp_lock);
   }
}
//-----------------------------------------------------------------------
//��������
void RecvWatchCallUpDown_Func(unsigned char *recv_buf, char *cFromIP, int length)
{
  int i,j;
  int isAddrOK;
  unsigned char send_b[1520];
  int sendlength;
//  int newframeno;
//  int currpackage;
//  int sub;
  short PackIsExist; //���ݰ��ѽ��ձ�־
  short FrameIsNew;  //���ݰ��Ƿ�����֡�Ŀ�ʼ
  int tmp;
  TempVideoNode1 * tmp_videonode;
  int isFull;
  struct talkdata1 talkdata;
  if((Local.Status == 3)||(Local.Status == 4))  //״̬Ϊ����
   switch(recv_buf[61])
   {
    case 2://��Ƶ  I֡  352*240
    case 3://��Ƶ  P֡  352*240
    case 4://��Ƶ  I֡  720*480
    case 5://��Ƶ  P֡  720*480
           if(((recv_buf[61] == 2)||(recv_buf[61] == 3))&&(Local.PlayPicSize == 2))
             break;
           if(((recv_buf[61] == 4)||(recv_buf[61] == 5))&&(Local.PlayPicSize == 1))
             break;    
                  //֡���
                  memcpy(&talkdata, recv_buf + 9, sizeof(talkdata));
                  PackIsExist = 0;
                  FrameIsNew = 1;
                  //����videoplaybuf[vpbuf_iget].buffer
                  pthread_mutex_lock(&sync_s.video_play_lock);

                  if(temp_video_n >= MP4VNUM)
                   {
                    temp_video_n = MP4VNUM;
                    #ifdef _DEBUG
                      printf("temp_video is full\n");
                    #endif
                   }
                  else
                   {
                    tmp_videonode = find_videonode(TempVideoNode_h, talkdata.Frameno, talkdata.CurrPackage);
                    if(tmp_videonode == NULL)
                      isFull = creat_videonode(TempVideoNode_h, talkdata, recv_buf, length);
                    else
                      isFull = add_videonode(tmp_videonode, talkdata, recv_buf, length);

                    if(isFull == 1)
                     {
                      TimeStamp.OldCurrVideo = TimeStamp.CurrVideo; //��һ�ε�ǰ��Ƶʱ��
                      TimeStamp.CurrVideo = talkdata.timestamp;

                      temp_video_n = length_videonode(TempVideoNode_h);
                      if(temp_video_n >= 4)
                        sem_post(&videorec2playsem);
                     }
                   }
                 //����
                 pthread_mutex_unlock(&sync_s.video_play_lock);
           break;
  }
}
//-----------------------------------------------------------------------
//�Խ�����
void RecvTalkCall_Func(unsigned char *recv_buf, char *cFromIP)
{
  char wavFile[80];
  int i,j;
  int isAddrOK;
  unsigned char send_b[1520];
  int sendlength;
  uint32_t Ip_Int;
  char str[100];
  char IP_Group[15];

  //����״̬Ϊ����
  if((Local.Status == 3)||(Local.Status == 4))
   {
     	Local.ForceEndWatch = 1;  //�к���ʱ��ǿ�ƹؼ���
    	 //WatchEnd_Func();
    	 return;
   }

  //����״̬Ϊ����
  if(Local.Status == 0)
   {
//    gettimeofday(&tv1, NULL);
    //��ȡ�Է���ַ
    memcpy(Remote.Addr[0], recv_buf+9, 20);
    memcpy(Remote.IP[0], recv_buf+29, 4);

    if(recv_buf[57] == 1)
     {
      //�鿴�Ƿ��������鲥����
      DropMultiGroup(m_VideoSocket, NULL);

      Local.IP_Group[0] = recv_buf[58]; //�鲥��ַ
      Local.IP_Group[1] = recv_buf[59];
      Local.IP_Group[2] = recv_buf[60];
      Local.IP_Group[3] = recv_buf[61];
      sprintf(IP_Group, "%d.%d.%d.%d\0",
                     Local.IP_Group[0],Local.IP_Group[1],Local.IP_Group[2],Local.IP_Group[3]);
      AddMultiGroup(m_VideoSocket, IP_Group);
     }

    Ip_Int=inet_addr(cFromIP);
    memcpy(Remote.DenIP, &Ip_Int,4);
    printf("Remote.DenIP, %d.%d.%d.%d\0",
            Remote.DenIP[0],Remote.DenIP[1],Remote.DenIP[2],Remote.DenIP[3]);

    Local.Status = 2;  //״̬Ϊ���Խ�
    //��ʾ�Խ�����
    DisplayTalkPicWindow(1);
    //��ʼ������Ƶ
    StartPlayVideo(CIF_W, CIF_H);

    Local.RecordPic = 1;   //��������Ƭ
    Local.IFrameCount = 0; //I֡����
    Local.IFrameNo = 3;    //���ڼ���I֡

    sprintf(wavFile, "%sring1.wav\0", wavPath);
    StartPlayWav(wavFile, 1);
    
    Local.CallConfirmFlag = 1; //�������߱�־
    Local.Timer1Num = 0;
    Local.TimeOut = 0;       //���ӳ�ʱ,  ͨ����ʱ,  ���г�ʱ�����˽���
    Local.OnlineNum = 0;     //����ȷ�����
    Local.OnlineFlag = 1;

	strcpy(CallListAddr,Remote.Addr[0]);
	CallListAddr[20] = '\0';
    if((Remote.DenIP[0] == Remote.IP[0][0]) && (Remote.DenIP[1] == Remote.IP[0][1])
      && (Remote.DenIP[2] == Remote.IP[0][2]) &&(Remote.DenIP[3] == Remote.IP[0][3]))
      {
       		Remote.isDirect = 0;
       		strcpy(str, Remote.Addr[0]);
       		strcat(str, "  ֱͨ�Խ�����");
       		ShowStatusText(CALLX, CALLY, 3, cBlack, 1, 1, str, 0);
      }
    else
      {
       Remote.isDirect = 1;
       strcpy(str, Remote.Addr[0]);
       strcat(str, "  ��ת�Խ�����");
       ShowStatusText(CALLX, CALLY , 3, cBlack, 1, 1, str, 0);
      }
	    //��ɴ�����ٸ�Ӧ��
	    memcpy(send_b, recv_buf, 62);
	    send_b[7]=ASK;    //����
	    send_b[8]=CALLANSWER;//�Խ�Ӧ��
	    sendlength=62;
	    UdpSendBuff(m_VideoSocket, cFromIP, send_b , sendlength);  
   }
  //����Ϊæ
  else
   {
    memcpy(send_b, recv_buf, 57);
    send_b[7]=ASK;    //����
    send_b[8]=LINEUSE;//ռ��Ӧ��
    sendlength=57;
    UdpSendBuff(m_VideoSocket, cFromIP, send_b , sendlength);
    #ifdef _DEBUG
      printf("�Է�����Խ�����\n");
    #endif
   }

}
//-----------------------------------------------------------------------
//�Խ�ռ��Ӧ��
void RecvTalkLineUse_Func(unsigned char *recv_buf, char *cFromIP)
{
  int i,j;
  int isAddrOK;

  //����������
  pthread_mutex_lock (&Local.udp_lock);
  if(recv_buf[7] == ASK)   //Ӧ��
   for(i=0; i<UDPSENDMAX; i++)
    if(Multi_Udp_Buff[i].isValid == 1)
     if(Multi_Udp_Buff[i].m_Socket == m_VideoSocket)
      if(Multi_Udp_Buff[i].SendNum  < MAXSENDNUM)
       if((Multi_Udp_Buff[i].buf[6] == VIDEOTALK)||(Multi_Udp_Buff[i].buf[6] == VIDEOTALKTRANS))
        if(Multi_Udp_Buff[i].buf[7] == ASK)
         if(Multi_Udp_Buff[i].buf[8] == CALL)
          if(strcmp(Multi_Udp_Buff[i].RemoteHost, cFromIP) == 0)
           {
           	 Multi_Udp_Buff[i].isValid = 0;
      		 //     Local.Status = 0;  //��״̬Ϊ����
            	ShowStatusText(CALLX, CALLY , 3, cBlack, 1, 1, "�Է�ռ��", 0);

		Local.Status = 0;
		DisplayMainWindow(MainWindow);
#if 0		
           	 //��ʱ����ʾ��Ϣ��־
            	PicStatBuf.Type = 16;
            	PicStatBuf.Time = 0;
            	PicStatBuf.Flag = 1;
            #ifdef _DEBUG
              	printf("�յ��Խ�ռ��Ӧ��\n");
            #endif
#endif
            break;
           }
  //�򿪻�����
  pthread_mutex_unlock (&Local.udp_lock);
}
//-----------------------------------------------------------------------
//�Խ�����Ӧ��
void RecvTalkCallAnswer_Func(unsigned char *recv_buf, char *cFromIP)
{
  char wavFile[80];
  int i,j;
  int isAddrOK;
  uint32_t Ip_Int;
  //����������
  pthread_mutex_lock (&Local.udp_lock); //�յ�����Ӧ��� �Ƚϻ��������Ƿ���ǰ���͵ĶԽ�����
  if(recv_buf[7] == ASK)   //Ӧ��
   for(i=0; i<UDPSENDMAX; i++)
    if(Multi_Udp_Buff[i].isValid == 1)
     if(Multi_Udp_Buff[i].m_Socket == m_VideoSocket)
      if(Multi_Udp_Buff[i].SendNum  < MAXSENDNUM)
       if((Multi_Udp_Buff[i].buf[6] == VIDEOTALK)||(Multi_Udp_Buff[i].buf[6] == VIDEOTALKTRANS))
        if(Multi_Udp_Buff[i].buf[7] == ASK)
         if(Multi_Udp_Buff[i].buf[8] == CALL)
          if(strcmp(Multi_Udp_Buff[i].RemoteHost, cFromIP) == 0) //��ǰ�ĺ��жԽ�����
          {
            	Multi_Udp_Buff[i].isValid = 0; //��Ϊ��Ч
            	if((LocalCfg.Addr[0] == 'S')||(LocalCfg.Addr[0] == 'B')||(LocalCfg.Addr[0] == 'Z')
               		||(Remote.DenNum == 1))
             	{
              		Ip_Int=inet_addr(cFromIP);
              		memcpy(Remote.DenIP, &Ip_Int,4); //�Է�IP����Ƶ������IP
             	}
            	else if(Remote.DenNum > 1)
                {
               		Remote.DenIP[0] = Remote.GroupIP[0];
               		Remote.DenIP[1] = Remote.GroupIP[1];
               		Remote.DenIP[2] = Remote.GroupIP[2];
               		Remote.DenIP[3] = Remote.GroupIP[3];
              	}
				
            	//��ʼ¼����Ƶ
            	#ifdef _R2RVIDEO           //�����Խ���Ƶ
              	StartRecVideo(CIF_W, CIF_H);
            	#endif

            	Local.Status = 1;  			//״̬Ϊ���жԽ�
            	Local.CallConfirmFlag = 1;  //�������߱�־
            	Local.Timer1Num = 0;
            	Local.TimeOut   = 0;        //���ӳ�ʱ,  ͨ����ʱ,  ���г�ʱ�����˽���
            	Local.OnlineNum = 0;        //����ȷ�����
            	Local.OnlineFlag = 1;
            	printf("Local.Status = %d\n", Local.Status);

            	sprintf(wavFile, "%sring1.wav\0", wavPath);
            	StartPlayWav(wavFile, 1);

            	#ifdef _DEBUG
              	printf("�յ��Խ�����Ӧ��\n");
            	#endif
            	ShowStatusText(CALLX, CALLY , 3, cBlack, 1, 1, "�յ��Խ�����Ӧ��", 0);
            	break;
           }
  //�򿪻�����
  pthread_mutex_unlock (&Local.udp_lock);
}
//-----------------------------------------------------------------------
//�Խ���ʼͨ��  �ɱ��з��������з�Ӧ��
void RecvTalkCallStart_Func(unsigned char *recv_buf, char *cFromIP)
{
  int i,j;
  int isAddrOK;
  unsigned char send_b[1520];
  int sendlength;
  uint32_t Ip_Int;
  struct timeval tv1,tv2;
  char OsdContent[20];
  //ʱ��
  time_t t;

  Local.TalkTimeOut = TALKTIMEOUT; //ͨ���ʱ��
  //����Ϊ���з� Ӧ��
  if((Local.Status == 1)&&(recv_buf[7] == ASK))
   {
    memcpy(send_b, recv_buf, 57);
    send_b[7]=REPLY;    //Ӧ��
    sendlength=57;
    UdpSendBuff(m_VideoSocket, cFromIP, send_b , sendlength);

    ExitGroup(recv_buf);      //���������з��˳��鲥������

    //��ȡ���з���ַ
    memcpy(Remote.Addr[0], recv_buf+33, 20);
    memcpy(Remote.IP[0], recv_buf+53, 4);
    Remote.DenNum = 1;
        
    Ip_Int=inet_addr(cFromIP);
    memcpy(Remote.DenIP, &Ip_Int,4);

    StopPlayWavFile();  //�ر�����
    usleep(200*1000);    //��ʱ��Ϊ�˵ȴ�����������ɣ������������
    //����Ƶ¼�ơ����ţ���Ƶ����
    #ifdef _R2RVIDEO           //�����Խ���Ƶ
      StartPlayVideo(CIF_W, CIF_H);
    #endif
    StartRecAudio();
    StartPlayAudio();

	DisplayImageButton(&talkpic_button[4],IMAGEUP,0);
	Local.ShowRecPic = 1;
    Local.Status = 5;  //״̬Ϊ����ͨ��
    Local.TimeOut = 0;       //���ӳ�ʱ,  ͨ����ʱ,  ���г�ʱ�����˽���

    #ifdef _DEBUG
      printf("�Է���ʼͨ��\n");
    #endif
    ShowStatusText(CALLX, CALLY , 3, cBlack, 1, 1, "��ʼͨ��", 0);
    OpenOsd();   //��OSD
    sprintf(OsdContent, "%02d:%02d\0", Local.TimeOut/INTRPERSEC/60,
           (Local.TimeOut/INTRPERSEC)%60);
    OsdContent[2] = ',';
    OsdContent[5] = '\0';
    ShowOsd(OsdContent);
    time(&t);
    Local.call_tm_t = localtime(&t);
   }
  else  //����Ϊ���з� ����
   if(Local.Status == 2)
   {
    //����������
    pthread_mutex_lock (&Local.udp_lock);
    //��������
    if(recv_buf[7] == REPLY)
     for(i=0; i<UDPSENDMAX; i++)
      if(Multi_Udp_Buff[i].isValid == 1)
       if(Multi_Udp_Buff[i].m_Socket == m_VideoSocket)
        if(Multi_Udp_Buff[i].SendNum  < MAXSENDNUM)
         if((Multi_Udp_Buff[i].buf[6] == VIDEOTALK)||(Multi_Udp_Buff[i].buf[6] == VIDEOTALKTRANS))
          if(Multi_Udp_Buff[i].buf[7] == ASK)
           if(Multi_Udp_Buff[i].buf[8] == CALLSTART)
            if(strcmp(Multi_Udp_Buff[i].RemoteHost, cFromIP) == 0)
             {
              //�鿴�Ƿ��������鲥����
              DropMultiGroup(m_VideoSocket, NULL);

              Multi_Udp_Buff[i].isValid = 0;

          //    StopPlayWavFile();  //�ر�����
              
              //����Ƶ¼�ơ����ţ���Ƶ¼��
              #ifdef _R2RVIDEO           //�����Խ���Ƶ
                if((Remote.Addr[0][0] == 'S')||(Remote.Addr[0][0] == 'B')||(Remote.Addr[0][0] == 'Z'))
                 	 StartRecVideo(CIF_W, CIF_H);
              #endif
              StartRecAudio();
              StartPlayAudio();
              Local.Status = 6;  //״̬Ϊ����ͨ��
              Local.TimeOut = 0;       //���ӳ�ʱ,  ͨ����ʱ,  ���г�ʱ�����˽���

              //Local.RecordPic = 2;   //ͨ������Ƭ
              //Local.IFrameCount = 0; //I֡����
              //Local.IFrameNo = 2;    //���ڼ���I֡
              
              #ifdef _DEBUG
                printf("�Է�Ӧ�𱾻���ʼͨ��\n");
              #endif
             ShowStatusText(CALLX,CALLY , 3, cBlack, 1, 1, "��ʼͨ��", 0);
              OpenOsd();   //��OSD
              sprintf(OsdContent, "%02d:%02d\0", Local.TimeOut/INTRPERSEC/60,
                     (Local.TimeOut/INTRPERSEC)%60);
              OsdContent[2] = ',';
              OsdContent[5] = '\0';
              ShowOsd(OsdContent);
              time(&t);
              Local.call_tm_t=localtime(&t);

              break;
             }
    //�򿪻�����
    pthread_mutex_unlock (&Local.udp_lock);
   }
}
//-----------------------------------------------------------------------
//�Խ�����ȷ��
void RecvTalkCallConfirm_Func(unsigned char *recv_buf, char *cFromIP)
{
  int i,j;
  int isAddrOK;
  unsigned char send_b[1520];
  int sendlength;

  //����Ϊ���з�
  if(((Local.Status == 1)||(Local.Status == 5)||(Local.Status == 7)||(Local.Status == 9))
     &&(recv_buf[7] == ASK))
   {
    memcpy(send_b, recv_buf, 61);
    send_b[7]=REPLY;    //Ӧ��
    sendlength=61;
    UdpSendBuff(m_VideoSocket, cFromIP, send_b , sendlength);
    Local.CallConfirmFlag = 1;
    #ifdef _DEBUG
   //   printf("�յ��Խ�����ȷ��\n");
    #endif
   }
  else  //����Ϊ���з�
   if(((Local.Status == 2)||(Local.Status == 6)||(Local.Status == 8)||(Local.Status == 10))
      &&(recv_buf[7] == REPLY))
   {
    Local.CallConfirmFlag = 1;
    #ifdef _DEBUG
//      printf("�յ��Է�Ӧ�𱾻��Խ�����ȷ��\n");
    #endif
   }
}
//-----------------------------------------------------------------------
//�Խ����н���
void RecvTalkCallEnd_Func(unsigned char *recv_buf, char *cFromIP)
{
	int i,j;
	int isAddrOK;
	unsigned char send_b[1520];
	int sendlength;
	//��������
	if(((Local.Status == 1)||(Local.Status == 2)||(Local.Status == 5)||(Local.Status == 6)
	||(Local.Status == 7)||(Local.Status == 8)||(Local.Status == 9)||(Local.Status == 10))
	&&(recv_buf[7] == ASK))
	{
		Local.OnlineFlag = 0;
		Local.CallConfirmFlag = 0; //�������߱�־

		memcpy(send_b, recv_buf, 57);
		send_b[7]=REPLY;    //Ӧ��
		sendlength=57;
		UdpSendBuff(m_VideoSocket, cFromIP, send_b , sendlength);

		ExitGroup(recv_buf);      //���������з��˳��鲥������
		TalkEnd_ClearStatus();
		if(Local.Status == 10)
		{
			if(Local.CurrentWindow == TalkPicWindow)
				ShowStatusText(50, 130 , 3, cBlack, 1, 1, "������Ӱ", 0);
		}
		else
		{
			if(Local.CurrentWindow == TalkPicWindow)
				ShowStatusText(50, 130 , 3, cBlack, 1, 1, "�Է������Խ�", 0);
		}
#ifdef _DEBUG
		printf("�Է������Խ�\n");
#endif
	}
	else  //��������
	if((Local.Status == 1)||(Local.Status == 2)||(Local.Status == 5)||(Local.Status == 6)
	||(Local.Status == 7)||(Local.Status == 8)||(Local.Status == 9)||(Local.Status == 10))   
	{

	Local.OnlineFlag = 0;
	Local.CallConfirmFlag = 0; //�������߱�־

	//����������
	pthread_mutex_lock (&Local.udp_lock);
	//��������
	if(recv_buf[7] == REPLY)
	for(i=0; i<UDPSENDMAX; i++)
	if(Multi_Udp_Buff[i].isValid == 1)
	if(Multi_Udp_Buff[i].m_Socket == m_VideoSocket)
	if(Multi_Udp_Buff[i].SendNum  < MAXSENDNUM)
	if((Multi_Udp_Buff[i].buf[6] == VIDEOTALK)||(Multi_Udp_Buff[i].buf[6] == VIDEOTALKTRANS))
	if(Multi_Udp_Buff[i].buf[7] == ASK)
	if(Multi_Udp_Buff[i].buf[8] == CALLEND)
	if(strcmp(Multi_Udp_Buff[i].RemoteHost, cFromIP) == 0)
	{
		Multi_Udp_Buff[i].isValid = 0;
		TalkEnd_ClearStatus();

		if(Local.CurrentWindow == TalkPicWindow)
		ShowStatusText(50, 130 , 3, cWhite, 1, 1, "���������Խ�", 0);
#ifdef _DEBUG
		printf("�Է�Ӧ�𱾻������Խ�\n");
#endif
		break;
	}
	//�򿪻�����
	pthread_mutex_unlock (&Local.udp_lock);
   }
}
//-----------------------------------------------------------------------
//�Խ���������״̬�͹ر�����Ƶ
void TalkEnd_ClearStatus(void)
{

    StopPlayWavFile();  //�ر�����

    //�鿴�Ƿ��������鲥����
    DropMultiGroup(m_VideoSocket, NULL);
    CloseOsd(); //�ر�OSD
    switch(Local.Status)
     {
		case 1: //��������
#ifdef _R2RVIDEO           //�����Խ���Ƶ
		StopRecVideo();
#endif      
		break;
		case 2: //��������
		StopPlayVideo();

		if(Local.HavePicRecorded == 1)  //����Ƭ��¼��
		{
		Local.HavePicRecorded = 0;
		SaveToFlash(5);       //δ����
		}
		break;
		case 5: //��������ͨ��
#ifdef _R2RVIDEO           //�����Խ���Ƶ
		StopRecVideo();
#endif
		StopPlayVideo();
		StopRecAudio();
		StopPlayAudio();
		//ShowVideoWindow();
		break;
		case 6: //��������ͨ��
#ifdef _R2RVIDEO           //�����Խ���Ƶ
		StopRecVideo();
#endif
		StopPlayVideo();
		StopRecAudio();
		StopPlayAudio();
		//ShowVideoWindow();
		if(Local.HavePicRecorded == 1)  //����Ƭ��¼��
		{
			Local.HavePicRecorded = 0;
			SaveToFlash(6);    //ͨ��
		}             
        break;
     }
    //��ʱ����ʾ��Ϣ��־
//    PicStatBuf.Type = Local.CurrentWindow;
	DisplayMainWindow(0);
	Local.Status = 0;
//    PicStatBuf.Time = 0;
//    PicStatBuf.Flag = 1;
}
//-----------------------------------------------------------------------
//Զ�̿���
void  RecvTalkRemoteOpenLock_Func(unsigned char *recv_buf, char *cFromIP)
{
  int i,j;
  int isAddrOK;
  unsigned char send_b[1520];
  int sendlength;

  #ifdef _ZHUHAIJINZHEN      //�麣����  ����ʱ�ɿ���
  if((Local.Status == 2)||(Local.Status == 6))  //״̬Ϊ���Խ� �򱻽�ͨ��
  #else
  if(Local.Status == 6)  //״̬Ϊ���Խ�
  #endif
   {
    //����������
    pthread_mutex_lock (&Local.udp_lock);
    //��������
    if(recv_buf[7] == REPLY)
     for(i=0; i<UDPSENDMAX; i++)
      if(Multi_Udp_Buff[i].isValid == 1)
       if(Multi_Udp_Buff[i].m_Socket == m_VideoSocket)
        if(Multi_Udp_Buff[i].SendNum  < MAXSENDNUM)
         if((Multi_Udp_Buff[i].buf[6] == VIDEOTALK)||(Multi_Udp_Buff[i].buf[6] == VIDEOTALKTRANS))
          if(Multi_Udp_Buff[i].buf[7] == ASK)
           if(Multi_Udp_Buff[i].buf[8] == REMOTEOPENLOCK)
            if(strcmp(Multi_Udp_Buff[i].RemoteHost, cFromIP) == 0)
             {
              Multi_Udp_Buff[i].isValid = 0;
              #ifdef _ZHUHAIJINZHEN      //�麣����  ����ʱ�ɿ���  ͨ��ʱ����2���Ҷ�
              if(Local.Status == 6)  //״̬Ϊ����ͨ��
               {
                if((Local.TimeOut + 2*(1000/INTRTIME)) < TALKTIMEOUT)
                  Local.TalkTimeOut = Local.TimeOut + 2*(1000/INTRTIME);
                else
                  Local.TalkTimeOut = TALKTIMEOUT;
               }
              #endif
              #ifdef _DEBUG
                printf("�Է�Ӧ�𱾻�Զ�̿���\n");
              #endif
              break;
             }
    //�򿪻�����
    pthread_mutex_unlock (&Local.udp_lock);
   }
}
//-----------------------------------------------------------------------
//�Ŵ�(720*480)
void RecvTalkZoomOut_Func(unsigned char *recv_buf, char *cFromIP)
{
  int i,j;
  int isAddrOK;
  unsigned char send_b[1520];
  int sendlength;

  //��������
  if(((Local.Status == 5)||(Local.Status == 6))
     &&(recv_buf[7] == ASK))
   {
/*    memcpy(send_b, recv_buf, 57);
    send_b[7]=REPLY;    //Ӧ��
    sendlength=57;
    UdpSendBuff(m_VideoSocket, cFromIP, send_b , sendlength);
    StopRecVideo();   //352*240
    StartRecVideo(720, 480);  //720*480
    #ifdef _DEBUG
      printf("�Է��Ŵ�ͼ��\n");
    #endif    */
   }
  else  //��������
   if((Local.Status == 5)||(Local.Status == 6))
   {
    //����������
    pthread_mutex_lock (&Local.udp_lock);
    //��������
    if(recv_buf[7] == REPLY)
     for(i=0; i<UDPSENDMAX; i++)
      if(Multi_Udp_Buff[i].isValid == 1)
       if(Multi_Udp_Buff[i].m_Socket == m_VideoSocket)
        if(Multi_Udp_Buff[i].SendNum  < MAXSENDNUM)
         if((Multi_Udp_Buff[i].buf[6] == VIDEOTALK)||(Multi_Udp_Buff[i].buf[6] == VIDEOTALKTRANS))
          if(Multi_Udp_Buff[i].buf[7] == ASK)
           if(Multi_Udp_Buff[i].buf[8] == ZOOMOUT)
            if(strcmp(Multi_Udp_Buff[i].RemoteHost, cFromIP) == 0)
             {
              Multi_Udp_Buff[i].isValid = 0;
              Local.ZoomInOutFlag = 1;   //���ڷŴ���С��
              if(Local.PlayPicSize == 1)
               {
                if(Local.Status != 0)
                  StopPlayVideo();   //352*288
                //����  �ڶ�ҳ  ��Ƶ720*480
                memset(fbmem + f_data.buf_len, 16, f_data.uv_offset);
                memset(fbmem + f_data.buf_len + f_data.uv_offset, 128, f_data.uv_offset/4);
                memset(fbmem + f_data.buf_len + f_data.uv_offset*5/4, 128, f_data.uv_offset/4);
                if(Local.Status != 0)
                  StartPlayVideo(D1_W, D1_H);  //720*480
                //for(j=0; j<2; j++)
                //  DisplayImageButton(&zoom_button[j], IMAGEUP, 1);
                Local.ZoomInOutFlag = 0;   //���ڷŴ���С��
                #ifdef _DEBUG
                  printf("�Է�Ӧ�𱾻��Ŵ�ͼ��\n");
                #endif
               }
              break;
             }
    //�򿪻�����
    pthread_mutex_unlock (&Local.udp_lock);
   }
}
//-----------------------------------------------------------------------
//��С(352*288)
void RecvTalkZoomIn_Func(unsigned char *recv_buf, char *cFromIP)
{
  int i,j;
  int isAddrOK;
  unsigned char send_b[1520];
  int sendlength;

  //��������
  if(((Local.Status == 5)||(Local.Status == 6))
     &&(recv_buf[7] == ASK))
   {
/*    memcpy(send_b, recv_buf, 57);
    send_b[7]=REPLY;    //Ӧ��
    sendlength=57;
    UdpSendBuff(m_VideoSocket, cFromIP, send_b , sendlength);
    StopRecVideo();   //720*480
    StartRecVideo(CIF_W, CIF_H);  //352*288
    #ifdef _DEBUG
      printf("�Է���Сͼ��\n");
    #endif           */
   }
  else  //��������
   if((Local.Status == 5)||(Local.Status == 6))
   {
    //����������
    pthread_mutex_lock (&Local.udp_lock);
    //��������
    if(recv_buf[7] == REPLY)
     for(i=0; i<UDPSENDMAX; i++)
      if(Multi_Udp_Buff[i].isValid == 1)
       if(Multi_Udp_Buff[i].m_Socket == m_VideoSocket)
        if(Multi_Udp_Buff[i].SendNum  < MAXSENDNUM)
         if((Multi_Udp_Buff[i].buf[6] == VIDEOTALK)||(Multi_Udp_Buff[i].buf[6] == VIDEOTALKTRANS))
          if(Multi_Udp_Buff[i].buf[7] == ASK)
           if(Multi_Udp_Buff[i].buf[8] == ZOOMIN)
            if(strcmp(Multi_Udp_Buff[i].RemoteHost, cFromIP) == 0)
             {
              Multi_Udp_Buff[i].isValid = 0;
              Local.ZoomInOutFlag = 1;   //���ڷŴ���С��
              if(Local.PlayPicSize == 2)
               {
                //��ʾ��Ƶ����
                //ShowVideoWindow();
                if(Local.Status != 0)
                  StopPlayVideo();   //720*480
                if(Local.Status != 0)
                  StartPlayVideo(CIF_W, CIF_H);  //352*288
                Local.ZoomInOutFlag = 0;   //���ڷŴ���С��
                #ifdef _DEBUG
                  printf("�Է�Ӧ�𱾻���Сͼ��\n");
                #endif
               }
              break;
             }
    //�򿪻�����
    pthread_mutex_unlock (&Local.udp_lock);
   }      
}
//-----------------------------------------------------------------------
//�Խ�����
void RecvTalkCallUpDown_Func(unsigned char *recv_buf, char *cFromIP, int length)
{
  int i,j;
  int isAddrOK;
  unsigned char send_b[1520];
  short PackIsExist; //���ݰ��ѽ��ձ�־
  short FrameIsNew;  //���ݰ��Ƿ�����֡�Ŀ�ʼ
  TempVideoNode1 * tmp_videonode;
  TempAudioNode1 * tmp_audionode;
  int isFull;
  struct talkdata1 talkdata;
//  struct tempvideobuf1 InitVideoCon;
  if((Local.Status == 1)||(Local.Status == 2)||(Local.Status == 5)||(Local.Status == 6)
     ||(Local.Status == 7)||(Local.Status == 8)||(Local.Status == 9)||(Local.Status == 10))  //״̬Ϊ�Խ�
   switch(recv_buf[61])
   {
    case 1://��Ƶ
           //֡���
   //       if(AudioMuteFlag == 0)
            //д��Ӱ���ݺ���, ����  ����  ���� 1--��Ƶ  2--��ƵI  3--��ƵP
           if((Local.Status == 5)||(Local.Status == 6)||(Local.Status == 10))
            {
             memcpy(&talkdata, recv_buf + 9, sizeof(talkdata));
             if(temp_audio_n >= G711NUM)
              {
               temp_audio_n = G711NUM;
               #ifdef _DEBUG
                 printf("temp_audio is full\n");
               #endif
              }
             else
              {
               tmp_audionode = find_audionode(TempAudioNode_h, talkdata.Frameno, talkdata.CurrPackage);
               if(tmp_audionode == NULL)
                {
                 isFull = creat_audionode(TempAudioNode_h, talkdata, recv_buf, length);
                 PackIsExist = 0;
                }
               else
                 PackIsExist = 1;

             if(PackIsExist == 0)
              {
               TimeStamp.OldCurrAudio = TimeStamp.CurrAudio; //��һ�ε�ǰ��Ƶʱ��
               TimeStamp.CurrAudio = talkdata.timestamp;

             //  temp_audio_n ++;
               temp_audio_n = length_audionode(TempAudioNode_h);
               if(temp_audio_n >= 6) //VPLAYNUM/2 4���� 128ms
                {
                   sem_post(&audiorec2playsem);
                }
              }
            }
           } 
           break;
    case 2://��Ƶ  I֡  352*288
    case 3://��Ƶ  P֡  352*288
    case 4://��Ƶ  I֡  720*480
    case 5://��Ƶ  P֡  720*480
           //  2 ���жԽ�  3 ����  5 ����ͨ��
               //6 ����ͨ��  8 ������Ӱ����Ԥ��
               //10 ������Ӱ����
           if((Local.Status == 2)||(Local.Status == 3)||(Local.Status == 5)||(Local.Status == 6)||(Local.Status == 8)||(Local.Status == 10))
            {
                  //֡���
                  memcpy(&talkdata, recv_buf + 9, sizeof(talkdata));
                  PackIsExist = 0;
                  FrameIsNew = 1;

                  if(temp_video_n >= MP4VNUM)
                   {
                    temp_video_n = MP4VNUM;
                    #ifdef _DEBUG
                      printf("temp_video is full\n");
                    #endif
                   }
                  else
                   {
                    //����videoplaybuf[vpbuf_iget].buffer
                    pthread_mutex_lock(&sync_s.video_play_lock);
                    tmp_videonode = find_videonode(TempVideoNode_h, talkdata.Frameno, talkdata.CurrPackage);
                    if(tmp_videonode == NULL)
                      isFull = creat_videonode(TempVideoNode_h, talkdata, recv_buf, length);
                    else
                      isFull = add_videonode(tmp_videonode, talkdata, recv_buf, length);

                    if(isFull == 1)
                     {
                           TimeStamp.OldCurrVideo = TimeStamp.CurrVideo; //��һ�ε�ǰ��Ƶʱ��
                           TimeStamp.CurrVideo = talkdata.timestamp;

                        //   temp_video_n ++;
                           temp_video_n = length_videonode(TempVideoNode_h);
                           if(temp_video_n >= 3)   //3֡ 40*3 120
                            {
                               sem_post(&videorec2playsem);
                            }
                    }
                   //����
                  pthread_mutex_unlock(&sync_s.video_play_lock);
                 }
             }
    break;
  }
}
//-----------------------------------------------------------------------
void ForceIFrame_Func(void)  //ǿ��I֡
{
  int i;

  for(i=0; i<UDPSENDMAX; i++)
   if(Multi_Udp_Buff[i].isValid == 0)
    {
     //����������
     pthread_mutex_lock (&Local.udp_lock);
     //ֻ����һ��
     Multi_Udp_Buff[i].SendNum = 5;
     Multi_Udp_Buff[i].m_Socket = m_VideoSocket;
     Multi_Udp_Buff[i].CurrOrder = 0;
     sprintf(Multi_Udp_Buff[i].RemoteHost, "%d.%d.%d.%d\0",Remote.DenIP[0],
             Remote.DenIP[1],Remote.DenIP[2],Remote.DenIP[3]);
     //ͷ��
     memcpy(Multi_Udp_Buff[i].buf, UdpPackageHead, 6);
     //����  ,�����㲥����
     if((Local.Status == 1)||(Local.Status == 2)||(Local.Status == 5)||(Local.Status == 6)
       ||(Local.Status == 7)||(Local.Status == 8)||(Local.Status == 9)||(Local.Status == 10))
         Multi_Udp_Buff[i].buf[6] = VIDEOTALK;
     if((Local.Status == 3)||(Local.Status == 4))
         Multi_Udp_Buff[i].buf[6] = VIDEOWATCH;
     Multi_Udp_Buff[i].buf[7] = ASK;    //����
     Multi_Udp_Buff[i].buf[8] = FORCEIFRAME;    //FORCEIFRAME

     memcpy(Multi_Udp_Buff[i].buf+9,LocalCfg.Addr,20);
     memcpy(Multi_Udp_Buff[i].buf+29,LocalCfg.IP,4);
     memcpy(Multi_Udp_Buff[i].buf+33,Remote.Addr[0],20);
     memcpy(Multi_Udp_Buff[i].buf+53,Remote.IP[0],4);

     Multi_Udp_Buff[i].nlength = 57;
     Multi_Udp_Buff[i].DelayTime = 100;
     Multi_Udp_Buff[i].isValid = 1;

     //�򿪻�����
     pthread_mutex_unlock (&Local.udp_lock);

     sem_post(&multi_send_sem);
     break;
    }
}
//-----------------------------------------------------------------------
void ExitGroup(unsigned char *buf)      //���������з��˳��鲥������
{
}
//-----------------------------------------------------------------------
//�����豸
void RecvFindEquip_Func(unsigned char *recv_buf, char *cFromIP, int length, int m_Socket)
{
  int i,j;
  int newlength;
  int isAddrOK;
  //ʱ��
  time_t t;
  struct tm *tm_t;
  unsigned char send_b[1520];
  int sendlength;
//  char addr[20];
  i = 0;
  isAddrOK = 1;

  for(j=32; j<32+12; j++)
   if(LocalCfg.Addr[j-32] != recv_buf[j])
    {
     isAddrOK = 0;
     break;
    }
  //��ַƥ��
  if(isAddrOK == 1)
   {
    memcpy(send_b, recv_buf, length);
    send_b[7] = REPLY;    //Ӧ��
    sendlength = length;
    UdpSendBuff(m_Socket, cFromIP, send_b , sendlength);
   }
}
//---------------------------------------------------------------------------
