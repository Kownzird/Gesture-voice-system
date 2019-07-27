#定义变量保存gcc,arm-linux-gcc
CC=arm-none-linux-gnueabi-gcc

ELF=arm-project

#CONFIG=-I./ -I./libjpeg -L./libjpeg -ljpeg   -lpthread 
#CONFIG=-I./ -I./libjpeg -L./libjpeg -ljpeg   -lpthread -mfloat-abi=softfp -mfpu=vfpv3-d16
CONFIG =-I./ -I./inc/ -I./libjpeg/ -L./lib/ -L./libjpeg/ -ljpeg -lxml2 -lz   
CONFIG+=-I./libv4l2 -L./libv4l2 -lv4l2
CONFIG+=-lpthread -mfloat-abi=softfp -mfpu=neon -ftree-vectorize

SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c, %.o, $(SRCS))

$(ELF):$(OBJS)
	$(CC) -o $@ $^ $(CONFIG)
%.o:%.c
	$(CC) -c $< -o $@ $(CONFIG)

clean:
	rm *.o $(TARGET)
