#include <stdio.h>
#include <sys/types.h>         
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "libxml/xmlmemory.h"
#include "libxml/parser.h"
#include "api_v4l2.h"
#include "lcd.h"

extern int ei;

xmlChar *__get_cmd_id(xmlDocPtr doc, xmlNodePtr cur)
{
	xmlChar *key, *id;
	
	
	char buf[16]={0};
	
	cur = cur->xmlChildrenNode;
	
	while (cur != NULL)
	{
		//查找到cmd子节点
		for(ei=1;ei<5;ei++)
		{
			memset(buf,0,sizeof buf);
			sprintf(buf,"cmd%d",ei);
			
			if ((!xmlStrcmp(cur->name,buf)))
			{
				//获取字符串

				key = xmlNodeGetContent(cur);
			
				printf("cmd%d: %s\n",ei,key);
				xmlFree(key);

				//读取节点属性
				id = xmlGetProp(cur, (const xmlChar *)"id");
				printf("id: %s\n", id);

				xmlFree(doc);
				return id;
			}
		}
		
		cur = cur->next;
	}
	//释放文档指针
	xmlFree(doc);
    return NULL;
}

xmlChar *parse_xml(char *xmlfile)
{
	xmlDocPtr doc;
	xmlNodePtr cur1, cur2;

	//分析一个xml文件，并返回一个xml文档的对象指针
	doc = xmlParseFile(xmlfile);
	if (doc == NULL)
	{
		fprintf(stderr,"Document not parsed successfully. \n");
		return NULL;
	}
	
	//获得文档的根节点
	cur1 = xmlDocGetRootElement(doc);
	if(cur1 == NULL)
	{
		fprintf(stderr,"empty document\n");
		xmlFreeDoc(doc);
		return NULL;
	}
	//检查根节点的名称是否为nlp
	if(xmlStrcmp(cur1->name, (const xmlChar *)"nlp"))
	{
		fprintf(stderr,"document of the wrong type, root node != nlp");
		xmlFreeDoc(doc);
		return NULL;
	}
	
	//获取子元素节点
	cur1 = cur1->xmlChildrenNode;

	while (cur1 != NULL)
	{
		//检查子元素是否为result
		if ((!xmlStrcmp(cur1->name, (const xmlChar *)"result")))
		{
			//得到result的子节点
			cur2 = cur1->xmlChildrenNode;
			while(cur2 != NULL)
			{
				//查找到准确率
				if((!xmlStrcmp(cur2->name, (const xmlChar *)"confidence")))
				{
#if 0					
					xmlChar *key = xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1);
#else					
					xmlChar *key = xmlNodeGetContent(cur2);
#endif					
					printf("confidence: %s\n",key);
					
					//若准确率低于30，则放弃当前识别
					if(atoi((char *)key) <30)
					{
						xmlFree(doc);
						lcd_draw_jpg_file(0,0,"error.jpg");
						fprintf(stderr, "sorry, I'm NOT sure what you say.\n");
						system("madplay -a -30 error.mp3");
						return NULL;
					}
					
					
				}
				
				//查找到object，则执行提取字符串及属性
				if((!xmlStrcmp(cur2->name, (const xmlChar *)"object")))
				{
					
					return __get_cmd_id(doc, cur2);
				}
				cur2 = cur2->next;
			}
		}
		cur1 = cur1->next;
	}

	//释放文档指针
	xmlFreeDoc(doc);
	return NULL;
}

int  tcp_send_pcm(int socket_fd,const char *pcm_file)
{
	int fd;
	
	//以只读方式打开pcm文件
	fd = open(pcm_file,O_RDONLY);
	
	if(fd < 0)
	{
		printf("open %s\n",pcm_file);
		return -1;
	}
	
	// 取得PCM数据的大小
	off_t pcm_size = lseek(fd, 0, SEEK_END);
	
	// 从新定位到文件的头部
	lseek(fd, 0, SEEK_SET);
	
	// 分配1个pcm_size字节大小的缓冲区
	char *pcm_buf = calloc(1, pcm_size);
	
	// 读取PCM文件数据
	read(fd, pcm_buf, pcm_size);

	// 将PCM文件发送给语音识别引擎系统
	int m = send(socket_fd, pcm_buf, pcm_size,0);
	
	printf("%d bytes has been sent\n", m);

	free(pcm_buf);	
	
	return 0;
}


int tcp_recv_xml(int socket_fd)
{
	
	//struct timeval timeout={3,0};
	//setsockopt(socket_fd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
	//calloc动态分配完内存后，自动初始化该内存空间为零
	//分配1个1024字节大小的缓冲区
	char *xml_buf = calloc(1, 1024);

	// 从ubuntu接收XML结果
	int n = recv(socket_fd, xml_buf, 1024,0);

	if(n <= 0)
	{
		perror("tcp recv:");
		
		return -1;
	}
	
	printf("%d bytes has been recv\n", n);
	
	//创建result.xml文件并清空
	int fd = open("result.xml",O_CREAT|O_RDWR|O_TRUNC);
	
	if(fd < 0)
	{
		printf("create result.xml fail\n");
		
		return -1;
		
	}
	
	//将接收到XML写入到result.xml文件
	n = write(fd,xml_buf,n);
	
	printf("%d bytes has been write to result.xml\n", n);

	//关闭result.xml文件
	close(fd);
	
	return 0;
}

