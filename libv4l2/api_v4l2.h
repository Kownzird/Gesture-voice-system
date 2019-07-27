#ifndef __API_V4L2_H__
#define __API_V4L2_H__

//add by stephen.wen
typedef unsigned short  WORD;       //定义BMP文件头的数据类型
typedef unsigned int  	DWORD;
 
#pragma pack(2)                     //修改结构体字节对齐规则
typedef struct _BITMAPFILEHEADER{
     WORD     bfType;                //BMP文件头，必须为BM
     DWORD    bfSize;                //文件大小，字节为单位
     DWORD    bfReserved;            //保留字，值为0
     DWORD    bfOffBits;             //位图数据起始位置，此处取54
}BITMAPFILEHEADER;
typedef struct _BITMAPINFOHEADER{
     DWORD    biSize;               //本结构所占字节数，此处取40
     DWORD    biWidth;              //图片宽度，像素为单位
     DWORD    biHeight;             //高度，同上
     WORD     biPlanes;             //目标设备级别，此处必须为1
     WORD     biBitCount;           //像素位数，此处取24（真彩色）
     DWORD    biCompression;        //位图类型，必须为0
     DWORD    biSizeImage;          //位图大小
     DWORD    biXPelsPerMeter;      //水平像素数，此处取0
     DWORD    biYPelsPerMeter;      //竖直像素数，此处取0
     DWORD    biClrUsed;            //位图实际使用的颜色表中的颜色数，此处取0
     DWORD    biClrImportant;       //位图显示过程中重要的颜色数，此处取0
}BITMAPINFOHEADER;
#pragma pack()


#define VIDEO_WIDTH  		640
#define VIDEO_HEIGHT 		480
#define MAX_CAM_RES 		32 		//camres res
#define BUFFER_COUNT 		4 		//buffer zone
#define FPS 				30
#define MAX_CAM_RES 		32

//add by stephen.wen
#define YUV_TO_NONE			0
#define YUV_TO_RGB			1
#define YUV_TO_BMP			2
#define YUV_TO_JPG			3


//add by stephen.wen
#define SORT_IS_RGB			0
#define SORT_IS_BGR			1

#define CAM_FMT_YUV			0
#define CAM_FMT_JPG			1
#define CAM_FMT_RGB			2
#define CAM_FMT_UNKNOWN		-1

#define exit_error(s)\
	do{\
		printf("%s is error\n",s);\
		return (-1);\
	}while(0)


/*********************************
*  NAME:VideoBuffer  struct
*  Function: Describe buffer V4L2 driver assigns and maps
*  Member:  start: point of buffer
*  length: total length of buffer
**********************************/
typedef struct Video_Buffer
{
	void 	*start;
	int 	length;
}VideoBuffer;
/*******************************
*  nan=me :Frame_Buffer
*  Function: save fream
*  member: buf :point of buf
*  length:total length op buf
********************************/
typedef struct Frame_Buffer
{
	char buf[VIDEO_WIDTH*VIDEO_HEIGHT*4];
	int length;
	
}FrameBuffer;
/************************************
*		name:CamRes
*		Function:CamRes format
*		Member: width : format width
*						height: format height 
*/
typedef struct CamRes 
{
	int width;
	int height;
}CamRes;
/*************************************
*		name:CamResList struct
*		Function:CamRes  format list
*		Member:  cam_res :struct CamRes
						res_num:camres format number		
*/
typedef struct CamResList
{
	struct CamRes *cam_res;
	int res_num;
}CamResList;

/**********************************
*
*
***********************************/

/*************************************
*		Name:open_device Function
*		Function:open the device
*/
void linux_v4l2_open_device(const char *dev_path);

/*************************************
*		name:device_init
*		Function:Initial Camera v4l2 
*/
int linux_v4l2_device_init(const char *dev_path,unsigned int camera_width,unsigned int camera_height);

/*************************************
*		name: init_mmap
*		Function: mmap 
*/
int linux_v4l2_init_mmap();

/************************************
*		name:start_capturing
*		Function: starting Capture Options 
*/
int linux_v4l2_start_capturing();

/************************************
*		name:stop_capturing
*		Function: stop Capture Options
*/
int linux_v4l2_stop_capturing();

/************************************
*		name :device_uinit
*		Function:uinit device
*/
int linux_v4l2_device_uinit();
/************************************
*		name :linux_v4l2_get_frame
*		Function:save device_stream data;
*/
int linux_v4l2_get_frame(FrameBuffer *framebuf_t);

/************************************
*		name :linux_v4l2_get_format
*		Function:camera support format
*/
int linux_v4l2_get_format(void);

/************************************
*		name :linux_v4l2_get_resolution
*		Function:camera resolution
*/
int linux_v4l2_get_resolution(unsigned int *width,unsigned int *height);
/************************************
*		name :linux_v4l2_save_image_file
*		Function:camera data to jpg file
*/
int linux_v4l2_save_image_file(const char *file_path,FrameBuffer framebuf_t);
/************************************
*		name :rgb_to_jpg_file
*		Function:rgb data to jpg file
*/
long rgb_to_jpg_file(const char *jpg_path,unsigned char *rgb);
/************************************
*		name :rgb_to_bmp_file
*		Function:rgb data to bmp file
*/
long rgb_to_bmp_file(const char *bmp_path,unsigned char *rgb);


#endif /*API_V4L2_H*/
