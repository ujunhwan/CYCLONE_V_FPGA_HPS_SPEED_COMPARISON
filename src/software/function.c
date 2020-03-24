#include <sys/mman.h>
#include <stdio.h>
#include "audio.h"
#include "fcntl.h"
#include "address_map_arm.h"

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

int left_buffer[BUF_SIZE];
int right_buffer[BUF_SIZE];

void recordInit()
{
    int fd = -1;
    void * LW_virtual;
    void * SDRAM_virtual;
    if((LW_virtual = map_physical(fd, LW_BRIDGE_BASE, LW_BRIDGE_SPAN)) == NULL)
        return (-1);
    if((SDRAM_virtual = map_physical (fd, SDRAM_BASE, SDRAM_SPAN)) == NULL)
        return (-1);

    volatile int * HEX3_HEX0_ptr = (unsigned int*)(LW_virtual + HEX3_HEX0_BASE);
    volatile int * HEX5_HEX4_ptr = (unsigned int*)(LW_virtual + HEX5_HEX4_BASE);
    volatile int * red_LED_ptr = (unsigned int*)(LW_virtual + LED_BASE);

    *(HEX5_HEX4_ptr) = 0x0;
    *(HEX3_HEX0_ptr) = 0x0;
    *(HEX3_HEX0_ptr) = 0x393F315E;
    *(HEX5_HEX4_ptr) = 0x3179;
    *(red_LED_ptr) = 0x01;
}

int recordAction()
{
    int fd = -1;
    void * LW_virtual;
    if((LW_virtual = map_physical(fd, LW_BRIDGE_BASE, LW_BRIDGE_SPAN)) == NULL)
        return (-1);

    volatile int * audio_ptr = (unsigned int*)(LW_virtual + AUDIO_BASE);
    volatile int * HEX3_HEX0_ptr = (unsigned int*)(LW_virtual + HEX3_HEX0_BASE);
    volatile int * HEX5_HEX4_ptr = (unsigned int*)(LW_virtual + HEX5_HEX4_BASE);
    volatile int * red_LED_ptr = (unsigned int*)(LW_virtual + LED_BASE);

    int fifospace = *(audio_ptr + 1);
    int buffer_index = 0;
    
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
            }
            fifospace = *(audio_ptr + 1);
        }
    }
    return -1;
}
void playInit()
{
    int fd = -1;
    void * LW_virtual;

    if((LW_virtual = map_physical(fd, LW_BRIDGE_BASE, LW_BRIDGE_SPAN)) == NULL)
        return (-1);
    
    volatile int * HEX3_HEX0_ptr = (unsigned int*)(LW_virtual + HEX3_HEX0_BASE);
    volatile int * HEX5_HEX4_ptr = (unsigned int*)(LW_virtual + HEX5_HEX4_BASE);
    volatile int * red_LED_ptr = (unsigned int*)(LW_virtual + LED_BASE);

    *(HEX3_HEX0_ptr) = 0x0;
    *(HEX5_HEX4_ptr) = 0x0;
    *(HEX3_HEX0_ptr) = 0x7338776E;
    *(red_LED_ptr) = 0x2;
}
int playAction()
{
    int fd = -1;
    void * LW_virtual;
    if((LW_virtual = map_physical(fd, LW_BRIDGE_BASE, LW_BRIDGE_SPAN)) == NULL)
        return (-1);

    volatile int * audio_ptr = (unsigned int*)(LW_virtual + AUDIO_BASE);
    volatile int * HEX3_HEX0_ptr = (unsigned int*)(LW_virtual + HEX3_HEX0_BASE);
    volatile int * HEX5_HEX4_ptr = (unsigned int*)(LW_virtual + HEX5_HEX4_BASE);
    volatile int * red_LED_ptr = (unsigned int*)(LW_virtual + LED_BASE);

    int fifospace = *(audio_ptr + 1);
    int buffer_index = 0;

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
                *(HEX3_HEX0_ptr) = 0x0;
                *(HEX5_HEX4_ptr) = 0x0;
                *(red_LED_ptr) = 0x0;
                printf("Playing Complete\n");
            }
            fifospace = *(audio_ptr + 1);
        }
    }
    return -1;
}