#ifndef __LCD_H__
#define __LCD_H__

#define LCD_WIDTH  			800
#define LCD_HEIGHT 			480
#define FB_SIZE				(LCD_WIDTH * LCD_HEIGHT * 4)

extern int 	lcd_open(void);
extern void lcd_draw_point(unsigned int x,unsigned int y, unsigned int color);
extern int 	lcd_draw_jpg_file(unsigned int x,unsigned int y,const char *pjpg_path); 
extern void lcd_close(void);
extern int  lcd_draw_rgb(unsigned int x,unsigned int y,unsigned int rgb_width,unsigned int rgb_height,unsigned char *rgb_buf);
extern int 	lcd_draw_jpg(unsigned int x,unsigned int y,char *pjpg_buf,unsigned int jpg_buf_size);  
extern int  lcd_draw_camera(unsigned int x,unsigned int y,FrameBuffer framebuf_t); 
extern int 	lcd_draw_jpg_file_ex(unsigned int x,unsigned int y,const char *pjpg_path) ;

#endif