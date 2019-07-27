#include <fcntl.h>
#include <linux/input.h>
#include <string.h>
#include <stdio.h>   		 	 
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <linux/videodev2.h>
#include "common.h"
#include "api_v4l2.h"
#include "lcd.h"
#include "jpeglib.h"
#include "password.h"

/*
static char color_buf[800*480*4]={0};
static char bmp_buf[800*480*3]={0};

int lcd_draw_bmp(const char *pathname)
{
	int fd_lcd;
	int fd_bmp;
	int i=0,j=0;
	
	
	//以只读的方式，打开1.bmp
	fd_bmp=open(pathname,O_RDONLY);
	
	if(fd_bmp < 0)
	{
		printf("open bmp_file fail\n");
		
		return -1;
	}
	
	
	//定位到RGB数据的起始位置(偏移文件头部54个字节)
	lseek(fd_bmp,54,SEEK_SET);	
	
	//读取所有的RGB数据
	read(fd_bmp,bmp_buf,800*480*3);
	
	//不再使用该图片，就关闭
	close(fd_bmp);
	
	//以可读可写的方式，打开LCD设备
	fd_lcd=open("/dev/fb0",O_RDWR);
	
	if(fd_lcd < 0)
	{
		printf("open lcd fail\n");
		
		return -1;
	}
	
	
	//定位到显示的起始位置(偏移文件头部0个字节)
	lseek(fd_lcd,0,SEEK_SET);
	
	//向LCD设备写入要显示的数据
	for(i=0,j=0; i<800*480*4; i+=4,j+=3)
	{
		color_buf[i] 	= bmp_buf[j];		//b
		color_buf[i+1] 	= bmp_buf[j+1];		//g	
		color_buf[i+2] 	= bmp_buf[j+2];		//r
		color_buf[i+3] 	= 0;		//a	
		
	}
	
	for(i=479; i>=0; i--)
		write(fd_lcd,&color_buf[800*i*4],800*4);
	for(i=479; i>=0; i--)
		write(fd_lcd,&color_buf[800*i*4],800*4);	
	

	//关闭lcd设备
	close(fd_lcd);
}

*/

void password()
{
	struct input_event input_buf;
	int fd,x,y,fp_pass;
	int z = 0, i=0;
	fd = open("/dev/input/event0",O_RDWR);
	
	lcd_draw_jpg_file(0,0,"pass_login.jpg");
	
	char password[60]={0};
	char password1[6]={0};
	int del=0, ent=0, right=0;
	fp_pass = open("pass.txt", O_RDONLY);
	read(fp_pass, password1, sizeof password1);
	close(fp_pass);
	while(1)
	{
		//读取坐标值
		read(fd,&input_buf,sizeof(input_buf));
		
		//判断是否绝对坐标值，EV_ABS，也就是触摸屏事件
		//判断是返回的是X坐标还是Y坐标
		if(input_buf.type == EV_ABS && input_buf.code == ABS_X)
		{
			x = input_buf.value;
		}		
		if(input_buf.type == EV_ABS && input_buf.code == ABS_Y)
		{
			y = input_buf.value;
		}
		if(input_buf.type == EV_KEY && input_buf.code == BTN_TOUCH )
		{
			//松开
			if(input_buf.value == 0)
			{
				if(x>221 && x<328)
				{
					if(y>110 && y<192)
					{
						printf("1");
						password[z]='1';
						z += 1;
					}
					if(y>=192 && y<271)
					{
						printf("4");
						password[z]='4';
						z += 1;
					}
					if(y>=271 && y<336)
					{
						printf("7");
						password[z]='7';
						z += 1;
					}
					if(y>=336 && y<420)
					{
						del = 1;
					}
				}
				if(x>=328 && x<450)
				{
					if(y>110 && y<192)
					{
						printf("2");
						password[z]='2';
						z += 1;
					}
					if(y>=192 && y<271)
					{
						printf("5");
						password[z]='5';
						z += 1;
					}
					if(y>=271 && y<336)
					{
						printf("8");
						password[z]='8';
						z += 1;
					}
					if(y>=336 && y<420)
					{
						printf("0");
						password[z]='0';
						z += 1;
					}
				}
				if(x>=450 && x<580)
				{
					if(y>110 && y<192)
					{
						printf("3");
						password[z]='3';
							z += 1;
					}
					if(y>=192 && y<271)
					{
						printf("6");
						password[z]='6';
							z += 1;
					}
					if(y>=271 && y<336)
					{
						printf("9");
						password[z]='9';
							z += 1;
					}
					if((y>=336 && y<420)||z==6)
					{
						printf("\nwait..\n");
						ent = 1;
					}
					if(z==6) ent = 1;
				}	
			}
		}
		if (del==1){
			if(z==0) continue;
			password[--z] = 0;
			del = 0;
			printf("\b");
		}
		if (ent==1){
			if(z<5)
			{
				printf("wrong numbers!try again\n");
				z= 0;
				ent =0 ;
				continue;
			}
			printf("Your input is ");
			for(i=0;i<z;i++){
				printf("%c", password[i]);
			}
			printf("\n");
			right = 0;
			for (i=0;i<6;i++){
				if(password[i] == password1[i]){
					right += 1;
				}
			}
			if(right==6){
				close(fd);
				printf("you are welcomed\n");
				break;
			}
			else{
			printf("wrong password!try again\n");
			z= 0;
			ent =0 ;
			}
		}
		
	}

}


