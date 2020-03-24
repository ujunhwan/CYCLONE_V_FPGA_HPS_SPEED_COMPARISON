#ifndef __AUDIO_H__
#define __AUDIO_H__

#define BUF_SIZE 80000 
#define BUF_THRESHOLD 96
#define LW_BRIDGE_SPAN 0x00005000
#define LW_BRIDGE_BASE 0xFF200000
#define SDRAM_SPAN 0x03FFFFFFF
#define DMA_M2S 0x00003080
#define DMA_S2M	0x00003060

int open_physical(int);
void* map_physical(int, unsigned int, unsigned int);
void close_physical(int);
int unmap_physical(void*, unsigned int);
void check_KEYs(int *, int *,int *, int *, int * );
void recordInit();
int recordAction();
void playInit();
int playAction();

#endif