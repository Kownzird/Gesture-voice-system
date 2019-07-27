#ifndef __VIDEOPLAY_H
#define __VIDEOPLAY_H

extern int mute;
extern int voice;

void video_play();
void send_cmd(char *cmd);
void mplayer_init(void);
void video_quit(void);
void video_mute(void);
void video_pause(void);
void video_voice_up(void);
void video_voice_down(void);
void video_backward(void);
void video_forward(void);


#endif
