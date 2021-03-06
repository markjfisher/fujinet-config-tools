/**
 * FujiNet Tools for CLI
 *
 * feject - Eject disk in device slot
 *
 * usage:
 *  feject <ds#>
 *
 * Author: Thomas Cherryhomes
 *  <thom.cherryhomes@gmail.com>
 *
 * Released under GPL, see COPYING
 * for details
 */

#include <atari.h>
#include <string.h>
#include <stdlib.h>
#include <peekpoke.h>
#include "sio.h"
#include "conio.h"
#include "err.h"

unsigned char buf[128];

union
{
  char host[8][32];
  unsigned char rawData[256];
} hostSlots;

/**
 * Read host slots
 */
void host_read(void)
{
  // Query for host slots
  OS.dcb.ddevic=0x70;
  OS.dcb.dunit=1;
  OS.dcb.dcomnd=0xF4; // Get host slots
  OS.dcb.dstats=0x40;
  OS.dcb.dbuf=&hostSlots.rawData;
  OS.dcb.dtimlo=0x0f;
  OS.dcb.dbyt=256;
  OS.dcb.daux=0;
  siov();

  if (OS.dcb.dstats!=1)
    {
      err_sio();
      exit(OS.dcb.dstats);
    }
}

/**
 * Mount a host slot
 */
void host_mount(unsigned char c)
{
  if (hostSlots.host[c][0]!=0x00)
    {
      OS.dcb.ddevic=0x70;
      OS.dcb.dunit=1;
      OS.dcb.dcomnd=0xF9;
      OS.dcb.dstats=0x00;
      OS.dcb.dbuf=NULL;
      OS.dcb.dtimlo=0x01;
      OS.dcb.dbyt=0;
      OS.dcb.daux=c;
      siov();
      if (OS.dcb.dstats!=1)
	{
	  err_sio();
	  exit(OS.dcb.dstats);
	}
    }
}

/**
 * Open TNFS directory on host slot
 */
void directory_open(unsigned char hs, char* b)
{
  // Open TNFS directory
  OS.dcb.ddevic=0x70;
  OS.dcb.dunit=1;
  OS.dcb.dcomnd=0xF7;
  OS.dcb.dstats=0x80;
  OS.dcb.dbuf=b;
  OS.dcb.dtimlo=0x0F;
  OS.dcb.dbyt=256;
  OS.dcb.daux=hs;
  siov();

  if (OS.dcb.dstats!=1)
    {
      err_sio();
      exit(OS.dcb.dstats);
    }
}

/**
 * Read TNFS directory entry in host slot
 * and of specific length
 */
void directory_read(unsigned char hs, unsigned char len)
{
  OS.dcb.dcomnd=0xF6;
  OS.dcb.dstats=0x40;
  OS.dcb.dbuf=&buf;
  OS.dcb.dbyt=len;
  OS.dcb.daux1=len;
  OS.dcb.daux2=hs;
  siov();
  
  if (OS.dcb.dstats!=1)
    {
      err_sio();
      exit(OS.dcb.dstats);
    }
}

/**
 * Close TNFS Directory
 */
void directory_close(unsigned char hs)
{
  // Close directory
  OS.dcb.dcomnd=0xF5;
  OS.dcb.dstats=0x00;
  OS.dcb.dbuf=NULL;
  OS.dcb.dtimlo=0x01;
  OS.dcb.dbyt=0;
  OS.dcb.daux=hs;
  siov();

  if (OS.dcb.dstats!=1)
    {
      err_sio();
      exit(OS.dcb.dstats);
    }
}

/**
 * Set current directory cursor
 */
void set_directory_position(unsigned short pos)
{
  OS.dcb.dcomnd = 0xE4;
  OS.dcb.dstats = 0x00;
  OS.dcb.dbuf = NULL;
  OS.dcb.dbyt = 0;
  OS.dcb.daux = pos;
  siov();
}

/**
 * show options
 */
void opts(char* argv[])
{
  print(argv[0]);
  print(" <hs#>,[path]:[filter]\x9b\x9b");
  print("<hs#> - host slot (1-8)\x9b");
  print("[path] - Path or '/' if not specified\x9b");
  print("[filter] - Optional wildcard.");
}

/**
 * main
 */
int main(int argc, char* argv[])
{
  unsigned char s=argv[1][0]-0x30;
  unsigned char sa[2];
  
  unsigned char i,j;

  unsigned short pos=0;
  
  OS.lmargn=2;
  
  if (_is_cmdline_dos())
    {
      if (argc<2)
	{
	  opts(argv);
	  return(1);
	}
      else
	{
	  for (i=1;i<=argc;i++)
	    {
	      strcat(buf,argv[i]);
	      if (i<argc-1)
		strcat(buf," ");
	    }
	}
    }
  else // DOS 2.0
    {
      print("HOST DIRECTORY--HOST SLOT, PATH?\x9b");
      get_line(buf,sizeof(buf));
    }

  if (buf[0]==0x00)
    s=1;
  else
    s=buf[0]-0x30;
  
  sa[0]=s+0x30;

  if (s<1 || s>8)
    {
      print("INVALID SLOT NUMBER.\x9b");
      return(1);
    }
  
  // Check for first comma
  for (i=0;i<strlen(buf);i++)
    {
      if (buf[i]==',')
	{
	  i++;
	  break;
	}	  
    }
  
  // Trim any whitespace
  while (buf[i]==' ')
    i++;
  
  j=i; // Store path cursor for later.
  
  // Check for colon for filter.
  for (i=i;i<strlen(buf);i++)
    {
      if (buf[i]==':')
	{
	  buf[i]=0x00; // NULL separator required for pattern.
	  break;
	}
    }
  
  s-=1;
  
  // Read in host and device slots from FujiNet
  host_read();

  // Mount host for reading
  host_mount(s);

  directory_open(s,&buf[j]);

  // Read directory
  while (buf[0]!=0x7F)
    {
      memset(buf,0,sizeof(buf));
      buf[0]=0x7f;

      directory_read(s,36); // show 36 chars max

      if (strlen(buf)==35)
	{
	  set_directory_position(pos);
	  directory_read(s,128);
	}
      
      if (buf[0]=='.')
	continue;
      else if (buf[0]==0x7F)
	break;
      else
	{
	  print(buf);
	  print("\x9b");
	}
      pos++;
    }

  directory_close(s);

  print("\x9b");
  
  return(0);
}
