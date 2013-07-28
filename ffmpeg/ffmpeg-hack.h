#ifndef _FFMPEG_HACK_H
#define _FFMPEG_HACK_H

void runHackThread(void *data);

void setHackCallback(void (*callback)(char **toSend, void *data));

#endif