#ifndef _FFMPEG_HACK_H
#define _FFMPEG_HACK_H

void runHackThread(void);

void setHackCallback(void (*callback)(char **toSend));

#endif