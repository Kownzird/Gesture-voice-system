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
#include "video_play.h"
#include "gesture.h"



/*note: 
	PAJ7620U2-手势探测驱动驱动文件：/dev/IIC_drv
	接口函数：open   read   close
	通信协议：用read函数每次只能读取1个字节并且100ms后会后返回值
				返回值--> -1:没有探测到手势    1-9：分别对应9中手势
				
	
	接线说明： 模块J1<--连接--->GEC6818J10
			     VCC			PIN1
				 GND			PIN19/20
				 SCL			PIN15
				 SDA			PIN17
*/
int mute=0;
int voice;
int gesture_mode=0;
int gesture_video=0;
int choose=0;
int gesture_value=0;
int quit_camera;

char voice_buf[10];
char cmd_voice_buf[30];

static char *leds_on_tbl[4];
static char *leds_off_tbl[4];

extern char d7_on[2];
extern char d7_off[2];

extern char d8_on[2];
extern char d8_off[2];

extern char d9_on[2];
extern char d9_off[2];

extern char d10_on[2];
extern char d10_off[2];

int gesture(void)
{

	int fd;
	char tmp;
	//打开手势识别模块的驱动程序
	fd = open("/dev/IIC_drv", O_RDWR);
	if(fd < 0)
	{
		perror("打开手势识别模块的驱动程序失败!\n");
		return -1;	
	}
	
	while(1)
	{
		if(gesture_mode==1 && gesture_video== 0)
		{
			read(fd,&tmp,1);
			if(tmp>=1 && tmp<=9)
			{
				printf("%x\n",tmp);
				switch(tmp)
				{
					
					case 1:
					{
						gesture_value=1;
						choose=1;
						printf("test-Up\n");
						break;//向上
					}
					case 2:
					{
						system("killall -9 madplay");
						gesture_value=2;
						choose=1;
						printf("test-Down\n");
						break;//向下
					}
					case 3:
					{
						gesture_value=3;
						choose=1;
						printf("test-Left\n");  
						break;//向左
					}
					case 4:
					{
						gesture_value=4;
						choose=1;
						printf("test-Right\n");
						break;//向右
					}
					case 5:
					{
						gesture_value=5;
						choose=1;
						printf("test-Forward\n");
						break;//向前
					}
					case 6:
					{
						gesture_value=6;
						choose=1;
						printf("test-Backward\n");
						break;//向后
					}
					case 7:
					{
						gesture_value=7;
						choose=1;
						printf("test-Clockwise\n");
						break;//顺时针
					}
					case 8:
					{
						quit_camera=1;
						gesture_value=8;
						choose=1;
						printf("test-AntiClockwise\n");
						break; //逆时针
					}
					case 9:
					{
						
						gesture_value=9;
						choose=1;
						system("killall -9 mplayer");
						printf("test-Wave\n");
						break;//挥动
					}
					default:break;
				}
				
			}
		}
		if(gesture_mode==1 && gesture_video== 1)  //手势控制视频
		{
			read(fd,&tmp,1);
			if(tmp>=1 && tmp<=9)
			{
				printf("%x\n",tmp);
				switch(tmp)
				{
					case 1:
					{
						video_voice_up(); //增加音量
						printf("test-Up\n");
						break;//向上
					}
					case 2:
					{
						video_voice_down(); //减小音量
						printf("test-Down\n");
						break;//向下
					}
					case 3:
					{
						video_backward(); //后退10秒
						printf("test-Left\n");  
						break;//向左
					}
					case 4:
					{
						video_forward(); //前进10秒
						printf("test-Right\n");
						break;//向右
					}
					case 5:
					{
						video_pause(); //暂停或播放视频
						printf("test-Forward\n");
						break;//向前
					}
					case 6:
					{
						video_mute(); //视频静音
						printf("test-Backward\n");
						break;//向后
					}
					case 7:
					{
						printf("test-Clockwise\n");
						break;//顺时针
					}
					case 8:
					{
						printf("test-AntiClockwise\n");
						break; //逆时针
					}
					case 9:
					{
						//退出
						printf("quit\n");				
						send_cmd("quit\n");
						gesture_video=0;
						gesture_value=6;
						choose=1;
						printf("test-Wave\n");
						break;//挥动
					}
					default:break;
				}
				
			}
		}
		 
		sleep(1);
		//printf("gesture_flag=%d\n",gesture_flag);
		
	}
	close(fd);
}
