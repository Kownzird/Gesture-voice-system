#ifndef __XML_H
#define __XML_H

extern int ei;

xmlChar *__get_cmd_id(xmlDocPtr doc, xmlNodePtr cur);
xmlChar *parse_xml(char *xmlfile);
int  tcp_send_pcm(int socket_fd,const char *pcm_file);
int tcp_recv_xml(int socket_fd);

#endif
