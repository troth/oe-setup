#include<unistd.h>
#include <sys/stat.h>
#include<sys/types.h>
#include<sys/mman.h>
#include<sys/time.h>
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include <sys/termios.h>
#include<math.h>
#include <sys/select.h>
#include <linux/input.h>
#include <linux/fb.h>
#include "tagmem.h"


/*

  TS-TEST
  Touchscreen Test Program for TS LCD boards

  by Michael Schmidt
  (c)2008 Technologic Systems

*/

int WIDE=800;
int HIGH=480;
#define PIXTOT (WIDE*HIGH)

//---------------------------------
volatile unsigned short *regs;

unsigned short *fb;
unsigned int yoffsets[800];
volatile unsigned short *ts;
volatile unsigned short *backlight;

void setpixel(int x, int y, const int clr)
{
  register unsigned int p;

  if (x < 0) x = 0; else if (x > WIDE-1) x = WIDE-1;
  if (y < 0) y = 0; else if (y > HIGH-1) y = HIGH-1;
  p=x+yoffsets[y];
  if ((p>=0) && (p<=PIXTOT-1)) {
    if (clr == -1) {
      fb[p] = ~fb[p];
    } else {
      fb[p]= (unsigned short)clr;
    }
  }

}

unsigned short getpixel(const int x, const int y)
{
  register unsigned int p;
  p=x+yoffsets[y];
  if ((p>=0) && (p<=PIXTOT-1)) {
    return fb[p];
  }
  return 0;
}

void clearvideo(unsigned short clr)
{
  register int idx;
  for(idx=0; idx<PIXTOT; idx++) {
       fb[idx]=clr;
  }
}

void invertvideo() {
  register int idx;
  register unsigned short clr;

  for(idx=0; idx<PIXTOT; idx++) {
    clr=fb[idx];
    clr = ~clr;
    fb[idx]=clr;
  }
}

void loadimage(char *filename) {
  int idx=0,j,i,h = open(filename,O_RDONLY);
  unsigned short buf[WIDE];

  if (h < 0) {
    perror(filename);
    return;
  }
  for (i=0;i<HIGH;i++) {
    read(h,buf,WIDE*sizeof(unsigned short));
    for (j=0;j<WIDE;j++) {
      fb[idx++] = buf[j];
    }
  }
  close(h);
}

void overlayimage(char *filename) {
  int idx=0,j,i,h = open(filename,O_RDONLY);
  unsigned short buf[WIDE];

  if (h < 0) {
    perror(filename);
    return;
  }
  for (i=0;i<HIGH;i++) {
    read(h,buf,WIDE*sizeof(unsigned short));
    for (j=0;j<WIDE;j++) {
      if (fb[idx] == 0) {
	fb[idx] = buf[j];
      }
      idx++;
    }
  }
  close(h);
}

void saveimage(char *filename) {
  int idx=0,j,i,h = open(filename,O_CREAT|O_TRUNC|O_WRONLY);
  unsigned short buf[WIDE];

  if (h < 0) {
    perror(filename);
    return;
  }
  for (i=0;i<HIGH;i++) {
    for (j=0;j<WIDE;j++) {
      buf[j] = fb[idx++];
    }
    write(h,buf,WIDE*sizeof(unsigned short));
  }
  close(h);
}


int imax(const int a, const int b)
{
    if (a>b) {
	return a;
    }
    else
    {
        return b;
    }
}

void linedda(const int xa, const int ya, const int xb, const int yb, const int clr)
{
  double dx=(double)(xb-xa);
  double dy=(double)(yb-ya);
  double x=(double) xa;
  double y=(double) ya;
  int adx,ady,k,steps;
  adx=abs(dx);
  ady=abs(dy);
  steps=imax(adx,ady);
  setpixel( (int)(x+0.5), (int)(y+0.5), clr);
  if (steps>0) {
    dx /=steps;
    dy /=steps;
    for(k=0; k<steps; k++) {
      x += dx;
      y += dy;
      setpixel( (int)(x+0.5), (int)(y+0.5), clr);
    }
  }
}

/*
int do_circle() {
        float rx,ry,px,py, pi;
        unsigned short c;
        unsigned int idx, ix,iy,cx,cy;

        // slight speed up for setpixel calls
        for(idx=0; idx<WIDE; idx++) yoffsets[idx]=idx*WIDE;

	int fd = open("/dev/mem", O_RDWR|O_SYNC);
	regs = (unsigned short *)mmap(0, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0xEF200000 );
	fb = (unsigned short *)mmap(0, WIDE * HIGH * 2, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0xED000000 );
	regs += (0x30 / sizeof(unsigned short));
	regs[4] = 0x301;
        pi=0.0;
        rx=319;
        ry=239;

        ix=px;
        iy=py;
        clearvideo(0);

        c=0x0001;

        px=sin(pi)*rx;
        py=cos(pi)*ry;
        cx=(int)px;
        cy=(int)py;


        while ((rx>1) && (ry>1)) {
           px=sin(pi)*rx;
           py=cos(pi)*ry;
	   ix=320+(int)px;
           iy=240+(int)py;
           linedda(cx,cy,  ix,iy,c);
           cx=ix;
           cy=iy;
           c=c+0x8;
           pi=pi+0.2;
           rx=rx-0.025;
           ry=ry-0.015;
        }

	regs[4] = 0x300;
	close(fd);
        return 0;
}
*/
void init_video(int use_fb) {
  int fd=-1,fd2=-1;
  unsigned int idx;

  // slight speed up for setpixel calls
  for(idx=0; idx<WIDE; idx++) yoffsets[idx]=idx*WIDE;

  if (use_fb) {
    fd2= open("/dev/fb0", O_RDWR);
    if (fd2 < 0) { perror("open /dev/fb:"); exit(3); }
    fb = (unsigned short *)mmap(0, WIDE * HIGH * 2, PROT_READ|PROT_WRITE, MAP_SHARED, fd2, 0x0);

    struct fb_var_screeninfo fb_var;
    if (-1 == ioctl(fd2,FBIOGET_VSCREENINFO,&fb_var)) {
      printf("ioctl FBIOGET_VSCREENINFO : %m\n");
      exit(3);
    }
    printf("%d x %d\n",fb_var.xres,fb_var.yres);
    fd = open("/dev/mem", O_RDWR|O_SYNC);
  } else {
    fd = open("/dev/mem", O_RDWR|O_SYNC);
    fb = (unsigned short *)mmap(0, WIDE * HIGH * 2, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x60100000);
  }
  if (fb == MAP_FAILED) {perror("mmap fb"); exit(3); }
  regs = (unsigned short *)mmap(0, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x600ff000);
  if (regs == MAP_FAILED) {perror("mmap regs"); exit(3); }

  /*
    regs = (unsigned short *)mmap(0, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0xEF200000 );
    fb = (unsigned short *)mmap(0, WIDE * HIGH * 2, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0xED000000 );
  */
  //
  ts = regs + 0x20;
  backlight = regs + 0x43;
  regs += (0x30 / sizeof(unsigned short));
  //regs[4] = 0x301;

  regs[4] = 0x300;
  *backlight |= 0x20;
  usleep(500000);
  *backlight |= 0x40;

}

void set_keypress(int on)
{
  static struct termios old_settings;
  struct termios new_settings;

  if (!on) {
    tcsetattr(0,TCSANOW,&old_settings);
    return;
  }

  tcgetattr(0,&new_settings);
  old_settings = new_settings;
  /* Disable canonical mode, and set buffer size to 1 byte */
  new_settings.c_lflag &= (~ICANON);
  new_settings.c_cc[VTIME] = 0;
  new_settings.c_cc[VMIN] = 1;

  new_settings.c_lflag &= ~(ECHO);
  tcsetattr(0,TCSANOW,&new_settings);
  return;
}

int swapxy = 0, negx = 0, negy = 0;
int calib_x0 = 87, calib_dx = 3867; // 3954-87;
int calib_y0 = 129, calib_dy = 3848; //3977-129;
int ev = 0;

int get_ts_ev(int *myx,int *myy) {
  fd_set fds;
  struct timeval t;
  struct input_event event;
  static int x=-1, y=-1;

  FD_ZERO(&fds);
  FD_SET(ev,&fds);
  t.tv_sec = 0;
  t.tv_usec = 0;
  while (select(ev+1,&fds,NULL,NULL,&t)) {
    read(ev,&event,sizeof(event));
    switch (event.type) {
    case EV_REL:
      switch (event.code) {
      case REL_X:
	printf("EV_REL:X:%d\n",event.value);
	break;
      case REL_Y:
	printf("EV_REL:Y:%d\n",event.value);
	break;
      default:
	printf("EV_REL code %d\n",event.code);
	break;
      }
      break;
    case EV_ABS:
      switch (event.code) {
      case ABS_X:
	x = event.value;
	x = x * HIGH / 4096;
	break;
      case ABS_Y:
	y = event.value;
	y = y * WIDE / 4096;

	*myx = y;
	*myy = x;
	//printf("%d,%d\n",x,y);
	return 1;
      default:
	printf("EV_ABS code %d\n",event.code);
	break;
      }
      break;
    default:
      //printf("Event code %d\n",event.code);
      break;
    }
  }
  return 0;
}

int get_ts_reg(int *myx,int *myy) {
  static int downfor = 0;
  int x = swapxy ? ts[1] : ts[0];
  int y = swapxy ? ts[0] : ts[1];

  if (x & 1 && y & 1) {
    if (++downfor < 4) return 0;
    x = ((~x) >> 4) & 0xfff;
    x = (x-calib_x0) * WIDE / calib_dx;
    if (x < 0) x = 0; else if (x > 4095) x = 4095;
    if (negx) x = 4095 - x;

    y = ((~y) >> 4) & 0xfff;
    y = (y-calib_y0) * HIGH / calib_dy;
    if (y < 0) y = 0; else if (y > 4095) y = 4095;
    if (negy) y = 4095 - y;

    *myx = x;
    *myy = y;
    return 1;
  } else {
    downfor = 0;
  }
  return 0;
}

int get_ts(int *myx,int *myy) {
  if (ev) {
    return get_ts_ev(myx,myy);
  } else {
    return get_ts_reg(myx,myy);
  }
}

void get_unevent() {
  int x,y,n=0;

  while (get_ts(&x,&y));
  while (n++ < 50) {
    if (get_ts(&x,&y)) {
      n = 0;
    }
  }
}

int get_ts_raw(int *x,int *y) {
  *x = swapxy ? ts[1] : ts[0];
  *y = swapxy ? ts[0] : ts[1];
  if ((*x & 1) && (*y & 1)) {
    *x = ((~*x) >> 4) & 0xfff;
    if (negx) *x = 4095-*x;
    *y = ((~*y) >> 4) & 0xfff;
    if (negy) *y = 4095-*y;
    return 1;
  }
  return 0;
}

void get_best_event_raw(int *px,int *py) {
  int i,x,y,tx,ty;

  for (i=0;i<8;i++) {
    get_ts_raw(&x,&y);
  }
  tx=0;
  ty=0;
  i = 0;
  while (i < 8) {
    if (get_ts_raw(&x,&y)) {
      tx += x;
      ty += y;
      i++;
    }
  }
  tx /= 8;
  ty /= 8;
  *px = tx;
  *py = ty;
}

int dist(int x0,int y0,int x1,int y1) {
  int sum = 0;

  // return manhattan distance...
  sum += (x0 > x1) ? (x0-x1) : (x1-x0);
  sum += (y0 > y1) ? (y0-y1) : (y1-y0);
  return sum;
}

void save_calibration() {
  int tagmem[20];

  read_tagmem(tagmem);

  // write calibration to lattice flash
  tagmem[CALIB1_OFFSET] = (calib_x0 << 16) + calib_dx;
  tagmem[CALIB2_OFFSET] = (calib_y0 << 16) + calib_dy;
  erase_tagmem();
  write_tagmem(tagmem);
}

void calibrate_screen() {
  int x0,y0,x1,y1,x2,y2,x3,y3;
  int L,R,T,B;


  clearvideo(0);
  get_unevent();
  linedda(45,50,55,50,0xFFFF);
  linedda(50,45,50,55,0xFFFF);
  setpixel(50,50,0);
  get_best_event_raw(&x0,&y0);
  printf("%d, %d\n",x0,y0);

  clearvideo(0);
  get_unevent();
  linedda(WIDE-55,50,WIDE-45,50,0xFFFF);
  linedda(WIDE-50,45,WIDE-50,55,0xFFFF);
  setpixel(WIDE-50,50,0);
  do {
    get_best_event_raw(&x1,&y1);
  } while (dist(x0,y0,x1,y1) < 200);
  printf("%d, %d\n",x1,y1);
  clearvideo(0);
  get_unevent();

  clearvideo(0);
  get_unevent();
  linedda(45,HIGH-50,55,HIGH-50,0xFFFF);
  linedda(50,HIGH-55,50,HIGH-45,0xFFFF);
  setpixel(50,HIGH-50,0);
  do {
    get_best_event_raw(&x2,&y2);
  } while (dist(x1,y1,x2,y2) < 200);
  printf("%d, %d\n",x2,y2);

  clearvideo(0);
  get_unevent();
  linedda(WIDE-55,HIGH-50,WIDE-45,HIGH-50,0xFFFF);
  linedda(WIDE-50,HIGH-45,WIDE-50,HIGH-55,0xFFFF);
  setpixel(WIDE-50,HIGH-50,0);
  do {
    get_best_event_raw(&x3,&y3);
  } while (dist(x2,y2,x3,y3) < 200);
  printf("%d, %d\n",x3,y3);
  clearvideo(0);
  get_unevent();

  L = (x0+x2) / 2; // average offset reported to create X=50
  R = (x1+x3) / 2; // average offset reported to create X=WIDE-50
  T = (y0+y1) / 2; // average offset reported to create Y=50
  B = (y2+y3) / 2; // average offset reported to create Y=430

  calib_x0 = x0 - (R-L) * 50 / (WIDE-100); // number of pels for 50 pixels wide
  calib_y0 = y0 - (B-T) * 50 / (HIGH-100); // number of pels for 50 pixels high

  calib_dx = (R-L) * WIDE / (WIDE-100); // number of pels for WIDE pixels wide
  calib_dy = (B-T) * HIGH / (HIGH-100); // number of pels for 480 pixels high

  printf("x calibration: offset=%d, width=%d\n",calib_x0,calib_dx);
  printf("y calibration: offset=%d, width=%d\n",calib_y0,calib_dy);
}

void read_calibration() {
  int tagmem[20];

  read_tagmem(tagmem);
  if ((tagmem[CALIB1_OFFSET] >> 16) != 0xFFFF) calib_x0 = tagmem[CALIB1_OFFSET] >> 16;
  if ((tagmem[CALIB1_OFFSET] & 0xFFFF) != 0xFFFF) calib_dx = tagmem[CALIB1_OFFSET] & 0xFFFF;

  if ((tagmem[CALIB2_OFFSET] >> 16) != 0xFFFF) calib_y0 = tagmem[CALIB2_OFFSET] >> 16;
  if ((tagmem[CALIB2_OFFSET] & 0xFFFF) != 0xFFFF) calib_dy = tagmem[CALIB2_OFFSET] & 0xFFFF;

  printf("x calibration: offset=%d, width=%d\n",calib_x0,calib_dx);
  printf("y calibration: offset=%d, width=%d\n",calib_y0,calib_dy);
}

int main (int argc, char **argv) {
  char buf[3];
  int h,i,j,n,x0=-1,y0=-1,x,y,tmp;
  int doLines=0,running=1,color=0xf000,offwhendone=0;
  fd_set fds;
  struct timeval t;
  int opt_usefb=0;

  for (i=1;i<argc;i++) {
    if (argv[i][0] == 'h' || argv[i][0] == '?') {
      printf("options:\n");
      printf("swapxy swaps the x and y axis\n");
      printf("negx reverses the x axis\n");
      printf("negy reverses the y axis\n");
      printf("fb uses the framebuffer driver and auto-detects resolution\n");
      printf("VGA uses VGA resolution (default is WVGA unless auto-detected\n");
      printf("off turns off the backlight when the program exits\n");
      printf("ev uses /dev/input/event1 instead of reading the touchscreen registers directly\n");
      return 0;
    }
    if (!strcmp(argv[i],"swapxy")) {
      if (!swapxy) {
	swapxy = 1;
	printf("Swapping X and Y axes\n");
      }
    } else if (!strcmp(argv[i],"negx")) {
      if (!negx) {
	negx = 1;
	printf("Reversing X axis\n");
      }
    } else if (!strcmp(argv[i],"negy")) {
      if (!negy) {
	negy = 1;
	printf("Reversing Y axis\n");
      }
    } else if (!strcmp(argv[i],"fb")) {
      if (!opt_usefb) {
	opt_usefb = 1;
	printf("Using /dev/fb0\n");
      }
    } else if (!strcmp(argv[i],"ev")) {
      if (!ev) {
	ev = open("/dev/event1",O_RDONLY);
	if (!ev) { perror("open /dev/event1:"); return 1; }
	printf("Using /dev/event1\n");
      }
    } else if (!strcmp(argv[i],"VGA")) {
      WIDE=640;
      HIGH=480;
    } else if (!strcmp(argv[i],"off")) {
      offwhendone=1;
    }
  }

  read_calibration();
  init_video(opt_usefb);

  //h = open(argv[1], O_RDONLY); // O_DIRECT | O_SYNC | O_NONBLOCK
  set_keypress(1);
  while (running) {
    FD_ZERO(&fds);
    FD_SET(0,&fds);
    //FD_SET(h,&fds);
    t.tv_sec = 0;
    t.tv_usec = 10000;
    if (n = select(1,&fds,NULL,NULL,&t)) {
      if (FD_ISSET(0,&fds)) {
	buf[0] = 0;
	i = read(0,buf,1);
	switch (buf[0]) {
	case 'c':
	  clearvideo(0);
	  printf("Cleared\n");
	  break;
	case 'h':
	case '?':
	case 'H':
	  printf("<c>lear the screen\n");
	  printf("<k>alibrate the touch screen\n");
	  printf("<K>alibration stored to FPGA flash\n");
	  printf("<i>nvert the screen\n");
	  printf("<s>ave the screen to tstest.img\n");
	  printf("<l>oad tstest.img to the screen\n");
	  printf("<o>verlay tstest.img on the screen\n");
	  printf("Toggle between drawing <p>oints and lines\n");
	  printf("<q>uit the program\n");
	  printf("Toggle the colo<r>\n");
	  break;
	case 'k':
	  calibrate_screen();
	  break;
	case 'K':
	  save_calibration();
	  printf("Calibration saved\n");
	  break;
	case 'i':
	  invertvideo();
	  break;
	case 's':
	  saveimage("./tstest.img");
	  printf("Saved\n");
	  break;
	case 'l':
	  loadimage("./tstest.img");
	  printf("Loaded\n");
	  break;
	case 'o':
	  overlayimage("./tstest.img");
	  printf("Overlay loaded\n");
	  break;
	case 'p':
	  doLines=!doLines;
	  printf("Drawing %s\n",doLines?"lines":"points");
	  break;
	case 'q':
	  running = 0;
	  break;
	case 'r':
	  color = (color == 0xFFFF)  ? -1 : 0xFFFF;
	  printf("Color: %s\n",(color == 0xFFFF)  ? "white" : "reverse");
	  break;
	default:
	  break;
	}
      }
    }
    if (get_ts(&x,&y)) {
      //x = x * 640 / 4096;
      //y = y * 480 / 4096;
      if (x0 == x && y0 == y) {
      } else {
	// setpixel(x0,y0,lastc);
	// lastc = getpixel(x,y);
	if (doLines && x0 != -1 && y0 != -1) {
	  linedda(x0,y0,x,y,color);
	} else {
	  setpixel(x,y,color);
	}
	x0 = x;
	y0 = y;
      }
    }
  }
  set_keypress(0);
  if (offwhendone) {
    *backlight &= ~0x60;
  }
}
// gcc -static -g -mcpu=arm9 -o tstest tstest.c tagmem.o
