#ifndef __GESTURE_H
#define __GESTURE_H

extern int mute;
extern int voice;
extern int quit_camera;
extern int gesture_mode;
extern int gesture_video;
extern int choose;
extern int gesture_value;
extern int gesture(void);

extern char voice_buf[10];
extern char cmd_voice_buf[30];

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


#endif
