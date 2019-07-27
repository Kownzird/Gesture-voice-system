/****************************************************************************************
 *文件名称:lcd.c
 *设    计:温子祺
 *日    期:2015-5-29
 *说	明:显示jpg文件
 ----------------------------------------------------------------------------------------
 *修改日期:2018-6-25
 *说	明:显示jpg、bmp文件，jpg与bmp数据、纯rgb数据。
 ----------------------------------------------------------------------------------------
 *修改日期:2018-7-3
 *说	明:添加了lcd_draw_camera函数，自动识别yuyv格式或jpg格式摄像头
			并显示。
****************************************************************************************/
#include <stdio.h>   	//printf scanf
#include <fcntl.h>		//open write read lseek close  	 
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <linux/videodev2.h>

#include "common.h"
#include "api_v4l2.h"
#include "lcd.h"
#include "jpeglib.h"


static unsigned char g_color_buf[FB_SIZE]={0};

static int  g_fb_fd;
static int *g_pfb_memory;


//初始化LCD
int lcd_open(void)
{
	g_fb_fd = open("/dev/fb0", O_RDWR);
	
	if(g_fb_fd<0)
	{
			printf("open lcd error\n");
			return -1;
	}

	g_pfb_memory  = (int *)mmap(	NULL, 					//映射区的开始地址，设置为NULL时表示由系统决定映射区的起始地址
									FB_SIZE, 				//映射区的长度
									PROT_READ|PROT_WRITE, 	//内容可以被读取和写入
									MAP_SHARED,				//共享内存
									g_fb_fd, 				//有效的文件描述词
									0						//被映射对象内容的起点
								);

	return g_fb_fd;

}

//LCD画点
void lcd_draw_point(unsigned int x,unsigned int y, unsigned int color)
{
	*(g_pfb_memory+y*800+x)=color;
}

//BMP数据
int lcd_draw_bmp(unsigned int x,unsigned int y,unsigned char *bmp_buf,unsigned int bmp_buf_size)   
{
	unsigned int blue, green, red;
	unsigned int color;
	unsigned int bmp_width;
	unsigned int bmp_height;
	unsigned int bmp_type;
	unsigned int bmp_size;
	unsigned int x_s = x;
	unsigned int x_e ;	
	unsigned int y_e ;
	unsigned char buf[54]={0};
	unsigned char *pbmp_buf;		 
	
	memcpy(buf,bmp_buf,54);
	
	/* 宽度  */
	bmp_width =buf[18];
	bmp_width|=buf[19]<<8;
	//printf("bmp_width=%d\r\n",bmp_width);
	
	/* 高度  */
	bmp_height =buf[22];
	bmp_height|=buf[23]<<8;
	//printf("bmp_height=%d\r\n",bmp_height);	
	
	/* 文件类型 */
	bmp_type =buf[28];
	bmp_type|=buf[29]<<8;
	//printf("bmp_type=%d\r\n",bmp_type);	

	/* 设置显示x、y坐标结束位置 */
	x_e = x + bmp_width;
	y_e = y + bmp_height;
	
	
	pbmp_buf = &bmp_buf[54];
	
	for(;y < y_e; y++)
	{
		for (;x < x_e; x++)
		{
			/* 获取红绿蓝颜色数据 */
			blue  = *pbmp_buf++;
			green = *pbmp_buf++;
			red   = *pbmp_buf++;
			
			/* 判断当前的位图是否32位颜色 */
			if(bmp_type == 32)
			{
				pbmp_buf++;
			}
			
			/* 组成24bit颜色 */
			color = red << 16 | green << 8 | blue << 0;
			lcd_draw_point(x, y, color);				
		}
		
		x = x_s;
	}			

	
	return 0;
}

//LCD任意地址绘制图片
int lcd_draw_bmp_file(unsigned int x,unsigned int y,const char *pbmp_path)   
{
			 int bmp_fd;
	unsigned int blue, green, red;
	unsigned int color;
	unsigned int bmp_width;
	unsigned int bmp_height;
	unsigned int bmp_type;
	unsigned int bmp_size;
	unsigned int x_s = x;
	unsigned int y_s = y;
	unsigned int x_e ;	
	unsigned int y_e ;
	unsigned char buf[54]={0};
	unsigned char *pbmp_buf=g_color_buf;
	unsigned char *tmp_buf=NULL;
	
	/* 申请位图资源，权限可读可写 */	
	bmp_fd=open(pbmp_path,O_RDWR);
	
	if(bmp_fd == -1)
	{
	   printf("open bmp error\r\n");
	   
	   return -1;	
	}
	
	/* 读取位图头部信息 */
	read(bmp_fd,buf,54);
	
	/* 宽度  */
	bmp_width =buf[18];
	bmp_width|=buf[19]<<8;
	//printf("bmp_width=%d\r\n",bmp_width);
	
	/* 高度  */
	bmp_height =buf[22];
	bmp_height|=buf[23]<<8;
	//printf("bmp_height=%d\r\n",bmp_height);	
	
	/* 文件类型 */
	bmp_type =buf[28];
	bmp_type|=buf[29]<<8;
	//printf("bmp_type=%d\r\n",bmp_type);	

	/* 设置显示x、y坐标结束位置 */
	x_e = x + bmp_width;
	y_e = y + bmp_height;
	
	/* 获取位图文件的大小 */
	bmp_size=file_size_get(pbmp_path);
	
	/* 读取所有RGB数据 */
	read(bmp_fd,pbmp_buf,bmp_size-54);
	
	//申请内存空间用于存储将Y轴镜像翻转的位图数据
	tmp_buf=(char *)calloc(1,bmp_size);
	
	//将图片翻转,关键将Y轴镜像翻转，同时每个像素点占用3个字节
	for(y=0;y<bmp_height;y++)
	{
		for(x=0;x<bmp_width;x++)
		{
			*(tmp_buf+(bmp_width*(bmp_height-1-y)+x)*3)		=*(pbmp_buf+(bmp_width*y+x)*3);
			*(tmp_buf+(bmp_width*(bmp_height-1-y)+x)*3+1) 	=*(pbmp_buf+(bmp_width*y+x)*3+1);
			*(tmp_buf+(bmp_width*(bmp_height)+x)*3+2) 		=*(pbmp_buf+(bmp_width)*3+2);
		}
	}	
	
	x=x_s;
	y=y_s;
	
	for(;y < y_e; y++)
	{
		for (;x < x_e; x++)
		{
			/* 获取红绿蓝颜色数据 */
			blue  = *pbmp_buf++;
			green = *pbmp_buf++;
			red   = *pbmp_buf++;
			
			/* 判断当前的位图是否32位颜色 */
			if(bmp_type == 32)
			{
				pbmp_buf++;
			}
			
			/* 组成24bit颜色 */
			color = red << 16 | green << 8 | blue << 0;
			lcd_draw_point(x, y, color);				
		}
		
		x = x_s;
	}
	
	//释放内存
	free(tmp_buf);
	
	/* 不再使用BMP，则释放bmp资源 */
	close(bmp_fd);	
	
	return 0;
}


//jpg数据
int lcd_draw_jpg(unsigned int x,unsigned int y,char *pjpg_buf,unsigned int jpg_buf_size)  
{
	/*定义解码对象，错误处理对象*/
	struct 	jpeg_decompress_struct 	cinfo;
	struct 	jpeg_error_mgr 			jerr;	
	
	unsigned char 	*pcolor_buf = g_color_buf;
	char 	*pjpg;
	
	unsigned int 	i=0;
	unsigned int	color =0;
	unsigned int	count =0;
	
	unsigned int 	x_s = x;
	unsigned int 	x_e ;	
	unsigned int 	y_e ;
	
	unsigned int 	jpg_size;
	
	unsigned int 	jpg_width;
	unsigned int 	jpg_height;
	


	jpg_size = jpg_buf_size;
		
	pjpg = pjpg_buf;


	/*注册出错处理*/
	cinfo.err = jpeg_std_error(&jerr);

	/*创建解码*/
	jpeg_create_decompress(&cinfo);

	/*直接解码内存数据*/		
	jpeg_mem_src(&cinfo,pjpg,jpg_size);
	
	/*读文件头*/
	jpeg_read_header(&cinfo, TRUE);

	/*开始解码*/
	jpeg_start_decompress(&cinfo);	
	
	x_e	= x_s+cinfo.output_width;
	y_e	= y  +cinfo.output_height;	

	/*读解码数据*/
	while(cinfo.output_scanline < cinfo.output_height )
	{		
		pcolor_buf = g_color_buf;
		
		/* 读取jpg一行的rgb值 */
		jpeg_read_scanlines(&cinfo,(JSAMPARRAY)&pcolor_buf,1);
		
		for(i=0; i<cinfo.output_width; i++)
		{
			/* 获取rgb值 */
			color = 		*(pcolor_buf+2);
			color = color | *(pcolor_buf+1)<<8;
			color = color | *(pcolor_buf)<<16;
			
			/* 显示像素点 */
			lcd_draw_point(x,y,color);
			
			pcolor_buf +=3;
			
			x++;
		}
		
		/* 换行 */
		y++;			
		
		x = x_s;	
	}		
				
	/* 解码完成 */
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	
	return 0;
}


int lcd_draw_jpg_file(unsigned int x,unsigned int y,const char *pjpg_path)  
{
	/*定义解码对象，错误处理对象*/
	struct 	jpeg_decompress_struct 	cinfo;
	struct 	jpeg_error_mgr 			jerr;	
	
	unsigned char 	*pcolor_buf = g_color_buf;
	char 			*pjpg;
	
	unsigned int 	i=0;
	unsigned int	color =0;
	unsigned int	count =0;
	
	unsigned int 	x_s = x;
	unsigned int 	x_e ;	
	unsigned int 	y_e ;
	
	FILE 			*pjpg_file;
	
			 int	jpg_fd;
	unsigned int 	jpg_size;
	
	unsigned int 	jpg_width;
	unsigned int 	jpg_height;
	

	if(pjpg_path!=NULL)
	{
		if ((pjpg_file = fopen(pjpg_path, "rb")) == NULL) 
		{
			printf("can't open %s\n", pjpg_path);
			return -1;
		}
	}
	else
	{
		printf("jpg path is null\n");
		return -1;
	}

	/*注册出错处理*/
	cinfo.err = jpeg_std_error(&jerr);

	/*创建解码*/
	jpeg_create_decompress(&cinfo);

	/*指定解码数据来源*/		
	jpeg_stdio_src(&cinfo, pjpg_file);
	
	/*读文件头信息*/
	jpeg_read_header(&cinfo, TRUE);

	/*开始解码*/
	jpeg_start_decompress(&cinfo);	

	/*读解码数据，进行逐行读取*/
	while(cinfo.output_scanline < cinfo.output_height )
	{		
		pcolor_buf = g_color_buf;
		
		/* 每次读取jpg一行的rgb值 */
		jpeg_read_scanlines(&cinfo,&pcolor_buf,1);
		
		for(i=0; i<cinfo.output_width; i++)
		{
			/* 获取rgb值 */
			color = 		*(pcolor_buf+2);
			color = color | *(pcolor_buf+1)<<8;
			color = color | *(pcolor_buf)<<16;
			
			/* 显示像素点 */
			lcd_draw_point(x,y,color);
			
			pcolor_buf +=3;
			
			x++;
		}
		
		/* 换行 */
		y++;			
		
		x = x_s;
	}		
			
	/*解码完成*/
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);


	/* 关闭jpg文件 */
	fclose(pjpg_file);

	return 0;	
}


//LCD显示摄像头转换后的rgb数据
int lcd_draw_rgb(unsigned int x,unsigned int y,unsigned int rgb_width,unsigned int rgb_height,unsigned char *rgb_buf)   
{
	unsigned int blue, green, red;
	unsigned int color;
	unsigned int x_s = x;
	unsigned int x_e ;	
	unsigned int y_e ;	
	unsigned char i=0;
	unsigned char *prgb_buf = rgb_buf;
	
	x_e = x + rgb_width;
	y_e = y + rgb_height;	
	
	for(;y < y_e; y++)
	{
		for (;x < x_e; x++)
		{
				/* 获取红绿蓝颜色数据 */
				color = 		*(prgb_buf+2);
				color = color | *(prgb_buf+1)<<8;
				color = color | *(prgb_buf)<<16;
				
				lcd_draw_point(x, y, color);

				prgb_buf+=3;
		}
		
		x = x_s;
	}			
	
	return 0;
}

//显示摄像头数据
int lcd_draw_camera(unsigned int x,unsigned int y,FrameBuffer framebuf_t)  
{
	int camera_support_format=-1;
	int rt=0;
	unsigned int width,height;
	
	//获取摄像头支持的格式
	camera_support_format=linux_v4l2_get_format();
	
	if(camera_support_format < 0)
		return -1;
	
	//获取摄像头当前支持的分辨率
	rt=linux_v4l2_get_resolution(&width,&height);
	
	if(rt < 0)
		return -1;
	
	//根据摄像头支持的格式，显示到LCD屏
	if(camera_support_format == V4L2_PIX_FMT_YUYV)		
		lcd_draw_rgb(x,y,width,height,framebuf_t.buf);
	
	if(camera_support_format == V4L2_PIX_FMT_JPEG)		
		lcd_draw_jpg(x,y,framebuf_t.buf,framebuf_t.length);	
	
	return 0;
}



//LCD关闭
void lcd_close(void)
{
	/* 取消内存映射 */
	munmap(g_pfb_memory, FB_SIZE);
	
	/* 关闭LCD设备 */
	close(g_fb_fd);
}

//缩略图显示
int lcd_draw_jpg_file_ex(unsigned int x,unsigned int y,const char *pjpg_path)  
{
	/*定义解码对象，错误处理对象*/
	struct 	jpeg_decompress_struct 	cinfo;
	struct 	jpeg_error_mgr 			jerr;	
	
	unsigned char 	*pcolor_buf = g_color_buf;
	char 			*pjpg;
	
	unsigned int 	i=0;
	unsigned int	color =0;
	unsigned int	count =0;
	
	unsigned int 	x_s = x;
	unsigned int 	x_e ;	
	unsigned int 	y_e ;
	
	FILE 			*pjpg_file;
	
			 int	jpg_fd;
	unsigned int 	jpg_size;
	
	unsigned int 	jpg_width;
	unsigned int 	jpg_height;
	

	if(pjpg_path!=NULL)
	{
		if ((pjpg_file = fopen(pjpg_path, "rb")) == NULL) 
		{
			printf("can't open %s\n", pjpg_path);
			return -1;
		}
	}
	else
	{
		printf("jpg path is null\n");
		return -1;
	}

	/*注册出错处理*/
	cinfo.err = jpeg_std_error(&jerr);

	/*创建解码*/
	jpeg_create_decompress(&cinfo);

	/*指定解码数据来源*/		
	jpeg_stdio_src(&cinfo, pjpg_file);
	
	/*读文件头信息*/
	jpeg_read_header(&cinfo, TRUE);
	
	//1/2大小输出显示，支持1/1、1/4、1/8输出显示。
	cinfo.scale_num=1;
	cinfo.scale_denom=8;

	/*开始解码*/
	jpeg_start_decompress(&cinfo);	

	/*读解码数据，进行逐行读取*/
	while(cinfo.output_scanline < cinfo.output_height )
	{		
		pcolor_buf = g_color_buf;
		
		/* 每次读取jpg一行的rgb值 */
		jpeg_read_scanlines(&cinfo,&pcolor_buf,1);
		
		for(i=0; i<cinfo.output_width; i++)
		{
			/* 获取rgb值 */
			color = 		*(pcolor_buf+2);
			color = color | *(pcolor_buf+1)<<8;
			color = color | *(pcolor_buf)<<16;
			
			/* 显示像素点 */
			lcd_draw_point(x,y,color);
			
			pcolor_buf +=3;
			
			x++;
		}
		
		/* 换行 */
		y++;			
		
		x = x_s;
	}		
			
	/*解码完成*/
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);


	/* 关闭jpg文件 */
	fclose(pjpg_file);

	return 0;	
}

