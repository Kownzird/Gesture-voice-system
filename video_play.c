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

int fi;
extern int mute;
extern int voice;

void send_cmd(char *cmd)
{
	write(fi,cmd,strlen(cmd));
}

void mplayer_init(void)
{
	if(access("/tmp/fifo",F_OK))
	{
		mkfifo("/tmp/fifo",0777);
	}
	
	fi = open("/tmp/fifo",O_RDWR);
}

//开始播放视频
void video_play(void)
{
	mplayer_init();
	
	system("mplayer -slave -quiet -input file=/tmp/fifo -geometry 0:0 -zoom -x 800 -y 480 Onepiece.avi &");//Faded3  Onepiece
	
}

//退出视频
void video_quit(void)
{
	printf("quit\n");				
	send_cmd("quit\n");
	lcd_draw_jpg_file(0,0,"stop_video.jpg");
	lcd_draw_jpg_file(0,0,"stop_video.jpg");
	lcd_draw_jpg_file(0,0,"stop_video.jpg");	
}

//视频静音
void video_mute(void)
{
	mute=!mute;
	if(mute)
	{
		//静音开
		printf("mute\n");
		send_cmd("mute 1\n");
	}
	else
	{
		//静音关
		printf("no mute\n");
		send_cmd("mute 0\n");
	}
}

//暂停或播放视频
void video_pause(void)
{
	printf("pause or play\n");
	send_cmd("pause\n");
}

//增加音量
void video_voice_up(void)
{
	printf("\ntop to button\n");
	if(voice>=100)
	{
		printf("没法更大声了");
		printf("voice = %d",voice);
	}
	else
	{
		voice = voice+10;
		sprintf(voice_buf,"%d",voice);
		strcpy(cmd_voice_buf,"volume ");
		strcat(cmd_voice_buf,voice_buf);
		strcat(cmd_voice_buf," 1\n");
		printf("%s\n",cmd_voice_buf);
		send_cmd(cmd_voice_buf);
	}
}

//减小音量
void video_voice_down(void)
{
	printf("\nbutton to top\n");
	if(voice <= 0)
	{
		printf("没法更小声了");
		printf("voice = %d",voice);
	}
	else
	{
		voice = voice-10;
		
		//itoa(voice,voice_buf,10);
		sprintf(voice_buf,"%d",voice);
		strcpy(cmd_voice_buf,"volume ");
		strcat(cmd_voice_buf,voice_buf);
		strcat(cmd_voice_buf," 1\n");
		printf("%s",cmd_voice_buf);
		send_cmd(cmd_voice_buf);
	}
}

//后退10秒
void video_backward(void)
{
	printf("seek -10\n");
	send_cmd("seek -10\n");	
}

//前进10秒
void video_forward(void)
{
	printf("seek 10\n");
	send_cmd("seek 10\n");
}
		
