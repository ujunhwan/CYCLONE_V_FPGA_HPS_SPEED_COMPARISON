#include "address_map_arm.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>

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

int main(void) 
{
    volatile int* red_LED_ptr;
    volatile int* audio_ptr;
    volatile int* SDRAM_ptr;
    volatile int* HEX3_HEX0_ptr;
    volatile int* HEX5_HEX4_ptr;

    volatile int* sdram2module_ptr;
	volatile int* sdram2module_status_ptr;
	volatile int* sdram2module_readaddr_ptr;
	volatile int* sdram2module_writeaddr_ptr;
	volatile int* sdram2module_leng_ptr;
	volatile int* sdram2module_control_ptr;
	time_t start,end;

	volatile int* module2sdram_ptr;
	volatile int* module2sdram_status_ptr;
	volatile int* module2sdram_writeaddr_ptr;
	volatile int* module2sdram_readaddr_ptr;
	volatile int* module2sdram_leng_ptr;
	volatile int* module2sdram_control_ptr;
	volatile int* SDRAM_result_ptr;
    volatile int* SW_ptr;

	int fd = -1, i;
    FILE* sp;
	void* LW_virtual;
    void* SDRAM_virtual;
	long double duration;

    int fifospace;
	int record = 0, play = 0, buffer_index = 0;
	int sdram = 0, result = 0;
    int left_buffer[BUF_SIZE];
    int right_buffer[BUF_SIZE];
    int software_case[BUF_SIZE];
    int software_compression = 0;
    int software_origin [BUF_SIZE];
	unsigned short data_buffer[2 * BUF_SIZE];

    record = 0;
    play   = 0;
    sdram = 0;

	if ((fd = open_physical(fd)) == -1)
		return (-1);
	if ((LW_virtual = map_physical(fd, LW_BRIDGE_BASE, LW_BRIDGE_SPAN)) == NULL)
		return (-1);
    if ((SDRAM_virtual = map_physical (fd, SDRAM_BASE, SDRAM_SPAN)) == NULL)
        return (-1);

	red_LED_ptr = (unsigned int*)(LW_virtual + LED_BASE);
	audio_ptr = (unsigned int*)(LW_virtual + AUDIO_BASE);
    HEX3_HEX0_ptr = (unsigned int*)(LW_virtual + HEX3_HEX0_BASE);
    HEX5_HEX4_ptr = (unsigned int*)(LW_virtual + HEX5_HEX4_BASE);
	SW_ptr = (unsigned int*)(LW_virtual + SW_BASE);
	SDRAM_ptr = (unsigned int*)(SDRAM_virtual);
	SDRAM_result_ptr = (unsigned int*)(SDRAM_virtual + 0x02000000);	//separated SDRAM for result

	sdram2module_ptr = (unsigned int*)(LW_virtual + DMA_S2M);
	sdram2module_status_ptr = sdram2module_ptr;
	sdram2module_readaddr_ptr = sdram2module_ptr + 1;
	sdram2module_writeaddr_ptr = sdram2module_ptr +2;
	sdram2module_leng_ptr = sdram2module_ptr + 3;
	sdram2module_control_ptr = sdram2module_ptr + 6;

	module2sdram_ptr = (unsigned int*)(LW_virtual + DMA_M2S);
	module2sdram_status_ptr = module2sdram_ptr;
	module2sdram_readaddr_ptr = module2sdram_ptr + 1;
	module2sdram_writeaddr_ptr = module2sdram_ptr + 2;
	module2sdram_leng_ptr = module2sdram_ptr + 3;
	module2sdram_control_ptr = module2sdram_ptr + 6;

    printf("READY\n");
    while (1) 
    {
        check_KEYs(&record, &play,&sdram, &result, &buffer_index);
        if (record) 
        {
            *(HEX3_HEX0_ptr) = 0x0;
            *(HEX5_HEX4_ptr) = 0x0;
            *(HEX3_HEX0_ptr) = 0x393F315E;
            *(HEX5_HEX4_ptr) = 0x3179;

            *(red_LED_ptr) = 0x1;
			fifospace = *(audio_ptr + 1);
            if ((fifospace & 0x000000FF) > BUF_THRESHOLD) 
            {
                while ((fifospace & 0x000000FF) && (buffer_index < BUF_SIZE))  
                {
                    left_buffer[buffer_index]  = *(audio_ptr + 2);
                    right_buffer[buffer_index] = *(audio_ptr + 3);
                    ++buffer_index;
                    if (buffer_index == BUF_SIZE) 
                    {
                        *(HEX3_HEX0_ptr) = 0x0;
                        *(HEX5_HEX4_ptr) = 0x0;
                        *(red_LED_ptr) = 0x0;
                        printf("Recording Complete\n");
						record = 0;
                    }
					fifospace = *(audio_ptr + 1);
                }
            }
        } 
        else if (play) 
        {
            *(HEX3_HEX0_ptr) = 0x0;
            *(HEX5_HEX4_ptr) = 0x0;
			*(HEX3_HEX0_ptr) = 0x7338776E;
            *(red_LED_ptr) = 0x2;
            fifospace = *(audio_ptr + 1);
            if ((fifospace & 0x00FF0000) > BUF_THRESHOLD) 
            {
                while ((fifospace & 0x00FF0000) && (buffer_index < BUF_SIZE)) 
                {
                    *(audio_ptr + 2) = left_buffer[buffer_index];
                    *(audio_ptr + 3) = right_buffer[buffer_index];
                    ++buffer_index;
                    if (buffer_index == BUF_SIZE) 
                    {
                        play = 0;
                        *(HEX3_HEX0_ptr) = 0x0;
                        *(HEX5_HEX4_ptr) = 0x0;
                        *(red_LED_ptr) = 0x0;
                        printf("Playing Complete\n");
                    }
                    fifospace = *(audio_ptr + 1);
                }
            }
        }
       else if (sdram)
        {
            printf("*******************\n");
            printf(" SDRAM state \n");
            printf("*******************\n");
			while ((buffer_index < BUF_SIZE)) 
            {
                *(SDRAM_ptr + buffer_index) =  left_buffer[buffer_index];
				++buffer_index;
				if (buffer_index == BUF_SIZE) 
                {
					*(HEX3_HEX0_ptr) = 0x0;
					*(HEX5_HEX4_ptr) = 0x0;
					*(red_LED_ptr) = 0x0; 
					printf("Transfering Complete\n");
					printf("Module to SDRAM Complete\n");
					printf("Clear Status Register\n");
					*(module2sdram_status_ptr) =*(module2sdram_status_ptr) & 0xFFFFFFFE;
					*(sdram2module_status_ptr) = *(sdram2module_status_ptr) & 0xFFFFFFFE;

                    printf("Clear Status Register\n");
                    printf("SDRAM to Module Status : %X \n", *(sdram2module_status_ptr));
                    printf("Module to SDRAM Status : %X \n", *(module2sdram_status_ptr));
                    *(sdram2module_readaddr_ptr) = 0x00000000;
                    *(module2sdram_readaddr_ptr) = 0x00000000;
                    *(sdram2module_writeaddr_ptr) = 0x00000000;
                    *(module2sdram_writeaddr_ptr) = 0x02000000;
					 
                    *(sdram2module_leng_ptr) = 320000;
                    *(module2sdram_leng_ptr) = 320000;
                    *(sdram2module_control_ptr) = 0x28C;
                    *(module2sdram_control_ptr) = 0x18C;
                    printf("SDRAM to Module Start\n");
                    start=clock();
                    while ((*(sdram2module_status_ptr) & 0x1) != 0x1);			
                    printf("SDRAM to Module Complete\n");
                    printf("Module to SDRAM Start\n");
                    while ((*(module2sdram_status_ptr) & 0x1) != 0x1);
					end=clock();
					printf("Module to SDRAM Complete\n");
                    printf("**********************\n");
					duration = (long double)(end-start)/(CLOCKS_PER_SEC);
					printf("Time Elapsed by Hardware : %0f ms \n", duration*1000);	
                    printf("**********************\n");
                    printf("SDRAM to Module Status : %d \n", *(sdram2module_status_ptr));
                    printf("Module to SDRAM Status : %d \n", *(module2sdram_status_ptr));
                    sdram = 0;
				}
			}
        }
        else if (result) 
        {
            start=clock();
            for (i=0 ; i<BUF_SIZE ;i++)
                software_origin[i] = *(SDRAM_ptr + i);
            while((buffer_index < BUF_SIZE))
            {
                if(((left_buffer[buffer_index] >> 31) &0x1) == 0x1)
                    software_case[buffer_index] = 0xF000 + ((software_origin[buffer_index] & (0xFFF00000 >> 1)) >> 19);
                else if(((left_buffer[buffer_index] >> 30) & 0x1) == 0x1)
                    software_case[buffer_index] = 0xE000 + ((software_origin[buffer_index] & (0xFFF00000 >> 2)) >> 18);
                else if(((left_buffer[buffer_index] >> 29) & 0x1) == 0x1)
                    software_case[buffer_index] = 0xD000 + ((software_origin[buffer_index] & (0xFFF00000 >> 3)) >> 17);
                else if(((left_buffer[buffer_index] >> 28) & 0x1) == 0x1)
                    software_case[buffer_index] = (0xC000 + ((software_origin[buffer_index] & (0xFFF00000 >> 4)) >> 16));
                else if(((left_buffer[buffer_index] >> 27) & 0x1) == 0x1)
                    software_case[buffer_index] = (0xB000 + ((software_origin[buffer_index] & (0xFFF00000 >> 5)) >> 15));
                else if(((left_buffer[buffer_index] >> 26) & 0x1) == 0x1)
                    software_case[buffer_index] = (0xA000 + ((software_origin[buffer_index] & (0xFFF00000 >> 6)) >> 14));
                else if(((left_buffer[buffer_index] >> 25) & 0x1) == 0x1)
                    software_case[buffer_index] = (0x9000 + ((software_origin[buffer_index] & (0xFFF00000 >> 7)) >> 13));
                else if(((left_buffer[buffer_index] >> 24) & 0x1) == 0x1)
                    software_case[buffer_index] = (0x8000 + ((software_origin[buffer_index] & (0xFFF00000 >> 8)) >> 12));
                else if(((left_buffer[buffer_index] >> 23) & 0x1) == 0x1)
                    software_case[buffer_index] = (0x7000 + ((software_origin[buffer_index] & (0xFFF00000 >> 9)) >> 11));
                else if(((left_buffer[buffer_index] >> 22) & 0x1) == 0x1)
                    software_case[buffer_index] = (0x6000 + ((software_origin[buffer_index] & (0xFFF00000 >> 10)) >> 10));
                else if(((left_buffer[buffer_index] >> 21) & 0x1) == 0x1)
                    software_case[buffer_index] = (0x5000 + ((software_origin[buffer_index] & (0xFFF00000 >> 11)) >> 9));
                else if(((left_buffer[buffer_index] >> 20) & 0x1) == 0x1)
                    software_case[buffer_index] = (0x4000 + ((software_origin[buffer_index] & (0xFFF00000 >> 12)) >> 8));
                else if(((left_buffer[buffer_index] >> 19) & 0x1) == 0x1)
                    software_case[buffer_index] = (0x3000 + ((software_origin[buffer_index] & (0xFFF00000 >> 13)) >> 7));
                else if(((left_buffer[buffer_index] >> 18) & 0x1) == 0x1)
                    software_case[buffer_index] = (0x2000 + ((software_origin[buffer_index] & (0xFFF00000 >> 14)) >> 6));
                else if(((left_buffer[buffer_index] >> 17) & 0x1) == 0x1)
                    software_case[buffer_index] = (0x1000 + ((software_origin[buffer_index] & (0xFFF00000 >> 15)) >> 5));
                else if(((left_buffer[buffer_index] >> 16) & 0x1) == 0x1)
                    software_case[buffer_index] = (0x0000 + ((software_origin[buffer_index] & (0xFFF00000 >> 16)) >> 4));
                else 
                    software_case[buffer_index] = (0x0000 + ((software_origin[buffer_index] & (0xFFF00000 >> 17)) >> 3));
                ++buffer_index;
            }
            for (i=0; i<BUF_SIZE; i++)
                *(SDRAM_result_ptr + i) = software_case[i];
            end=clock();
            printf("**********************\n");
            duration = (long double)(end-start)/(CLOCKS_PER_SEC);
            printf("Time Elapsed by Software : %0f ms \n", duration*1000);
            printf("**********************\n");
            if(buffer_index == BUF_SIZE)
            {
                sp = fopen("result.txt", "wt");
                if(sp == NULL)
                {
                    printf("Can't open txt file \n");
                    exit(1);
                }
                for(i=0; i<BUF_SIZE; i++)
                    fprintf(sp, "Origin data : %X, | Hardware Compression Result : %X, | Software Compression Result : %X \n", left_buffer[i], *(SDRAM_result_ptr + i), software_case[i]);
                fclose(sp);
                printf("Write text file Complete\n");
            }
            result = 0;
        }
    }
    unmap_physical(LW_virtual, LW_BRIDGE_SPAN);
    close_physical(fd);
}

int open_physical(int fd)
{
    if (fd == -1)
	if((fd = open("/dev/mem", (O_RDWR | O_SYNC))) == -1)
    {
        printf("ERROR: could not open \"/dev/mem\"...\n");
        return (-1);
    }
    return fd;
}
void close_physical(int fd)
{
    close(fd);
}
void* map_physical(int fd, unsigned int base, unsigned int span)
{
    void *virtual_base;
    virtual_base = mmap(NULL, span, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, base);
    if (virtual_base == MAP_FAILED)
    {
        printf("ERROR: mmap() failed...\n");
        close(fd);
        return (NULL);
    }
    return virtual_base;
}
int unmap_physical(void* virtual_base, unsigned int span)
{
    if (munmap(virtual_base, span) != 0)
    {
        printf("ERROR: munmap() failed...\n");
        return (-1);
    }
    return 0;
}
void check_KEYs(int * KEY0, int * KEY1, int * KEY2,int * KEY3, int * counter) 
{
    volatile int * KEY_ptr;
    volatile int * audio_ptr;
    int KEY_value;
	int fd = -1;
	void* LW_KEY_virtual;

    if ((fd = open_physical(fd)) == -1)
        return (-1);
	if ((LW_KEY_virtual = map_physical(fd, LW_BRIDGE_BASE, LW_BRIDGE_SPAN)) == NULL)
		return (-1);

	KEY_ptr = (unsigned int*)(LW_KEY_virtual + KEY_BASE);
	audio_ptr = (unsigned int*)(LW_KEY_virtual + AUDIO_BASE);
	
    KEY_value = *(KEY_ptr);
    while (*KEY_ptr);
    if (KEY_value == 0x1) 
    {
        *counter = 0;
        *(audio_ptr) = 0x4;
        *(audio_ptr) = 0x0;
        *KEY0 = 1;
    }
    else if (KEY_value == 0x2)
    {
        *counter = 0;
        *(audio_ptr) = 0x8;
        *(audio_ptr) = 0x0;
        *KEY1 = 1;
    } 
    else if(KEY_value == 0x4)
    {
        *counter = 0;
        *KEY2 = 1;
	}
	else if (KEY_value == 0x8) 
    {
		*counter = 0;
		*KEY3 = 1;
    }
	unmap_physical(LW_KEY_virtual, LW_BRIDGE_SPAN);
	close_physical(fd);
}