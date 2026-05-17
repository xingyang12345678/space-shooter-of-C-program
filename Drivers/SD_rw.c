#include "SD_rw.h"
FATFS  fs;
FIL file;
UINT bw, br;

void SD_Init(void)
{
	 f_mount(&fs, "0:", 1);
}

void SD_Write(int32_t value)
{
	char buf[20];
	if(f_open(&file, "0:value.txt", FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)
    {
        sprintf(buf, "%d", value);
        f_write(&file, buf, strlen(buf), &bw);
        f_close(&file);
    }
}

int32_t SD_Read(void)
{
	char buf[20];
  int32_t value = 0;
	if(f_open(&file, "0:value.txt", FA_READ) == FR_OK)
    {
        f_read(&file, buf, sizeof(buf), &br);
        buf[br] = 0;
        value = atoi(buf);
        f_close(&file);
    }
    return value;
}
