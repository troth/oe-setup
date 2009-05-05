/*
  DWORD 0-6 reserved
  DWORD 7 x offset, x width
  DWORD 8 y offset, y width
 */
#define CALIB1_OFFSET 7
#define CALIB2_OFFSET 8
  //printf("x calibration: offset=%d, width=%d\n",tagmem[CALIB1_OFFSET] >> 16,tagmem[CALIB1_OFFSET] & 0xFFFF);
  //printf("y calibration: offset=%d, width=%d\n",tagmem[CALIB2_OFFSET] >> 16,tagmem[CALIB2_OFFSET] & 0xFFFF);

int read_tagmem(int *tagmem);
int write_tagmem(int *tagmem);
int erase_tagmem();
