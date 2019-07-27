#include <sys/types.h>         
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <linux/input.h>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <pthread.h>
#include "libxml/xmlmemory.h"
#include "libxml/parser.h"
#include "api_v4l2.h"
#include "lcd.h"
#include "jpeglib.h"
#include "password.h"
#include "video_play.h"
#include "gesture.h"
#include "xml.h"


int socket_fd;
static FrameBuffer camera_frame;
char jpgname[4][8]={"1.jpg","2.jpg","3.jpg","4.jpg"};
char photo[4][7]={"p1.jpg","p2.jpg","p3.jpg","p4.jpg"};
static int sent=0; //数据传输标志位
static int eid=-1;
int ei=0;
int quit_camera=0;
static int video_flag=0;
int snap=0,show=0,pic_show=0;

int x_min=0,y_min=0;
int x, y, x1, x_n, y_n, x_c, y_c;
int x_slip=0,y_slip=0;

int mute; //静音标志位
int voice=50;
int vedio_time = 0;
char voice_buf[10]={0};
char cmd_voice_buf[30]={0};
char cmd_time_buf[30]={0};

int sound_mode=0;
int gesture_video;
int quit_program=0;

char d7_on[2] ={7,1};
char d7_off[2]={7,0};
char d8_on[2] ={8,1};
char d8_off[2]={8,0};
char d9_on[2] ={9,1};
char d9_off[2]={9,0};
char d10_on[2] ={10,1};
char d10_off[2]={10,0};


static char *leds_on_tbl[4]={
	d7_on,d8_on,d9_on,d10_on
};

static char *leds_off_tbl[4]={
	d7_off,d8_off,d9_off,d10_off
};


void *thread_ts(void *parg)
{
	struct input_event input_buf;
	int fd,press1,press2,press3,i=0,j=0;
	fd = open("/dev/input/event0",O_RDWR);
	
	while(1)
	{	
		//读取坐标值
		read(fd,&input_buf,sizeof(input_buf));
		
		if(input_buf.type == EV_ABS && input_buf.code == ABS_X)
		{
			x = input_buf.value;
			
		}
		if(input_buf.type == EV_ABS && input_buf.code == ABS_Y)
		{
			y = input_buf.value;
		}
		
		//判断当前的释放状态，触摸屏按下和释放是模拟按键的
		if(input_buf.type == EV_KEY && input_buf.code == BTN_TOUCH )
		{
			//松开
			if(input_buf.value == 0)
			{
				write(fd,leds_off_tbl[2],2); //D9_OFF
				if(x > 400 && x < 720 && y > 70 && y < 150 && (sound_mode==0) && (gesture_mode==0)) //语音模式
				{
					sound_mode=1;
					lcd_draw_jpg_file(0,0,"voice_mode.jpg");  //语音界面
					lcd_draw_jpg_file(0,0,"voice_mode.jpg");
					system("madplay -a -30 enter_voice_mode.mp3");
				}
				else if(x > 400 && x < 720 && y > 200 && y < 280 && (sound_mode==0) && (gesture_mode==0)) //手势模式
				{
					gesture_mode=1;
					lcd_draw_jpg_file(0,0,"gesture_mode.jpg");  //手势界面
					lcd_draw_jpg_file(0,0,"gesture_mode.jpg");
					system("madplay -a -30 enter_gesture_mode.mp3");
				}
				else if(x > 400 && x < 720 && y > 320 && y < 400 && (sound_mode==0) && (gesture_mode==0)) //退出程序
				{
					quit_program=1;
					lcd_draw_jpg_file(0,0,"goodbye.jpg");
					lcd_draw_jpg_file(0,0,"goodbye.jpg");
					system("madplay -a -30 goodbye.mp3");
					exit(0);
				}
				//拍照
				else if(x > 700 && x < 791 && y >197 && y < 285 && press1 == 1 && show != 1)
				{
					printf("you have got a snap!\n");
					lcd_draw_jpg_file(0,0,"camera.jpg");
					/* 创建jpg图片 */
					i++;
					if(i==4) i=0;
					linux_v4l2_save_image_file(photo[i],camera_frame);	
					lcd_draw_jpg_file_ex(710,380,photo[i]);
					snap=1;
				}
				//浏览图片
				else if(x > 707 && x < 791 && y >375 && y < 460 && press2 == 1 && show == 0)
				{
					printf("browse pictures!\n");
					lcd_draw_jpg_file(0,0,photo[i]);
					show=1;
					j=i;
					pic_show=1;
				}
				//结束浏览图片
				else if(x > 707 && x < 791 && y >375 && y < 460 && press2 == 1 && show == 1)
				{
					printf("camera on!\n");
					show=0;
					pic_show=0;
				}
				//浏览状态下判断向左向右
				else if(press3==1 && x<x1)
				{
					printf("next photo!\n");
					if(j==i) ;
					else
					{
						if(j==3) j=-1;
						j++;
						lcd_draw_jpg_file(0,0,photo[j]);
					}
				}
				else if(press3==1 && x>x1)
				{
					printf("last photo!\n");
					if(j==i+1||i==3 && j==0 ) ;
					else
					{
						if(j==0) j=4;
						j--;
						lcd_draw_jpg_file(0,0,photo[j]);
					}
				}
				
				//正常模式
				else if(video_flag==0 && pic_show == 0)
				{
					lcd_draw_jpg_file(0,0,"ack.jpg");
					sent=1;
					quit_camera=1;
					usleep(10*1000);
					
				}
			
				//视频模式
				else if(video_flag==1)
				{
					x_n = x;
					y_n = y;
					
					//左上角退出
					if(x>0 && x<200 && y>0 && y<100)
					{
						video_quit(); //退出视频
						video_flag=0; //清除播放视频标志位
					}
					
					//右上角静音
					if(x>600 && x<800 && y>0 && y<100)
					{
						video_mute(); //静音
					}
					
					//中间暂停
					if(x>150 && x<650 && y>100 && y<400)
					{
						video_pause(); //暂停或播放视频
					}
					
					//从上往下滑动100，音量+10
					if(y_slip > 100)
					{
						video_voice_up(); //增加音量
					}
					
					//从下往上滑动100，音量-10
					if(y_slip < -100)
					{
						video_voice_down(); //减小音量
					}
					
					//从右往左滑动200，时间-10
					if(x_slip > 100)
					{
						video_backward(); //后退10秒
					}
					
					//从左往右滑动200，时间+10
					if(x_slip < -100)
					{
						video_forward(); //前进10秒
					}
				}
				press1=0;
				press2=0;
				press3=0;	
			}
			else
			{
				write(fd,leds_on_tbl[2],2); //D9_ON
				//拍照
				if(x > 700 && x < 791 && y >197 && y<285 && show != 1)
				{
					lcd_draw_jpg_file(0,0,"record.jpg");
					press1=1;
				}
				//预览
				else if(x > 700 && x < 791 && y >375 && y<460)
				{
					press2=1;
				}	
				//滑动浏览
				else if(x <= 640 && show == 1)
				{
					press3=1;
					x1=x;
				}
				
				x_c = x;
				y_c = y;
				
			}
			x_slip = x_c - x_n;
			y_slip = y_c - y_n;
			
		}
		usleep(10*1000);
	}
	
	return 0;
}

void *thread_gesture(void *parg)
{
	gesture();
	//system("killall -9 madplay");
	//system("madplay -a -30 1.mp3 -r");
	return 0;
}

void *thread_music(void *parg)
{
	system("killall -9 madplay");
	system("madplay -a -30 1.mp3 -r");
	return 0;
}


void *thread_send(void *parg)
{
	//开始录音
	system("arecord -d3 -c1 -r16000 -traw -fS16_LE cmd.pcm");	
	//向语音识别引擎系统发送cmd.pcm
	tcp_send_pcm(socket_fd,"cmd.pcm");
	
		
	//从语音识别引擎系统接收XML结果，并将结果保存到result.xml	
	tcp_recv_xml(socket_fd);	
	
	//分析result.xml
	xmlChar *id = parse_xml("result.xml");
	if(id)
	{
		//打印id号
		printf("id=%s\n",id);
				
		//根据id号响应不同的操作
		eid=atoi(id);
		choose=1;
	}
	
	return 0;
}

int main (int argc,char **argv)
{

	struct timeval timeout={5,0};//设置发送超时
	
	pthread_t tid_send;
	pthread_t tid_ts;
	pthread_t tid_music;
	pthread_t tid_gesture;
	int rt;
	int len;
	int i=0;
	int fd_led = -1;
	
	lcd_open();
	
	password();
	
	//创建套接字，协议为IPv4，类型为TCP
	socket_fd = socket(AF_INET,SOCK_STREAM,0);
	
	if(socket_fd<0)
	{
		perror("create socket fail:");
		
		return -1;
	}
	
	fd_led = open("/dev/myleds",O_RDWR);
	if(fd_led < 0)
	{
		
		perror("open /dev/myleds:");
		
		return fd_led;
	}
	
	struct sockaddr_in	 dest_addr;
	
	dest_addr.sin_family 		= AF_INET;					//IPv4
	dest_addr.sin_port   		= htons(54321);				//目的端口为54321
	dest_addr.sin_addr.s_addr	= inet_addr("192.168.18.56");	//目的IP地址填写
	
	//设置发送超时
	 rt = setsockopt(socket_fd,SOL_SOCKET,SO_SNDTIMEO,(const char *)&timeout,sizeof timeout);
	 if(rt < 0)
	 {
	  perror("setsockopt SO_RCVTIMEO:");
	  return -1;
	 }
	 
	 //设置接收超时
	 rt = setsockopt(socket_fd,SOL_SOCKET,SO_RCVTIMEO,(const char *)&timeout,sizeof timeout);
	 if(rt < 0)
	 {
	  perror("setsockopt SO_RCVTIMEO:");
	  return -1;
	 }

	
	//要跟服务器建立连接（等同于登陆游戏）
	rt=connect(socket_fd,(struct sockaddr *)&dest_addr,sizeof dest_addr);
	
	if(rt < 0)
	{
		write(fd_led,leds_off_tbl[1],2);
		perror("connect fail:");
		return -1;
	}
	else
	{
		write(fd_led,leds_on_tbl[1],2);
	}
	
	pthread_create(&tid_ts,NULL,thread_ts,NULL);
	pthread_create(&tid_gesture,NULL,thread_gesture,NULL);
	
	//启动画面
	lcd_draw_jpg_file(0,0,"welcome.jpg");
	system("madplay -a -30 welcome.mp3");
	//打开摄像头设备
	linux_v4l2_device_init("/dev/video7",640,480);

	//启动摄像头捕捉
	linux_v4l2_start_capturing();
	
	//根据id号响应不同的操作
	while(1)
	{
		
		if((sound_mode==1)&&(gesture_mode==0))  //语音模式
		{
			if(sent==1)
			{
				sent=0;
				pthread_create(&tid_send,NULL,thread_send,NULL);
			}
			if(choose==1)
			{
				if(ei==1 && eid == 1)
				{
				//拍照
					lcd_draw_jpg_file(0,0,"camera.jpg");
					quit_camera=0;
					while(quit_camera==0)
					{
						//获取摄像头的数据
						linux_v4l2_get_frame(&camera_frame);
							
						//显示摄像头的数据
						lcd_draw_camera(0,0,camera_frame);
						
						lcd_draw_jpg_file_ex(710,380,photo[i]);
						
						//拍照保存照片
						if(snap==1 && show!=1)
						{
							snap=0;
						}
						//浏览图片状态下先展示最后一张，然后等待show变化
						if(show==1)
						{
							while(show==1) usleep(10*1000);
						}
					}
					
					printf("take photo\n");
					lcd_draw_jpg_file(0,0,"ack.jpg");
					//system("madplay -a -30 show_picture.mp3");
				}
				
				if(ei==1 && eid == 2)
				{
				//开灯
					write(fd_led,leds_on_tbl[0],2);  //D7_ON
					printf("open light\n");
					lcd_draw_jpg_file(0,0,"light_on.jpg");
					system("madplay -a -30 light_on.mp3");
				}
				
				if(ei==1 && eid == 3)
				{
				//关灯
					write(fd_led,leds_off_tbl[0],2); //D7_OFF
					printf("close light\n");
					lcd_draw_jpg_file(0,0,"light_off.jpg");
					system("madplay -a -30 light_off.mp3");
				}
				
				if(ei==2 && eid == 1)
				{
				//播放图片
					printf("shou picture\n");
					lcd_draw_jpg_file(0,0,"1.jpg");
					system("madplay -a -30 show_picture.mp3");
				}
				if((ei==2 && eid == 2))
				{
				//切换下一张图片
					if(i==3) i=-1;
					printf("next picture\n");
					lcd_draw_jpg_file(0,0,jpgname[++i]);
					system("madplay -a -30 next_picture.mp3");
				}
						
				if((ei==2 && eid == 3))
				{
					//切换上一张图片
					if(i==0) i=4;
					printf("last piture\n");
					lcd_draw_jpg_file(0,0,jpgname[--i]);	
					system("madplay -a -30 last_picture.mp3");
				}	
				if(ei==3 && eid == 1)
				{
					//播放音乐
					printf("music play\n");
					lcd_draw_jpg_file(0,0,"play_music.jpg");
					system("madplay -a -30 play_music.mp3");
					pthread_create(&tid_music,NULL,thread_music,NULL);
				}
				
				if(ei==3 && eid == 2)
				{
					//停止音乐
					printf("music play\n");
					lcd_draw_jpg_file(0,0,"stop_music.jpg");
					system("killall -9 madplay");
					system("madplay -a -30 stop_music.mp3");
				}
				
				if(ei==3 && eid == 3)
				{
					
					//播放视频
					system("killall -9 mplayer");
					video_flag=1;
					printf("viode play\n");
					video_play();
				}
				
				if(ei==4 && eid == 1)
				{
					//路飞同学
					printf("i am here\n");
					lcd_draw_jpg_file(0,0,"ack.jpg");
					system("killall -9 madplay");
					system("madplay -a -30 i_am_here.mp3");
				}
				if((ei==4 && eid == 2)||(ei==4 && eid==3))
				{
					//退出
					printf("quit voice mode\n");
					lcd_draw_jpg_file(0,0,"stop_voice_mode.jpg");
					system("killall -9 madplay");
					system("madplay -a -30 stop_voice_mode.mp3");
					sound_mode=0;//退出语音模式
				}	
				choose=0;
			}
			usleep(20*1000);
		}
		else if((sound_mode==0)&&(gesture_mode==1))  //手势模式
		{
			
			usleep(10*1000);
			//printf("111111111111111\n");
			if(choose==1)
			{
				if(gesture_value==7)
				{
					//拍照
					lcd_draw_jpg_file(0,0,"camera.jpg");
					quit_camera=0;
					while(quit_camera==0)
					{
						//获取摄像头的数据
						linux_v4l2_get_frame(&camera_frame);
							
						//显示摄像头的数据
						lcd_draw_camera(0,0,camera_frame);
						
						lcd_draw_jpg_file_ex(710,380,photo[i]);
						
						//拍照保存照片
						if(snap==1 && show!=1)
						{
							snap=0;
						}
						//浏览图片状态下先展示最后一张，然后等待show变化
						if(show==1)
						{
							while(show==1) usleep(10*1000);
						}
					}
					
					printf("take photo\n");
					lcd_draw_jpg_file(0,0,"ack.jpg");
				}
				
				if(gesture_value==8)
				{
					//逆时针退出拍照模式
					quit_camera=1;
					printf("quit camera\n");
					lcd_draw_jpg_file(0,0,"stop_camera.jpg");	
				}

				if(gesture_value==3)
				{
					//切换上一张图片
					if(i==0) i=4;
					printf("last piture\n");
					lcd_draw_jpg_file(0,0,jpgname[--i]);	
					system("madplay -a -30 last_picture.mp3");
				}
				
				if(gesture_value==4)
				{
				//切换下一张图片
					if(i==3) i=-1;
					printf("next picture\n");
					lcd_draw_jpg_file(0,0,jpgname[++i]);
					system("madplay -a -30 next_picture.mp3");
				}
						

				if(gesture_value==1)
				{
					//播放音乐
					printf("music play\n");
					lcd_draw_jpg_file(0,0,"play_music.jpg");
					system("killall -9 madplay");
					system("madplay -a -30 play_music.mp3");
					system("madplay -a -30 1.mp3 -r");
				}
				
				if(gesture_value==2)
				{
					//停止音乐
					printf("music play\n");
					lcd_draw_jpg_file(0,0,"stop_music.jpg");
					system("killall -9 madplay");
					system("madplay -a -30 stop_music.mp3");
				}
				
				if(gesture_value==5)
				{	
					//播放视频
					system("killall -9 mplayer");
					video_flag=1;
					gesture_video=1;
					printf("viode play\n");
					video_play();
				}
				
				if(gesture_value==6)
				{
					//停止视频
					system("killall -9 mplayer");
					lcd_draw_jpg_file(0,0,"stop_video.jpg");
					lcd_draw_jpg_file(0,0,"stop_video.jpg");
				}
				
				
				if(gesture_value==9)
				{
					//退出
					printf("quit gesture mode\n");
					lcd_draw_jpg_file(0,0,"stop_gesture_mode.jpg");
					system("killall -9 madplay");
					system("madplay -a -30 stop_gesture_mode.mp3");
					gesture_mode=0;//退出手势模式
				}
			}
			choose=0;
		}
		else
		{
			if(quit_program==0)
			lcd_draw_jpg_file(0,0,"main.jpg");  //模式主界面
		}
	}
	close(fd_led);
	//关闭套接字
	close(socket_fd);
	lcd_close();
	
	return 0;
}

