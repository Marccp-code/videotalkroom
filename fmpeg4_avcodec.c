/* fmpeg4_avcodec.c
   It is sample code for ffmpeg application usage, to define AVCodec structure.
   Note it's not a part of fmpeg driver, but collect int the same folder
 */
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <linux/videodev.h>
#include <errno.h>
#include <linux/fb.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/un.h>
#include <signal.h>
#include <semaphore.h>       //sem_t

#define CommonH
#include "common.h"

#include "fmpeg4_avcodec.h"
#include "ratecontrol.h"

#define MP4_ROIX 0
#define MP4_ROIY 0
#define FMPEG4_DECODER_DEV  "/dev/fdec"
#define FMPEG4_ENCODER_DEV  "/dev/fenc"
#define VIDEO_PICTURE_QUEUE_SIZE_MAX 20


int fmpeg4_dec_fd=0;
int fmpeg4_enc_fd=0;

int                     video_fd=-1;
struct video_mbuf       gb_buffers;
struct video_mmap       gb_buf;
struct video_capability video_cap;
struct video_window     win; 
struct video_tuner      tuner;
unsigned char           *video_buf_virt;
unsigned char           *out_virt_buffer;
//unsigned char           *mmap_addr;
unsigned int            mmap_done=0;
//int                     cap_width,cap_height;
int                     frame_num = 0;
int                     fidx=0;
RateControl     ratec;

//short  bypass;
pthread_t  bypass_thread;
void bypassthread_func(video_profile  video_setting);
void our_bypass(int n_width, int n_height);
void our_bypass_off(void);
int ourbypassflag;

/*
#################################################
    capture
#################################################
 */
struct fcap_frame_buff fcap_frame_buff_t;


//��Ƶ
struct videobuf1 videorecbuf;    //��Ƶ�ɼ����λ���  ����?

int vpbuf_iput; // ��Ƶ���Ż��λ������ĵ�ǰ����λ��
int vpbuf_iget; // ��Ƶ���Ż������ĵ�ǰȡ��λ��
int vpbuf_n; // ��Ƶ���Ż��λ������е�Ԫ��������
struct videoplaybuf1 videoplaybuf[VPLAYNUM];    //��Ƶ���Ż���,���ɻ���

int video_rec_flag;
pthread_t video_rec_deal_thread;      //��Ƶ�ɼ����ݴ����߳�
pthread_t video_rec_thread;      //��Ƶ�ɼ��߳�
void video_rec_deal_thread_func(void);
void video_rec_thread_func(void);
void StartRecVideo(int width, int height);
void StopRecVideo(void);

int video_direct_rec_flag;
pthread_t video_direct_rec_thread;     
void video_direct_rec_thread_func(void);
void StartDirectVideo(int width, int height);
void StopDirectVideo(void);


int video_play_flag;
extern int audio_play_flag;
pthread_t video_play_deal_thread;     //��Ƶ�������ݴ����߳�
pthread_t video_play_thread;     //��Ƶ�����߳�

void video_play_deal_thread_func(void);
void video_play_thread_func(void);

sem_t videoplaysem;
sem_t videorecsem;
sem_t videorec2playsem;
struct _SYNC sync_s;

struct timeval ref_time;  //��׼ʱ��,��Ƶ����Ƶ��һ֡
//�������
video_profile   enc_video_setting;
//�������
video_profile   dec_video_setting;

extern sem_t audioplaysem;
extern struct audiobuf1 playbuf;    //��Ƶ���Ż��λ���

int curr_video_timestamp;       //��ǰ�Ѳ�����Ƶ֡ʱ���
int curr_audio_timestamp;       //��ǰ�Ѳ�����Ƶ֡ʱ���


#define USE_MMAP
void StartPlayVideo(int width, int height);
void StopPlayVideo(void);
//Ϊ��ֹ��β������´���
int VideoRecIsStart=0;
int VideoPlayIsStart=0;

//Ŀ¼���ļ�����
// *h�����ͷ����ָ�룬*pָ��ǰ����ǰһ����㣬*sָ��ǰ���

TempVideoNode1 * init_videonode(void); //��ʼ��������ĺ���
//������������������
int length_videonode(TempVideoNode1 *h);
//����������β�����
int creat_videonode(TempVideoNode1 *h, struct talkdata1 talkdata,
        unsigned char *r_buf , int r_length);
int add_videonode(TempVideoNode1 *p, struct talkdata1 talkdata,
        unsigned char *r_buf , int r_length);
//��Ӱ����
int creat_leavemovienode(TempVideoNode1 *h, uint32_t rframeno, uint32_t rtimestamp,
        unsigned short rframe_flag, unsigned char *r_buf , int r_length);
//����������ɾ������
int delete_videonode(TempVideoNode1 *p);
int delete_all_videonode(TempVideoNode1 *h);
int delete_lost_videonode(TempVideoNode1 *h, uint32_t currframeno, uint32_t currtimestamp); //ɾ����ȫ֡
//�������������Һ���
TempVideoNode1 * find_videonode(TempVideoNode1 *h, int currframeno, int currpackage);
//�������ϵ�֡
TempVideoNode1 * search_videonode(TempVideoNode1 *h);
//
int free_videonode(TempVideoNode1 *h);
//---------------------------------------------------------------------------
void fast_memcpy (uint8_t *to, uint8_t *from, size_t count)
{
    register size_t n = (count + 7) / 8;    /* count > 0 assumed */
    uint8_t c = count % 8;
    switch (c)
    {
    case 0:       do { *to++ = *from++;
    case 7:        *to++ = *from++;
    case 6:        *to++ = *from++;
    case 5:        *to++ = *from++;
    case 4:        *to++ = *from++;
    case 3:        *to++ = *from++;
    case 2:        *to++ = *from++;
    case 1:        *to++ = *from++;
              } while (--n > 0);
    }
}
//---------------------------------------------------------------------------
int xvid_decoder_init(video_profile *video_setting)
{
    int i,j,k;

    int rect1width;    
    int rect1height;
    int yTop,xLeft;
    char jpgfilename[80];    

    FMP4_DEC_PARAM  tDecParam;

    if(fmpeg4_dec_fd==0)
	    fmpeg4_dec_fd=open(FMPEG4_DECODER_DEV,O_RDWR);
	    
    if(fmpeg4_dec_fd<=0) {
	    printf("Fail to open %s\n",FMPEG4_DECODER_DEV);
	    return -1;
    }
//   printf("screensize=%d",screensize);
#ifdef USE_MMAP
  //��Ƶ���Ż���
  vpbuf_iput = 0; // ��Ƶ���Ż��λ������ĵ�ǰ����λ��
  vpbuf_iget = 0; // ��Ƶ���Ż������ĵ�ǰȡ��λ��
  vpbuf_n = 0; // ��Ƶ���Ż��λ������е�Ԫ��������
  for(i=0; i<VPLAYNUM; i++)
   {
    videoplaybuf[i].isUse = 0;
    videoplaybuf[i].frameno = 0;
    videoplaybuf[i].timestamp = 0;
    if(Local.PlayPicSize == 1)  //CIF
     {
      videoplaybuf[i].buffer = (int)mmap(0,(dec_video_setting.framewidth*dec_video_setting.frameheight*3)/2,
                    PROT_READ|PROT_WRITE,MAP_SHARED,fmpeg4_dec_fd, 0);
      if(videoplaybuf[i].buffer <= 0)
       {
         #ifdef _DEBUG
           printf("videoplaybuf.buffer mmap error\n");
         #endif
         return -1;
       }
     }
    else                       //D1
     {
      memset(fbmem + f_data.buf_len, 16, f_data.uv_offset);
      memset(fbmem + f_data.buf_len + f_data.uv_offset, 128, f_data.uv_offset/4);
      memset(fbmem + f_data.buf_len + f_data.uv_offset*5/4, 128, f_data.uv_offset/4);
     }

    yTop = 0;
    xLeft = 0;
    rect1width = 352;
    rect1height = 240;
   }
#endif

    tDecParam.u32API_version = 0x00020002;//MP4VER;//0x00020002;
    tDecParam.u32MaxWidth = video_setting->width;
    tDecParam.u32MaxHeight = video_setting->height;
    tDecParam.u32FrameWidth = video_setting->framewidth;
    tDecParam.u32FrameHeight = video_setting->frameheight;

    if ( ioctl(fmpeg4_dec_fd,FMPEG4_IOCTL_DECODE_INIT,&tDecParam) < 0 ) {
    	close(fmpeg4_dec_fd);
        #ifdef _DEBUG
          printf("Error to set FMPEG4_IOCTL_DECODE_INIT\n");
        #endif  
	return -1;
    }
	
    return 0;
}
//---------------------------------------------------------------------------
int xvid_decoder_end(video_profile *video_setting)
{
    int i;
    if(fmpeg4_dec_fd)     {
    #ifdef USE_MMAP

      //��Ƶ���Ż���
      vpbuf_iput = 0; // ��Ƶ���Ż��λ������ĵ�ǰ����λ��
      vpbuf_iget = 0; // ��Ƶ���Ż������ĵ�ǰȡ��λ��
      vpbuf_n = 0; // ��Ƶ���Ż��λ������е�Ԫ��������
      for(i=0; i<VPLAYNUM; i++)
       {
        videoplaybuf[i].isUse = 0;
        videoplaybuf[i].frameno = 0;
        videoplaybuf[i].timestamp = 0;
        if(Local.PlayPicSize == 1)  //CIF
          munmap(videoplaybuf[i].buffer,(dec_video_setting.framewidth*dec_video_setting.frameheight*3)/2);
       }
        #endif
        close(fmpeg4_dec_fd);
    }
    fmpeg4_dec_fd=0;


#ifdef SHOW_PERFORMANCE_EVALUATION_RESULT	
//    s->nsecs = time_stop(&(s->t1)) / 1000;
//    printf("spend %lld milliseconds with toal frame count %d = %f(frames/per second)\n", s->nsecs,s->pic_count,(float)((float)s->pic_count)/((float)s->nsecs/(float)1000.0));
//    printf("MP4 decoding is done. %d frames are encoded\n",s->pic_count);	
#endif		
    return 0;
}
//---------------------------------------------------------------------------
int xvid_decode(video_profile *video_setting, void *data, int *got_picture, uint8_t *buf, int buf_size)
{
    int x,y;
    fmpeg4_parm_t   parm;
    x = 32;
    y = 0;
    if((SCRWIDTH == 800)&&(SCRHEIGHT == 600))
      y = 64;
    parm.input_va_y=(unsigned int)buf;                     //input bit stream
    if(Local.PlayPicSize == 1)  //CIF
     {	   
         parm.output_va_y = /*(unsigned int)*/(data);
         parm.output_va_u = /*(unsigned int)*/(data+video_setting->framewidth*video_setting->frameheight);
         parm.output_va_v = /*(unsigned int)*/(data+video_setting->framewidth*video_setting->frameheight*5/4);
     }
    else                     //D1
     {
      parm.output_va_y = /*(unsigned int)*/(data + x + y*SCRWIDTH);
      parm.output_va_u = /*(unsigned int)*/(data+f_data.uv_offset + x/2 + y*SCRWIDTH/4);
      parm.output_va_v = /*(unsigned int)*/(data+f_data.uv_offset*5/4 + x/2 + y*SCRWIDTH/4);
     }
    parm.length=buf_size;
    if ( ioctl(fmpeg4_dec_fd,FMPEG4_IOCTL_DECODE_FRAME,&parm) < 0 ) {
         close(fmpeg4_dec_fd);
         #ifdef _DEBUG
           printf("Error to set FMPEG4_IOCTL_DECODE_FRAME\n");
         #endif  
	return -1;
    }
    *got_picture=parm.got_picture;
    //getchar();
    if(*got_picture==0)   {
        #ifdef _DEBUG
          printf("Bad MPEG4 Decoding!\n");
        #endif  
        return -1;
    }
	
    #ifdef SHOW_PERFORMANCE_EVALUATION_RESULT
//    s->pic_count++;
    #endif
	
    return parm.length;
}
//---------------------------------------------------------------------------
int mpeg4_decode_init(int dec_width, int dec_height, int framewidth, int frameheight, int xleft, int ytop)
{
  int ret;
  int i,j;

  int blength;
  int readsize;

  dec_video_setting.qmax = 31;
  dec_video_setting.qmin = 1;
  dec_video_setting.quant = 4;
  if(Local.PlayPicSize == 1)
    dec_video_setting.bit_rate = 512;
  else
    dec_video_setting.bit_rate = 1024;
  dec_video_setting.width = dec_width;       //ԭʼ�ߴ�
  dec_video_setting.height = dec_height;
  dec_video_setting.framewidth = framewidth; //��ʾ�ߴ�
  dec_video_setting.frameheight = frameheight;
  dec_video_setting.framerate = 25;
  dec_video_setting.frame_rate_base = 1;
  dec_video_setting.gop_size = 50;

  dec_video_setting.roi_x = MP4_ROIX;
  dec_video_setting.roi_y = MP4_ROIY;
  dec_video_setting.roi_width = 320;
  dec_video_setting.roi_height = 240;

  if(xvid_decoder_init(&dec_video_setting)<0)
     return -1;

    return 0;
}
//---------------------------------------------------------------------------
void StartPlayVideo(int width, int height)
{
  int i, j;
  pthread_attr_t attr;
  int frame_width, frame_height;  
  char OsdContent[20];
  if(VideoPlayIsStart == 0)
   {
    TimeStamp.OldCurrVideo = 0;       //��һ�ε�ǰ��Ƶʱ��
    TimeStamp.CurrVideo = 0;

    Local.RecordPic = 0;   //ͨ������Ƭ
    Local.IFrameCount = 0; //I֡����
    Local.IFrameNo = 0;    //���ڼ���I֡
    Local.HavePicRecorded = 0;  //����Ƭ��¼��
    Local.HavePicRecorded_flag = 0; 

    sprintf(OsdContent, "%02d:%02d\0", Local.TimeOut/INTRPERSEC/60,
           (Local.TimeOut/INTRPERSEC)%60);
    OsdContent[2] = ',';
    OsdContent[5] = '\0';
    ShowOsd(OsdContent);
//    width = 720;
//    height = 240;
    if((width == D1_W)&&(height == D1_H))
     {
      Local.PlayPicSize = 2;      //720*480
      if(Local.CurrFbPage != 1)
       {
        Local.CurrFbPage = 1;
        fb_setpage(fbdev, Local.CurrFbPage);
        TalkOsd();  //ͨ���ͼ���ʱOSD��ʾ
       }
      frame_width = SCRWIDTH;
      frame_height = SCRHEIGHT;
     }
    else
     {
      Local.PlayPicSize = 1;      //352*240
      frame_width = width;
      frame_height = height;
     }
    //��Ƶ���뼰����
    if(mpeg4_decode_init(width, height, frame_width, frame_height, 0, 0) !=0 )
     {
      printf("mpeg4_decode_init error!\\n");
      return;
     }                  
    temp_video_n = 0;
    //��Ƶ���ջ�������
    delete_all_videonode(TempVideoNode_h);

    if(sem_init(&videoplaysem,0,0) == -1)
      printf("cannot init videoplaysem\n");
    if(sem_init(&videorec2playsem,0,0) == -1)
      printf("cannot init videorec2playsem\n");
    if(pthread_mutex_init (&sync_s.video_play_lock, NULL) == -1)
      printf("cannot init sync_s.video_play_lock\n");

    curr_video_timestamp = 0;       //��ǰ�Ѳ�����Ƶ֡ʱ���
    curr_audio_timestamp = 0;       //��ǰ�Ѳ�����Ƶ֡ʱ���
    //�鿴ͬ�������߳��Ƿ��Ѵ���
 //   sync_play_init();

    video_play_flag = 1;  //��Ƶ���ű�־
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
      pthread_create(&video_play_deal_thread, &attr, (void *)video_play_deal_thread_func,NULL);
      if ( video_play_deal_thread == 0 ) {
            printf("�޷�������Ƶ�������ݴ����߳�\n");
            return;
        }
      pthread_create(&video_play_thread, &attr, (void *)video_play_thread_func,NULL);
      if ( video_play_thread == 0 ) {
            printf("�޷�������Ƶ�����߳�\n");
            return;
        }
    pthread_attr_destroy(&attr);

    VideoPlayIsStart = 1;    
   }
  else
   {
    #ifdef _DEBUG
      printf("�ظ� VideoPlayStart\n");
    #endif
   }
}
//---------------------------------------------------------------------------
void StopPlayVideo(void)
{
  int i;
  int delaytime;
  delaytime=40;
  if(VideoPlayIsStart == 1)
   {
    VideoPlayIsStart = 0;
    video_play_flag = 0;  //��Ƶ���ű�־
    usleep(delaytime*1000);
      if(pthread_cancel(video_play_deal_thread) ==0)
        printf("video_play_deal_thread cancel success\n");
      else
        printf("video_play_deal_thread cancel fail\n");

    usleep(delaytime*1000);
      if(pthread_cancel(video_play_thread) ==0)
        printf("video_play_thread cancel success\n");
      else
        printf("video_play_thread cancel fail\n");

    usleep(delaytime*1000);
    sem_destroy(&videoplaysem);
    sem_destroy(&videorec2playsem);
    pthread_mutex_destroy(&sync_s.video_play_lock);
                                     
    xvid_decoder_end(&dec_video_setting);

    temp_video_n = 0;
    //��Ƶ���ջ�������
    delete_all_videonode(TempVideoNode_h);

    //Ϊ720*480
    if(Local.PlayPicSize == 2)
     {
      Local.PlayPicSize = 1;
     }    
    //����FBҳ��
    if(Local.CurrFbPage != 0)
     {
      Local.CurrFbPage = 0;
      TalkOsd();  //ͨ���ͼ���ʱOSD��ʾ      
      fb_setpage(fbdev, Local.CurrFbPage);
     }

   }
  else
   {
    #ifdef _DEBUG
      printf("�ظ� VideoPlayStop\n");
    #endif
   }
}
//---------------------------------------------------------------------------
//MPEG4�����߳�
void video_play_deal_thread_func(void)
{
  int dwSize;
  int i, j, k;//, sub;
  int tmp;
  int rect1width,rect1height;
  int got_picture;
  int ytop,xleft;
  TempVideoNode1 * tmp_videonode;
  uint32_t dellostframeno;
  uint32_t dellosttimestamp;
  ytop = 100;
  xleft = 200;
  got_picture=0;
//  char audio_out[3072];
  #ifdef _DEBUG
    printf("������Ƶ�������ݴ����̣߳�\n" );
  #endif

  while(video_play_flag == 1)
   {
    //�ȴ��ɼ��߳������ݵ��ź�, ������
    //�ȴ�UDP�����߳������ݵ��ź�
    sem_wait(&videorec2playsem);
     dellostframeno = 0;
     dellosttimestamp = 0;    

   //   while(temp_video_n > 0)    //�п���ͬ������
      if(temp_video_n > 0)
       {
        if(video_play_flag == 0)
          break;
        //������Ӱ
        if(Local.Status == 11)
            usleep(30*1000);  //40ms
        //����videoplaybuf[vpbuf_iget].buffer
        pthread_mutex_lock(&sync_s.video_play_lock);
        //�������ϵ�֡
        tmp_videonode = search_videonode(TempVideoNode_h);
        if(tmp_videonode == NULL)
          temp_video_n = 0;
        if((tmp_videonode != NULL)&&(tmp_videonode->Content.isFull == 1))
         {
          if(vpbuf_n < VPLAYNUM)
           {
            if(Local.PlayPicSize == 1) //CIF
             {
                  xvid_decode(&dec_video_setting, videoplaybuf[vpbuf_iput].buffer, &got_picture,
                              tmp_videonode->Content.buffer, tmp_videonode->Content.Len);
             }
            else                       //D1
             {
              if((tmp_videonode->Content.frame_flag == 4)||(tmp_videonode->Content.frame_flag == 5))
               xvid_decode(&dec_video_setting, fbmem + f_data.buf_len, &got_picture,
                          tmp_videonode->Content.buffer, tmp_videonode->Content.Len);
             }
            videoplaybuf[vpbuf_iput].isUse = 1;
            videoplaybuf[vpbuf_iput].frameno = tmp_videonode->Content.frameno;
            videoplaybuf[vpbuf_iput].timestamp = tmp_videonode->Content.timestamp;
            videoplaybuf[vpbuf_iput].frame_flag = tmp_videonode->Content.frame_flag;

            vpbuf_iput ++;
            if((vpbuf_iput) >= VPLAYNUM)
              vpbuf_iput = 0;
            vpbuf_n ++;

            if(temp_video_n > 0)
              temp_video_n --;
            dellostframeno = tmp_videonode->Content.frameno;
            dellosttimestamp = tmp_videonode->Content.timestamp;

            delete_videonode(tmp_videonode);
           }
          else
           {
            printf("vpbuf_n is full, pause decode one frame,temp_video_n = %d,vpbuf_n = %d\n",
                   temp_video_n, vpbuf_n);
           }
          if(vpbuf_n > VPLAYNUM)
           {
            vpbuf_n = VPLAYNUM;
            printf("vpbuf_n is full\n");
           }

          //ɾ����ȫ֡
          delete_lost_videonode(TempVideoNode_h, dellostframeno, dellosttimestamp);
          sem_post(&videoplaysem);
         }
       //����
       pthread_mutex_unlock(&sync_s.video_play_lock);
      }

  }
}
//---------------------------------------------------------------------------
int oldframeno=0;
void video_play_thread_func(void)
{
  struct timeval tv;
  uint32_t prevtime,nowtime;
  int SkipFrame;
  unsigned int oldvideoplaytime; //��һ֡��Ƶ����ʱ��
  unsigned int videoplaytime; //��Ƶ����ʱ��

  int dwSize;
  int i, j;
  int delaytime;
  int tmp_iget;
  int rect1width,rect1height;
  int got_picture;
  int yTop,xLeft;
  int jump_buf;  //�ѽ��뻺������֡��
  int jump_tmp;  //���ջ�������֡��
  int jump_frame;
  TempVideoNode1 * tmp_videonode;
  int timeperframe;
  unsigned char *rgb_data;
  int IFrameCount;

  int D1_Start, D1_End, D1_deltaX;
  int frametrans = 0;

  oldvideoplaytime = 0;
  videoplaytime = 0;
  got_picture=0;
  delaytime = 0;
  SkipFrame = 0;
  IFrameCount = 0;
  if(Local.PlayPicSize == 2) //D1
   {
    xLeft = 40;
    yTop = 0;
    D1_Start = 32;
    D1_End = D1_H - 32;
    D1_deltaX = 32;
   }
  else
   {
    xLeft = 129;
    yTop  = 122;
   }
  #ifdef _DEBUG
    printf("������Ƶ�����̣߳�\n" );
    printf("video_play_flag=%d\n",video_play_flag);
  #endif  
  while(video_play_flag == 1)
   {
     //���Ż�����Ϊ��,����
     sem_wait(&videoplaysem);
     if(video_play_flag == 0)
       break;
     if(vpbuf_n > 0)
      {
       //����videoplaybuf[vpbuf_iget].buffer
       pthread_mutex_lock(&sync_s.video_play_lock);
       //��ǰ�Ѳ���֡ʱ���
       curr_video_timestamp = videoplaybuf[vpbuf_iget].timestamp;

       rect1width = dec_video_setting.framewidth;
       rect1height = dec_video_setting.frameheight;

    //   printf("Local.PlayPicSize = %d, videoplaybuf[vpbuf_iget].frame_flag = %d\n", Local.PlayPicSize, videoplaybuf[vpbuf_iget].frame_flag);
       if(((Local.PlayPicSize == 2) && ((videoplaybuf[vpbuf_iget].frame_flag == 4)||
         (videoplaybuf[vpbuf_iget].frame_flag == 5))) ||
         ((Local.PlayPicSize == 1) && ((videoplaybuf[vpbuf_iget].frame_flag == 2)||
         (videoplaybuf[vpbuf_iget].frame_flag == 3))))
         {          
            if(Local.PlayPicSize == 1)
             if(videoplaybuf[vpbuf_iget].buffer != NULL)
              {
               for(j=0;j<rect1height;j++)
                 memcpy(fbmem+(j+yTop)*fb_width+xLeft,
                       (int)(videoplaybuf[vpbuf_iget].buffer+j*rect1width), rect1width);
               for(j=0;j<rect1height/2;j++)
                 memcpy(fbmem + f_data.uv_offset + (j+yTop/2)*fb_width/2 +xLeft/2,
                       (int)(videoplaybuf[vpbuf_iget].buffer+rect1width*rect1height+j*rect1width/2), rect1width/2);
               for(j=0;j<rect1height/2;j++)
                 memcpy(fbmem + f_data.uv_offset+f_data.uv_offset/4 + (j+yTop/2)*fb_width/2 +xLeft/2,
                       (int)(videoplaybuf[vpbuf_iget].buffer+rect1width*rect1height*5/4+j*rect1width/2), rect1width/2);

               if((Local.RecordPic == 1)||(Local.RecordPic == 2))
                {
                 //�洢��3��I֡
                 if(videoplaybuf[vpbuf_iget].frame_flag == 2)
                   Local.IFrameCount ++;
                 if(Local.IFrameCount > Local.IFrameNo)
                  {
                   memcpy(Local.yuv[0], videoplaybuf[vpbuf_iget].buffer,
                          rect1width*rect1height*3/2);
                   Local.IFrameCount = 0;
                   Local.RecordPic = 0;
                   Local.HavePicRecorded = 1;  //����Ƭ��¼��
                   Local.HavePicRecorded_flag = 1; 
                   Local.recpic_tm_t = curr_tm_t;
                  }
                }         
             }
        }
       //�鿴���޶�����������
       if((videoplaybuf[vpbuf_iget].frameno - oldframeno) != 1)
             {
              printf("frameno lost %d ::: %d\n", videoplaybuf[vpbuf_iget].frameno, oldframeno);
             }
       oldframeno = videoplaybuf[vpbuf_iget].frameno;

       videoplaybuf[vpbuf_iget].isUse = 0;
       videoplaybuf[vpbuf_iget].frameno = 0;
       videoplaybuf[vpbuf_iget].timestamp = 0;

       vpbuf_iget ++;
       if((vpbuf_iget) >= VPLAYNUM)
         vpbuf_iget = 0;
       if(vpbuf_n > 0)
         vpbuf_n --;
       else
         printf("vpbuf_n is error\n");

     if((TimeStamp.OldCurrVideo != TimeStamp.CurrVideo) //��һ�ε�ǰ��Ƶʱ��
        &&(TimeStamp.OldCurrVideo != 0)&&(TimeStamp.CurrVideo != 0))
      {
                      //����3֡ 120ms �ӳٳ���160ms����֡
        timeperframe = 40;          //����3֡ 120ms �ӳٳ���160ms����֡
        if((TimeStamp.CurrVideo - curr_video_timestamp) > 280)
         {
          jump_frame = (TimeStamp.CurrVideo - curr_video_timestamp)/timeperframe - 3;
          if(vpbuf_n >= jump_frame)
           {
            jump_buf = jump_frame;
            jump_tmp = 0;
           }
          else
           {
             temp_video_n = length_videonode(TempVideoNode_h);
             jump_buf = vpbuf_n;
             if(temp_video_n > (jump_frame - vpbuf_n))
               jump_tmp = jump_frame - vpbuf_n;
             else
               jump_tmp = temp_video_n;
           }

          printf("video jump_buf =%d , jump_tmp = %d, jump_frame = %d\n", jump_buf, jump_tmp, jump_frame);

          for(i=0; i<jump_buf; i++)
           {
              vpbuf_iget ++;
              if((vpbuf_iget) >= VPLAYNUM)
                vpbuf_iget = 0;
              if(vpbuf_n > 0)
                vpbuf_n = vpbuf_n - 1;
            }
           for(i=0; i<jump_tmp; i++)
            {
              //�������ϵ�֡
              tmp_videonode = search_videonode(TempVideoNode_h);
              if((tmp_videonode != NULL)&&(temp_video_n > 0))
               if((tmp_videonode->Content.frame_flag !=2)&&(tmp_videonode->Content.frame_flag !=4))
                {
                 delete_videonode(tmp_videonode);
                 temp_video_n --;
                }
            }
         }
      }          

       //����
       pthread_mutex_unlock(&sync_s.video_play_lock);

      }
   }
}
//---------------------------------------------------------------------------
TempVideoNode1 * init_videonode(void) //��ʼ��������ĺ���
{
  TempVideoNode1 *h; // *h�����ͷ����ָ�룬*pָ��ǰ����ǰһ����㣬*sָ��ǰ���
  if((h=(TempVideoNode1 *)malloc(sizeof(TempVideoNode1)))==NULL) //����ռ䲢���
  {
    printf("���ܷ����ڴ�ռ�!");
    return NULL;
  }
  h->llink=NULL; //������
  h->rlink=NULL; //������
  return(h);
}
//---------------------------------------------------------------------------
//�������ƣ�creat
//����������������β���������
//�������ͣ��޷���ֵ
//���������� h:������ͷָ��
int creat_videonode(TempVideoNode1 *h, struct talkdata1 talkdata,
        unsigned char *r_buf , int r_length)
{
    TempVideoNode1 *t;
    TempVideoNode1 *p;
    int j;
    int DataOk;
    t=h;
  //  t=h->next;
    while(t->rlink!=NULL)    //ѭ����ֱ��tָ���
      t=t->rlink;   //tָ����һ���
    DataOk = 1;
    if((talkdata.DataType < 1) || (talkdata.DataType > 5))
      DataOk = 0;
    if(talkdata.Framelen > VIDEOMAX)
      DataOk = 0;
    if(talkdata.CurrPackage > talkdata.TotalPackage)
      DataOk = 0;
    if(t&&(DataOk == 1))
    {
      //β�巨��������
       if((p=(TempVideoNode1 *)malloc(sizeof(TempVideoNode1)))==NULL) //�����½��s���������ڴ�ռ�
       {
        printf("���ܷ����ڴ�ռ�!\n");
        return 0;
       }
       if((p->Content.buffer=(unsigned char *)malloc(talkdata.Framelen))==NULL)
        {
         printf("���ܷ�����Ƶ�����ڴ�ռ�!\n");
         return 0;
        }
      p->Content.isFull = 0;
      for(j=0; j<MAXPACKNUM; j++)
        p->Content.CurrPackage[j] = 0;
               
      p->Content.frame_flag = talkdata.DataType;
      p->Content.frameno = talkdata.Frameno;
      p->Content.TotalPackage = talkdata.TotalPackage;
      p->Content.timestamp = talkdata.timestamp;
      p->Content.CurrPackage[talkdata.CurrPackage - 1] = 1;
      if(talkdata.CurrPackage == p->Content.TotalPackage)
        p->Content.Len =  (talkdata.CurrPackage - 1) * talkdata.PackLen + r_length - DeltaLen;
      p->Content.Len = talkdata.Framelen;
      memcpy(p->Content.buffer + (talkdata.CurrPackage - 1) * talkdata.PackLen,
             r_buf + DeltaLen, r_length - DeltaLen);
      p->Content.isFull = 1;
      for(j=0; j< p->Content.TotalPackage; j++)
       if(p->Content.CurrPackage[j] == 0)
        {
         p->Content.isFull = 0;
         break;
        }
      p->rlink=NULL;    //p��ָ����Ϊ��
      p->llink=t;
      t->rlink=p;       //p��nextָ��������
  //    t=p;             //tָ��������
      return p->Content.isFull;
    }
}
//---------------------------------------------------------------------------
//�������ƣ�add
//����������������β���������
//�������ͣ��޷���ֵ
//���������� h:������ͷָ��
int add_videonode(TempVideoNode1 *p, struct talkdata1 talkdata,
      unsigned char *r_buf , int r_length)
{
    int j;
    int DataOk;

    DataOk = 1;
    if((talkdata.DataType < 1) || (talkdata.DataType > 5))
      DataOk = 0;
    if(talkdata.Framelen > VIDEOMAX)
      DataOk = 0;
    if(talkdata.CurrPackage > talkdata.TotalPackage)
      DataOk = 0;
    if(talkdata.CurrPackage <= 0)
      DataOk = 0;
    if(talkdata.TotalPackage <= 0)
      DataOk = 0;
    if(DataOk == 1)
     {
      p->Content.frame_flag = talkdata.DataType;
      p->Content.frameno = talkdata.Frameno;
      p->Content.TotalPackage = talkdata.TotalPackage;
      p->Content.timestamp = talkdata.timestamp;
      p->Content.CurrPackage[talkdata.CurrPackage - 1] = 1;
      if(talkdata.CurrPackage == p->Content.TotalPackage)
        p->Content.Len =  (talkdata.CurrPackage - 1) * talkdata.PackLen + r_length - DeltaLen;
      memcpy(p->Content.buffer + (talkdata.CurrPackage - 1) * talkdata.PackLen,
             r_buf + DeltaLen, r_length - DeltaLen);
      p->Content.isFull = 1;
      for(j=0; j< p->Content.TotalPackage; j++)
       if(p->Content.CurrPackage[j] == 0)
        {
         p->Content.isFull = 0;
         break;
        }

      return p->Content.isFull;
     }
}
//---------------------------------------------------------------------------
//�������ƣ�creat
//����������������β���������
//�������ͣ��޷���ֵ
//���������� h:������ͷָ��
int creat_leavemovienode(TempVideoNode1 *h, uint32_t rframeno, uint32_t rtimestamp,
        unsigned short rframe_flag, unsigned char *r_buf , int r_length)
{
    TempVideoNode1 *t;
    TempVideoNode1 *p;
    int j;
    uint32_t newframeno;
    int currpackage;
    t=h;
  //  t=h->next;
    while(t->rlink!=NULL)    //ѭ����ֱ��tָ���
      t=t->rlink;   //tָ����һ���
    if(t)
    {
      //β�巨��������
       if((p=(TempVideoNode1 *)malloc(sizeof(TempVideoNode1)))==NULL) //�����½��s���������ڴ�ռ�
       {
        printf("���ܷ����ڴ�ռ�!\n");
        return 0;
       }
       if((p->Content.buffer=(unsigned char *)malloc(r_length))==NULL)
        {
         printf("���ܷ�����Ƶ�����ڴ�ռ�!\n");
         return 0;
        }       
      p->Content.frame_flag = rframe_flag;
      p->Content.frameno = rframeno;
      p->Content.timestamp = rtimestamp;

      p->Content.Len =  r_length;
      memcpy(p->Content.buffer,
             r_buf, r_length);
      p->Content.isFull = 1;
      p->rlink=NULL;    //p��ָ����Ϊ��
      p->llink=t;
      t->rlink=p;       //p��nextָ��������
      t=p;             //tָ��������
      return p->Content.isFull;
    }
}
//---------------------------------------------------------------------------
//�������ƣ�length
//������������������
//�������ͣ��޷���ֵ
//����������h:������ͷָ��
int length_videonode(TempVideoNode1 *h)
{
    TempVideoNode1 *p;
    int i=0;         //��¼������
    p=h->rlink;
    while(p!=NULL)    //ѭ����ֱ��pָ���
    {
        i=i+1;
        p=p->rlink;   //pָ����һ���
     }
    return i;
 //    printf(" %d",i); //���p��ָ�ӵ��������
}
//---------------------------------------------------------------------------
//�������ƣ�delete_
//����������ɾ������
//�������ͣ�����
//����������h:������ͷָ�� i:Ҫɾ����λ��
int delete_videonode(TempVideoNode1 *p)
{
  if(p != NULL)
   {
    //��Ϊ���һ�����
    if(p->rlink != NULL)
     {
      (p->rlink)->llink=p->llink;
      (p->llink)->rlink=p->rlink;
      if(p->Content.buffer)
        free(p->Content.buffer);
      if(p)
        free(p);
     }
    else
     {
      (p->llink)->rlink=p->rlink;
      if(p->Content.buffer)
        free(p->Content.buffer);
      if(p)
        free(p);
     }
    return(1);
   }
  else
    printf("video delete null\n");   
  return(0);
}
//---------------------------------------------------------------------------
int delete_all_videonode(TempVideoNode1 *h)
{
  TempVideoNode1 *p,*q;
  p=h->rlink;        //��ʱpΪ�׽��
  while(p != NULL)   //�ҵ�Ҫɾ������λ��
   {
      //��Ϊ���һ�����
      q = p;
      if(p->rlink != NULL)
       {
        (p->rlink)->llink=p->llink;
        (p->llink)->rlink=p->rlink;
       }
      else
        (p->llink)->rlink=p->rlink;
      p = p->rlink;
      if(q->Content.buffer)
        free(q->Content.buffer);
      if(q)
        free(q);
   }
}
//---------------------------------------------------------------------------
int delete_lost_videonode(TempVideoNode1 *h, uint32_t currframeno, uint32_t currtimestamp) //ɾ����ȫ֡
{
  TempVideoNode1 *p,*q;
  p=h->rlink;        //��ʱpΪ�׽��
  while(p != NULL)   //�ҵ�Ҫɾ������λ��
   {
      //��Ϊ���һ�����
      q = p;
      if(p->rlink != NULL)
       {
//        if(p->Content.frameno < currframeno) //����ѭ����ֱ��pΪ�գ����ҵ�x
         if(p->Content.timestamp < currtimestamp)
          {
           (p->rlink)->llink=p->llink;
           (p->llink)->rlink=p->rlink;
           p = p->llink;
           if(q->Content.buffer)
             free(q->Content.buffer);
           if(q)
             free(q);
           if(temp_video_n > 0)
             temp_video_n --;             
          }
       }
      else
       {
//        if(p->Content.frameno < currframeno) //����ѭ����ֱ��pΪ�գ����ҵ�x
         if(p->Content.timestamp < currtimestamp)
          {
           (p->llink)->rlink=p->rlink;
           p = p->llink;
           if(q->Content.buffer)
             free(q->Content.buffer);
           if(q)
             free(q);
           if(temp_video_n > 0)
             temp_video_n --;
          }
       }   
      p = p->rlink;
   }
  return 1;
}
//---------------------------------------------------------------------------
//�������ƣ�find_
//�������������Һ���
//�������ͣ�����
//����������h:������ͷָ�� x:Ҫ���ҵ�ֵ
//���Ҹ�֡�ð��Ƿ��Ѵ���
TempVideoNode1 * find_videonode(TempVideoNode1 *h, int currframeno, int currpackage)
{
  TempVideoNode1 *p;
  int PackIsExist; //���ݰ��ѽ��ձ�־
  int FrameIsNew;  //���ݰ��Ƿ�����֡�Ŀ�ʼ
  p=h->rlink;    //��ʱpΪ�׽��
  PackIsExist = 0;
  FrameIsNew = 1;
  while(p!=NULL)
   {
    if(p->Content.frameno == currframeno) //����ѭ����ֱ��pΪ�գ����ҵ�x
     {
      FrameIsNew = 0;
      if(p->Content.CurrPackage[currpackage - 1] == 1)
       {
        #ifdef _DEBUG
          printf("pack exist %d\n", currframeno);
        #endif
        PackIsExist = 1;
       }
      break;
     }
    p=p->rlink;   //sָ��p����һ���
   }
  if(p!=NULL)
    return p;
  else
    return NULL;
}
//---------------------------------------------------------------------------
//�������ƣ�find_
//�������������Һ���
//�������ͣ�����
//����������h:������ͷָ�� x:Ҫ���ҵ�ֵ
//�������ϵ�֡
TempVideoNode1 * search_videonode(TempVideoNode1 *h)
{
  TempVideoNode1 *p;
  TempVideoNode1 *tem_p;

  tem_p = NULL;  
  p=h->rlink;    //��ʱpΪ�׽��
  //��ȡ��tem_p, ����ֱ��ȡ��һ������Ϊ����һ��Ϊ��ȫ֡�������ѭ��
  while(p!=NULL)
   {
    if(p->Content.isFull == 1)
     {
      tem_p = p;
      break;
     }
    p=p->rlink;   //sָ��p����һ���
   }

  if(tem_p != NULL)
   while(p!=NULL)
    {
     if(p->Content.isFull == 1)
//     if(p->Content.frameno < tem_p->Content.frameno) //����ѭ����ֱ��pΪ�գ����ҵ�x
      if(p->Content.timestamp < tem_p->Content.timestamp) //����ѭ����ֱ��pΪ�գ����ҵ�x
       {
        tem_p = p;
       }
     p=p->rlink;   //sָ��p����һ���
    }

  return tem_p;

}
//---------------------------------------------------------------------------
int free_videonode(TempVideoNode1 *h)
{
  TempVideoNode1 *p,*t;
  int i=0;         //��¼������
  p=h->rlink;
  while(p!=NULL)    //ѭ����ֱ��pָ���
   {
    i=i+1;
    t = p;
    p=p->rlink;   //pָ����һ���
    free(t);
   }
  return i;
}

int v4l_init(video_profile *profile,int seq, int ingrab)
{
    int type;
    int uframerate;
    video_fd = open("/dev/video0", O_RDWR);

    if(video_fd < 0)    {
        printf("open device failure");
        return -1;
    }
    if (!ioctl(video_fd, VIDIOCGTUNER, &tuner))    {
        tuner.mode = VIDEO_MODE_NTSC;
        ioctl(video_fd, VIDIOCSTUNER, &tuner);
    }
    #ifdef _DEBUG
      printf("gb_buffers.size1=%d\n",gb_buffers.size);
    #endif  
    if ( ioctl(video_fd, VIDIOCGMBUF, &gb_buffers) < 0 )	{
		printf("capture not support mbuf\n");
		return -1;
	}
    #ifdef _DEBUG
      printf("gb_buffers.size2=%d\n",gb_buffers.size);
    #endif  

    if(mmap_done==0)
	{
		printf("+++++++++++++++++++map++++++++++++++++++++\n");
        		mmap_done = 1;
        		video_buf_virt = mmap(0, gb_buffers.size, PROT_READ|PROT_WRITE, MAP_SHARED, video_fd, 0);
	}

    ioctl(video_fd, 0x5679, &ingrab);            //grab type
    type = seq;
    ioctl(video_fd, 0x5678, &type); 

    uframerate =  profile->framerate;
    uframerate = 15;
    ioctl(video_fd, 0x5680, &uframerate); 

    gb_buf.height=profile->height;
    gb_buf.width=profile->width;
    gb_buf.format=VIDEO_PALETTE_YUV420P;
    gb_buf.frame=0;
    return 0;
}
//---------------------------------------------------------------------------
int v4l_close(void)
{
  munmap(video_buf_virt, gb_buffers.size);
  close(video_fd);
  mmap_done=0;
  video_fd = -1;
}
//---------------------------------------------------------------------------
int capture_video(video_profile *video)
{
  struct timeval tv;
  uint32_t nowtime,prevtime;

	uint8_t             *ptr;    

    /* Setup to capture the next frame */
    if ((errno=ioctl(video_fd, VIDIOCMCAPTURE, &gb_buf)) < 0) 
    {
        return;
    }

    while (ioctl(video_fd, VIDIOCSYNC, &frame_num) < 0 &&(errno == EAGAIN || errno == EINTR))
        printf("error\n");
    
    ptr = video_buf_virt + gb_buffers.offsets[frame_num];
    frame_num = gb_buf.frame;

    
    videorecbuf.buffer_y[videorecbuf.iput] = (unsigned char *)(ptr);      //?


    if(video->height % 16 != 0)
        video->height = 16 * ((video->height / 16) + 1 );
    
    if(video->width % 16 != 0)
        video->width = 16 * ((video->width / 16) + 1 );

    // UV    	
    videorecbuf.buffer_u[videorecbuf.iput] = (unsigned char *) (ptr + (video->width * video->height));
    videorecbuf.buffer_v[videorecbuf.iput] = (unsigned char *) (ptr + (video->width * video->height * 5 / 4));

    return 0;
}
//---------------------------------------------------------------------------
int enc_init(video_profile *video_setting)
{
    FMP4_ENC_PARAM  enc_param;

    if(fmpeg4_enc_fd == 0)
	    fmpeg4_enc_fd = open( FMPEG4_ENCODER_DEV,O_RDWR );

    if(fmpeg4_enc_fd == 0)
    {
	printf("Fail to open %s\n",FMPEG4_ENCODER_DEV);
	return -1;
    }

        memset(&enc_param, 0, sizeof(FMP4_ENC_PARAM));
        enc_param.u32API_version = 0x00020002;//0x00020002;//MP4VER;
	enc_param.u32BitRate=video_setting->bit_rate;
	enc_param.u32FrameWidth=video_setting->width;
	enc_param.u32FrameHeight=video_setting->height;
	enc_param.fFrameRate=(float)video_setting->framerate/(float)video_setting->frame_rate_base;
         enc_param.u32InitialQuant=video_setting->quant;
	enc_param.u32IPInterval=video_setting->gop_size;
        #ifdef _DEBUG
          printf("enc_param.u32IPInterval=%d\n" ,enc_param.u32IPInterval);
        #endif  
	enc_param.bEnable4MV=0;
         enc_param.bH263Quant=1;

#if 0 //def H263
	enc_param.bShortHeader=1;
#else
	enc_param.bShortHeader=0;
#endif
	enc_param.bResyncMarker=0;
	enc_param.u32MaxQuant=video_setting->qmax;
	enc_param.u32MinQuant=video_setting->qmin;

         enc_param.bROIEnable   = video_setting->roi_enable;     //0:disable 1:enable
	enc_param.u32ROIX      = video_setting->roi_x;     // must be a multiple of 16 pixels
         enc_param.u32ROIY      = video_setting->roi_y;     // must be a multiple of 16 pixels
         enc_param.u32ROIWidth  = video_setting->roi_width;   // must be a multiple of 16 pixels
         enc_param.u32ROIHeight = video_setting->roi_height;  // must be a multiple of 16 pixels
         //enc_param.motion_dection_enable=0;


    RateControlInit(&ratec,enc_param.u32BitRate*1000, DELAY_FACTOR, RC_AVERAGE_PERIOD,
                    ARG_RC_BUFFER, get_framerate(enc_param.fFrameRate),
                    enc_param.u32MaxQuant,enc_param.u32MinQuant,enc_param.u32InitialQuant);

    if (  ioctl(fmpeg4_enc_fd,FMPEG4_IOCTL_ENCODE_INIT,&enc_param) < 0)
    {
    	close(fmpeg4_enc_fd);
	    printf("Error to set FMPEG4_IOCTL_INIT\n");
	    return -1;
    }
    
	return 0;
}


//---------------------------------------------------------------------------
int enc_close(video_profile *video_setting)
{
	if(fmpeg4_enc_fd)
		close(fmpeg4_enc_fd);
	fmpeg4_enc_fd = 0;
 //	free(video_setting->coded_frame);
	return 0;
}
//---------------------------------------------------------------------------
int mpeg4_encode(video_profile *video_setting, unsigned char *frame, unsigned char * data_y,
                 unsigned char * data_u, unsigned char * data_v)
{
//    AVFrame             *pict=(AVFrame *)data;
    GM_ENC_FRAME   enc_frame;

    if ( video_setting->roi_enable) {
	enc_frame.roi_X = video_setting->roi_x;
	enc_frame.roi_Y = video_setting->roi_y;
    }
    enc_frame.pu8YFrameBaseAddr = data_y;//(unsigned char *)pict->data[0];   //input user continued virtual address (Y), Y=0 when NVOP
    enc_frame.pu8UFrameBaseAddr = data_u;//(unsigned char *)pict->data[1];   //input user continued virtual address (U)
    enc_frame.pu8VFrameBaseAddr = data_v;//(unsigned char *)pict->data[2];   //input user continued virtual address (V)
    enc_frame.bitstream = frame;  //output User Virtual address
   	enc_frame.quant = ratec.rtn_quant;

    enc_frame.intra = video_setting->intra;
    if (  ioctl(fmpeg4_enc_fd, FMPEG4_IOCTL_ENCODE_FRAME, &enc_frame) <0)
    {
	    printf("Error to set FMPEG4_IOCTL_ENCODE_FRAME\n");
	    return -1;
    }

    RateControlUpdate(&ratec, enc_frame.quant, enc_frame.length, enc_frame.intra);

    video_setting->intra = enc_frame.intra; //the return value in enc_frame.intra

    return enc_frame.length;
}
//---------------------------------------------------------------------------
int mpeg4_encode_init(int enc_width, int enc_height, int framerate)
{
    int ret;
    int i;
    int blength;

    enc_video_setting.roi_enable = 0;
    //set the default value
    enc_video_setting.qmax = 31;
    enc_video_setting.qmin = 1;
    enc_video_setting.quant = 4;
    if(Local.RecPicSize == 2)       //D1
      enc_video_setting.bit_rate = 1024;
    else
     {
      enc_video_setting.bit_rate = 512;
     }
    printf("enc_video_setting.bit_rate = %d\n", enc_video_setting.bit_rate);
    enc_video_setting.width = enc_width;
    enc_video_setting.height = enc_height;
    enc_video_setting.framerate = framerate;
    enc_video_setting.frame_rate_base = 1;
    enc_video_setting.gop_size = 50;

    enc_video_setting.roi_x = MP4_ROIX;
    enc_video_setting.roi_y = MP4_ROIY;
    enc_video_setting.roi_width = 320;
    enc_video_setting.roi_height = 240;

    if (enc_video_setting.roi_enable )
       printf("roi width %d height %d\n", enc_video_setting.roi_width, enc_video_setting.roi_height);
    else
       printf("frame width %d height %d\n", enc_video_setting.width, enc_video_setting.height);

   out_virt_buffer = (unsigned char *) malloc(enc_video_setting.width*enc_video_setting.height*3/2);
   //1 for mpeg4  ԭ���������
    //JPEG need sequential format
    // 0 for Mpeg4 2D YUV
    // 1 for JPG Sequencoal YUV
    // 2 for H.264 2D
//    if((ret=v4l_init(&enc_video_setting,1)) < 0)
    if((ret=v4l_init(&enc_video_setting, 0, 8)) < 0)
        return -1;

    if(enc_init(&enc_video_setting)<0)
        return -1;

    return 0;
}
//---------------------------------------------------------------------------

void StartRecVideo(int width, int height)
{
  pthread_attr_t attr;
  int i, j;
  int framerate;
  if(VideoRecIsStart == 0)
  {
    VideoRecIsStart = 1;
    if((width == D1_W)&&(height == D1_H))
     {
      Local.RecPicSize = 2;      //720*240
      framerate = 5;
     }
    else
     {
      Local.RecPicSize = 1;      //352*288
      framerate = 30;
     }
    usleep(200*1000); 

    if(VideoRecIsStart == 0)
      return;
    //��Ƶ�ɼ�������
    framerate = 25;
    if(mpeg4_encode_init(width, height, framerate) !=0 )
     {
      printf("mpeg4_encode_init error!\\n");
      return;
     }

    Local.nowvideoframeno = 1; 
    //��Ƶ�ɼ�����
    videorecbuf.iput = 0;
    videorecbuf.iget = 0;
    videorecbuf.n = 0;
    for(i=0; i<VNUM; i++)
     {
      videorecbuf.frameno[i] = 0;
      videorecbuf.timestamp[i] = 0;
     }

    sem_init(&videorecsem,0,0);
    pthread_mutex_init (&sync_s.video_rec_lock, NULL);

    video_rec_flag = 1;   //��Ƶ�ɼ���־
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&video_rec_deal_thread,&attr,(void *)video_rec_deal_thread_func,NULL);
    if ( video_rec_deal_thread == 0 ) {
          printf("�޷�������Ƶ�ɼ����ݴ����߳�\n");
          pthread_attr_destroy(&attr);
          return;
      }
    pthread_create(&video_rec_thread,&attr,(void *)video_rec_thread_func,NULL);
    pthread_attr_destroy(&attr);
    if ( video_rec_thread == 0 ) {
          printf("�޷�������Ƶ�ɼ��߳�\n");
          return;
      }
   }
  else
   {
    #ifdef _DEBUG
      printf("�ظ� VideoRecStart\n");
    #endif
   }
}
//---------------------------------------------------------------------------
void StopRecVideo(void)
{
  int i;
  int delaytime;
  delaytime=40;
  if(VideoRecIsStart == 1)
   {
    VideoRecIsStart = 0;
    video_rec_flag = 0;   //��Ƶ�ɼ���־
    usleep(delaytime*1000);
    if(pthread_cancel(video_rec_thread) ==0)
      printf("video_rec_thread cancel success\n");
    else
      printf("video_rec_thread cancel fail\n");
    usleep(delaytime*1000);
    if(pthread_cancel(video_rec_deal_thread) ==0)
      printf("video_rec_deal_thread cancel success\n");
    else
      printf("video_rec_deal_thread cancel fail\n");
    usleep(delaytime*1000);
    sem_destroy(&videorecsem);
    pthread_mutex_destroy(&sync_s.video_rec_lock);

    //�ͷ�ͬ�������̼߳�����
    //sync_play_destroy();
    //����
    	v4l_close();
    	enc_close(&enc_video_setting);
    	free(out_virt_buffer);
   }
  else
   {
    #ifdef _DEBUG
      printf("�ظ� VideoRecStop\n");
    #endif
   }
}
//---------------------------------------------------------------------------
//MPEG4���뼰�����߳�
void video_rec_deal_thread_func(void)
{
  int dwSize;
  int i,j;
  int TotalPackage; //�ܰ���
  int FrameNum;
  unsigned char mpeg4_out[1600];
//������
  int rect1width,rect1height;
  int got_picture;
  int ytop,xleft;
  char RemoteHost[20];
  //ͨ�����ݽṹ
  struct talkdata1 talkdata;
  ytop = 0;
  xleft = 0;
  got_picture=0;

  #ifdef _DEBUG
    printf("�����ɼ����ݴ����̣߳�\n" );
  #endif
  while(video_rec_flag == 1)
   {
    //�ȴ��ɼ��߳������ݵ��ź�
    sem_wait(&videorecsem);
      //����
    pthread_mutex_lock(&sync_s.video_rec_lock);
    FrameNum = videorecbuf.n;
 //   for(i=0; i<videorecbuf.n; i++)
 //   while(videorecbuf.n > 0)
    if(videorecbuf.n > 0)
     {
      //ͷ��
      memcpy(mpeg4_out, UdpPackageHead, 6);
      //����

      if((Local.Status == 1)||(Local.Status == 2)||(Local.Status == 5)||(Local.Status == 6)
        ||(Local.Status == 7)||(Local.Status == 8)||(Local.Status == 9)||(Local.Status == 10))  //�Խ�
        {
         if(Remote.isDirect == 1)
           mpeg4_out[6] = VIDEOTALKTRANS;
         else
           mpeg4_out[6] = VIDEOTALK;
        }
      if((Local.Status == 3)||(Local.Status == 4))  //����
        {
         if(Remote.isDirect == 1)
           mpeg4_out[6] = VIDEOWATCHTRANS;
         else
           mpeg4_out[6] = VIDEOWATCH;
        }

      mpeg4_out[7] = 1;
      //������
      if((Local.Status == 1)||(Local.Status == 3)||(Local.Status == 5)||(Local.Status == 7)||(Local.Status == 9))  //����Ϊ���з�
       {
        mpeg4_out[8] = CALLUP;
        memcpy(talkdata.HostAddr, LocalCfg.Addr, 20);
        memcpy(talkdata.HostIP, LocalCfg.IP, 4);
        memcpy(talkdata.AssiAddr, Remote.Addr[0], 20);
        if((mpeg4_out[6] == VIDEOTALKTRANS)||(mpeg4_out[6] == VIDEOWATCHTRANS))
          memcpy(talkdata.AssiIP, Remote.DenIP, 4);
        else
          memcpy(talkdata.AssiIP, Remote.IP[0], 4);
       }
      if((Local.Status == 2)||(Local.Status == 4)||(Local.Status == 6)||(Local.Status == 8)||(Local.Status == 10))  //����Ϊ���з�
       {
        mpeg4_out[8] = CALLDOWN;
        memcpy(talkdata.HostAddr, Remote.Addr[0], 20);
        if((mpeg4_out[6] == VIDEOTALKTRANS)||(mpeg4_out[6] == VIDEOWATCHTRANS))
          memcpy(talkdata.HostIP, Remote.DenIP, 4);
        else
          memcpy(talkdata.HostIP, Remote.IP[0], 4);
        memcpy(talkdata.AssiAddr, LocalCfg.Addr, 20);
        memcpy(talkdata.AssiIP, LocalCfg.IP, 4);
       }

      //ʱ���
      talkdata.timestamp = videorecbuf.timestamp[videorecbuf.iget];

      //֡���
      talkdata.Frameno = videorecbuf.frameno[videorecbuf.iget];

       if(Local.ForceIFrame == 1)
        {
         enc_video_setting.intra = 1;
         Local.ForceIFrame = 0;
        }
       else
         enc_video_setting.intra = -1;
       dwSize=mpeg4_encode(&enc_video_setting, out_virt_buffer, videorecbuf.buffer_y[videorecbuf.iget],
                           videorecbuf.buffer_u[videorecbuf.iget], videorecbuf.buffer_v[videorecbuf.iget]);
      //֡���ݳ���
      talkdata.Framelen = dwSize;
       //I֡
       if(enc_video_setting.intra == 1)
        {
         //��������
         if(Local.RecPicSize == 2)
           talkdata.DataType = 4;  //720*480 I֡
         else
           talkdata.DataType = 2;   //352*240 I֡
        }
       else
        {
         //��������
         if(Local.RecPicSize == 2)
           talkdata.DataType = 5;   //720*480 P֡
         else
           talkdata.DataType = 3;   //352*240 P֡
        }
      if((videorecbuf.iget + 1) >= VNUM)
        videorecbuf.iget = 0;
      else
        videorecbuf.iget ++;
      videorecbuf.n --;

       if(Local.Status != 0)
        {
         talkdata.PackLen = PACKDATALEN;
         //�ܰ���
         if((dwSize%talkdata.PackLen)==0)
           TotalPackage=dwSize/talkdata.PackLen;
         else
           TotalPackage=dwSize/talkdata.PackLen+1;
         talkdata.TotalPackage = TotalPackage;
         sprintf(RemoteHost, "%d.%d.%d.%d\0",Remote.DenIP[0],
                 Remote.DenIP[1],Remote.DenIP[2],Remote.DenIP[3]);

         for(j=1; j<=TotalPackage; j++)         //����˳��Ӵ�С
          {
           if(j==TotalPackage)
            {
             talkdata.CurrPackage = j;      //��ǰ��
             talkdata.Datalen = dwSize-(j-1)*talkdata.PackLen;     //���ݳ���
             memcpy(mpeg4_out + 9, &talkdata, sizeof(talkdata));
             memcpy(mpeg4_out +DeltaLen, out_virt_buffer + (j-1)*talkdata.PackLen, (dwSize-(j-1)*talkdata.PackLen));
             //UDP����
             UdpSendBuff(m_VideoSocket, RemoteHost, mpeg4_out , DeltaLen + (dwSize-(j-1)*talkdata.PackLen));
            }
           else
            {
             talkdata.CurrPackage = j;           //��ǰ��
             talkdata.Datalen = talkdata.PackLen;       //���ݳ���
             memcpy(mpeg4_out + 9, &talkdata, sizeof(talkdata));
             memcpy(mpeg4_out +DeltaLen, out_virt_buffer + (j-1)*talkdata.PackLen, talkdata.PackLen);
             //UDP����
             UdpSendBuff(m_VideoSocket, RemoteHost, mpeg4_out , DeltaLen + talkdata.PackLen);
            }
          }
        }
     }
  //����
  pthread_mutex_unlock(&sync_s.video_rec_lock);
  //  video_rec_flag = 0;
    //������Ϣ���������ݴ����߳�,������
 //   sem_post(&videorec2playsem);
   }
}
//---------------------------------------------------------------------------
void video_rec_thread_func(void)
{
  struct timeval tv;
  uint32_t nowtime,prevtime;
  int dwSize;
  #ifdef _DEBUG
    printf("������Ƶ�ɼ��̣߳�\n" );
    printf("video_rec_flag=%d\n",video_rec_flag);
  #endif
  gettimeofday(&tv, NULL);
  nowtime = tv.tv_sec *1000 + tv.tv_usec/1000;
  prevtime = nowtime;
  while(video_rec_flag == 1)
   {
    //����
    pthread_mutex_lock(&sync_s.video_rec_lock);

    videorecbuf.frameno[videorecbuf.iput] = Local.nowvideoframeno;
    Local.nowvideoframeno++;
    if(Local.nowvideoframeno >= 65536)
			Local.nowvideoframeno = 1;
		
		//ʱ���
		gettimeofday(&tv, NULL);
		//��һ֡,�趨��ʼʱ���
		if((ref_time.tv_sec ==0)&&(ref_time.tv_usec ==0))
		 {
		  ref_time.tv_sec = tv.tv_sec;
		  ref_time.tv_usec = tv.tv_usec;

     }
    nowtime = (tv.tv_sec - ref_time.tv_sec) *1000 + (tv.tv_usec - ref_time.tv_usec)/1000;
    videorecbuf.timestamp[videorecbuf.iput] = nowtime;
//    printf("nowframeno=%d, videorecbuf.timestamp=%d\n", nowframeno, videorecbuf.timestamp[videorecbuf.iput]);

    dwSize = capture_video(&enc_video_setting);

    if((videorecbuf.iput + 1) >= VNUM)
      videorecbuf.iput = 0;
    else
      videorecbuf.iput ++;
    if(videorecbuf.n < VNUM)
      videorecbuf.n ++ ;
    else
      printf("videorecbuf.Buffer is full\n");

    //����
    pthread_mutex_unlock(&sync_s.video_rec_lock);
    sem_post(&videorecsem);
   }
}


/********С�ſڻ�*************/
int direct_capture_video(video_profile *video)
{
 int rect1width;
 int rect1height;
 int yTop,xLeft;
  int D1_Start, D1_End, D1_deltaX;
  int j; 

  uint8_t *ptr;
  uint8_t *buffer_y;
  uint8_t *buffer_u;
  uint8_t *buffer_v;

    /* Setup to capture the next frame */
    if ((errno=ioctl(video_fd, VIDIOCMCAPTURE, &gb_buf)) < 0) 
    {
      return;
    }

    while (ioctl(video_fd, VIDIOCSYNC, &frame_num) < 0 &&(errno == EAGAIN || errno == EINTR))
        printf("error\n");
    
    ptr = video_buf_virt + gb_buffers.offsets[frame_num];
    frame_num = gb_buf.frame;

    buffer_y = (unsigned char *)(ptr);      //?

    if(video->height % 16 != 0)
        video->height = 16 * ((video->height / 16) + 1 );
    
    if(video->width % 16 != 0)
        video->width = 16 * ((video->width / 16) + 1 );

    // UV    	
    buffer_u = (unsigned char *) (ptr + (video->width * video->height));
    buffer_v = (unsigned char *) (ptr + (video->width * video->height * 5 / 4));

    rect1width = video->width;
    rect1height = video->height;

    if(Local.RecPicSize == 2) //D1
    {
      xLeft = 40;
      yTop = 0;
      D1_Start = 32;
      D1_End = D1_H - 32;
      D1_deltaX = 32;
    }
    else
    {
      xLeft = CIF_X;
      yTop  = CIF_Y;	  
    }

    if(Local.RecPicSize == 1)
    {
      for(j=0;j<rect1height;j++)
        memcpy(fbmem+(j+yTop)*fb_width+xLeft,
               (int)(buffer_y+j*rect1width), rect1width);
      for(j=0;j<rect1height/2;j++)
        memcpy(fbmem + f_data.uv_offset + (j+yTop/2)*fb_width/2 +xLeft/2,
               (int)(buffer_u+j*rect1width/2), rect1width/2);
      for(j=0;j<rect1height/2;j++)
        memcpy(fbmem + f_data.uv_offset+f_data.uv_offset/4 + (j+yTop/2)*fb_width/2 +xLeft/2,
               (int)(buffer_v+j*rect1width/2), rect1width/2);
    }
    if(Local.RecPicSize == 2)
     {
      for(j=0;j<rect1height;j++)
        memcpy(fbmem + f_data.buf_len+(j+yTop)*fb_width+xLeft,
               (int)(buffer_y+j*rect1width), rect1width);
      for(j=0;j<rect1height/2;j++)
        memcpy(fbmem + f_data.buf_len + f_data.uv_offset + (j+yTop/2)*fb_width/2 +xLeft/2,
               (int)(buffer_u+j*rect1width/2), rect1width/2);
      for(j=0;j<rect1height/2;j++)
        memcpy(fbmem + f_data.buf_len + f_data.uv_offset+f_data.uv_offset/4 + (j+yTop/2)*fb_width/2 +xLeft/2,
               (int)(buffer_v+j*rect1width/2), rect1width/2);
     }

    return 0;
}
//---------------------------------------------------------------------------
int mpeg4_capture_init(int enc_width, int enc_height, int framerate, int YUVFormat)
{
    int ret;
    int i;
    int blength;

    enc_video_setting.roi_enable = 0;
    //set the default value
    enc_video_setting.qmax = 31;
    enc_video_setting.qmin = 1;
    enc_video_setting.quant = 4;
    enc_video_setting.bit_rate = 1024;
    printf("enc_video_setting.bit_rate = %d\n", enc_video_setting.bit_rate);
    enc_video_setting.width = enc_width;
    enc_video_setting.height = enc_height;
    enc_video_setting.framerate = framerate;
    enc_video_setting.frame_rate_base = 1;
    enc_video_setting.gop_size = 50;

    enc_video_setting.roi_x = MP4_ROIX;
    enc_video_setting.roi_y = MP4_ROIY;
    enc_video_setting.roi_width = 320;
    enc_video_setting.roi_height = 240;

    if (enc_video_setting.roi_enable )
       printf("roi width %d height %d\n", enc_video_setting.roi_width, enc_video_setting.roi_height);
    else
       printf("frame width %d height %d\n", enc_video_setting.width, enc_video_setting.height);

   //1 for mpeg4  ԭ���������
    //JPEG need sequential format
    // 0 for Mpeg4 2D YUV
    // 1 for JPG Sequencoal YUV
    // 2 for H.264 2D
    if((ret=v4l_init(&enc_video_setting, YUVFormat,7)) < 0)
//    if((ret=v4l_init(&enc_video_setting, 0)) < 0)
        return -1;
    return 0;
}

void StartDirectVideo(int width, int height)
{
  pthread_attr_t attr;
  int i, j;
  int framerate;
  if(VideoRecIsStart == 0)
  {
    	VideoRecIsStart = 1;
		
    	if((width == D1_W)&&(height == D1_H))
     	{
      		Local.RecPicSize = 2;      //720*480
      		if(Local.CurrFbPage != 1)
       		{
        		Local.CurrFbPage = 1;
        		fb_setpage(fbdev, Local.CurrFbPage);
        		TalkOsd();  //ͨ���ͼ���ʱOSD��ʾ
       		}
      		framerate = 5;
     	}
    	else
     	{
      		Local.RecPicSize = 1;      //352*288
      		framerate = 30;
     	}

    	//��ͨ��Ƶ
    	ioctl(gpio_fd, IO_CLEAR, MINIDOOR_POWER_IO);
    	usleep(200*1000);   //����ͷ�ϵ磬�ȴ���Ƶ�ȶ�

    	if(VideoRecIsStart == 0)
      	return;
		
    	//��Ƶ�ɼ�������
    	framerate = 25;
    // 0 for Mpeg4 2D YUV
    // 1 for JPG Sequencoal YUV
    // 2 for H.264 2D
    	if(mpeg4_capture_init(width, height, framerate, 1) !=0 )
     	{
      		printf("mpeg4_capture_init error!\\n");
      		return;
     	}

    	Local.nowvideoframeno = 1;
    //��Ƶ�ɼ�����
    	videorecbuf.iput = 0;
    	videorecbuf.iget = 0;
    	videorecbuf.n = 0;
    	for(i=0; i<VNUM; i++)
     	{
      		videorecbuf.frameno[i] = 0;
      		videorecbuf.timestamp[i] = 0;
     	}

//    	sem_init(&videorecsem,0,0);
    	pthread_mutex_init (&sync_s.video_rec_lock, NULL);

    	video_direct_rec_flag = 1;   //��Ƶ�ɼ���־
    	pthread_attr_init(&attr);
    	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);    
    	pthread_create(&video_direct_rec_thread,&attr,(void *)video_direct_rec_thread_func, NULL);
    	pthread_attr_destroy(&attr);
    	if ( video_direct_rec_flag == 0 ) {
          printf("�޷�������Ƶ�ɼ��߳�\n");
          return;
      }            
   }
  else
   {
    #ifdef _DEBUG
      printf("�ظ� VideoRecStart\n");
    #endif
   }
}
//---------------------------------------------------------------------------
void StopDirectVideo(void)
{
  int i;
  int delaytime;
  delaytime=40;
  if(VideoRecIsStart == 1)
   {
    VideoRecIsStart = 0;
    video_direct_rec_flag = 0;   //��Ƶ�ɼ���־
    usleep(delaytime*1000);
    if(pthread_cancel(video_direct_rec_thread) ==0)
      printf("video_direct_rec_thread cancel success\n");
    else
      printf("video_direct_rec_thread cancel fail\n");
    usleep(delaytime*1000);
//    sem_destroy(&videorecsem);
    pthread_mutex_destroy(&sync_s.video_rec_lock);

    //����
    v4l_close();
    //�ر���Ƶ
    ioctl(gpio_fd, IO_PUT, MINIDOOR_POWER_IO);
    printf("ddddddddddddddddd\n");
    //Ϊ720*480
    if(Local.RecPicSize == 2)
     {
      Local.RecPicSize = 1;
     }
    //����FBҳ��
    if(Local.CurrFbPage != 0)
     {
      Local.CurrFbPage = 0;
      TalkOsd();  //ͨ���ͼ���ʱOSD��ʾ
      fb_setpage(fbdev, Local.CurrFbPage);
     }
   }
  else
   {
    #ifdef _DEBUG
      printf("�ظ� VideoRecStop\n");
    #endif
   }
}
//---------------------------------------------------------------------------
void video_direct_rec_thread_func(void)
{
  struct timeval tv;
  uint32_t nowtime,prevtime;
  int dwSize;
  #ifdef _DEBUG
    printf("����С�ſڻ���Ƶ�ɼ��̣߳�\n" );
    printf("video_direct_rec_flag=%d\n",video_direct_rec_flag);
  #endif
  gettimeofday(&tv, NULL);
  nowtime = tv.tv_sec *1000 + tv.tv_usec/1000;
  prevtime = nowtime;
  
  while(video_direct_rec_flag == 1)
  {
    //����
    pthread_mutex_lock(&sync_s.video_rec_lock);

    videorecbuf.frameno[videorecbuf.iput] = Local.nowvideoframeno;
    Local.nowvideoframeno++;
    if(Local.nowvideoframeno >= 65536)
        Local.nowvideoframeno = 1;

    //ʱ���
    gettimeofday(&tv, NULL);
    //��һ֡,�趨��ʼʱ���
    if((ref_time.tv_sec ==0)&&(ref_time.tv_usec ==0))
     {
      ref_time.tv_sec = tv.tv_sec;
      ref_time.tv_usec = tv.tv_usec;
     }
    nowtime = (tv.tv_sec - ref_time.tv_sec) *1000 + (tv.tv_usec - ref_time.tv_usec)/1000;
    videorecbuf.timestamp[videorecbuf.iput] = nowtime;
    dwSize = direct_capture_video(&enc_video_setting);

    //����
    pthread_mutex_unlock(&sync_s.video_rec_lock);
   }
}
