/* See keyboard shortcuts for signals: stty -a
 *   SIGINT  ^C -> quit program
 *   SIGQUIT ^\ -> next test
 */

/*
test/udpStripSend cmd=0,1,3,150,9,100,0,0,0,100,0,0,0,100,
test/udpStripSend cmd=149,-1,3,150,9,100,0,0,0,100,0,0,0,100,

test/udpStripSend cmd=76,-1,3,75,9,50,50,0,0,50,50,50,0,50,
test/udpStripSend cmd=76,-1,3,150,9,50,50,0,0,50,50,50,0,50,

test/udpStripSend cmd=76,-1,3,9,36,100,40,0,100,40,0,100,40,0,100,0,0,100,0,0,100,0,0,100,100,100,100,100,100,100,100,100,0,0,100,0,0,100,0,0,100,
*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#define SRV_IP "192.168.3.200"
#define NUM_LED  150
#define BUF_LEN  (NUM_LED * 3)
#define PORT  1337

typedef struct
{
  char *d;
  int dLen;
  int start;
  int shiftLen;
  int i1;
  int i2;
} test_t;


char buf[BUF_LEN];
int s;
struct sockaddr_in si_other;
int slen = sizeof(si_other);

char data1[] =
{
  100, 0, 0, 
  0, 100, 0, 
  0, 0, 100,
  100, 0, 100,
  0, 100, 100,
  100, 100, 0,
  50, 0, 100,
  100, 50, 0,
  0, 100, 50,
  0, 50, 100,
  100, 0, 50,
  50, 100, 0,
};

char data2[] =
{
  100, 40, 0,
  100, 40, 0,
  100, 40, 0,
  100, 0, 0,
  100, 0, 0,
  100, 0, 0,
  100, 100, 100,
  100, 100, 100,
  100, 100, 100,
  0, 0, 100,
  0, 0, 100,
  0, 0, 100,
};

test_t test[] 
  = {{data1, sizeof(data1), 0, 1, 3, NUM_LED}
   , {data1, sizeof(data1), NUM_LED - 1, 1, 3, NUM_LED}
   , {data1, sizeof(data1), NUM_LED / 2 + 1, 1, 3, NUM_LED / 2}
   , {data2, sizeof(data2), NUM_LED / 2 + 1, 1, 3, 3}
  };

int testIndex = 0;


void diep(char *s);
void sigHandler(int sig);
void msleep(int msec);
void sendBuf(int len);
void fill(char *buf, int r, int g, int b);
// buf: buffer with led data (NUM_LED * 3)
// start: index of LED to start shifting
// dir: number of places to shift; number of LEDs; sign of dir determines 
//      the shift direction.
// data: data to shift in
// dataIndex: first byte of data to use
// len: length of data in bytes
void shift(char *buf, int start, int dir
  , char *data, int dataIndex, int dataLen);


int main(int argc, char *argv[])
{
  int i;
  const char *srvIp = SRV_IP;
  int port = PORT;
  char *cmd = 0;

  for (int i = 1; i < argc; i++)
  {
    if (strncmp(argv[i], "s=", 2) == 0)
    {
      srvIp = &(argv[i][2]);
    }
    else if (strncmp(argv[i], "p=", 2) == 0)
    {
      port = atoi(&(argv[i][2]));
    }
    else if (strncmp(argv[i], "cmd=", 4) == 0)
    {
      cmd = argv[i];
    }
  }
  printf("Using %s@%d\n", srvIp, port);

  if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
  {
    diep((char *) "socket");
  }

  memset((char *) &si_other, 0, sizeof(si_other));
  si_other.sin_family = AF_INET;
  si_other.sin_port = htons(port);
  if (inet_aton(srvIp, &si_other.sin_addr) == 0) 
  {
    fprintf(stderr, "inet_aton() failed\n");
    exit(1);
  }

  signal(SIGINT, sigHandler);
  signal(SIGQUIT, sigHandler);
 
  if (cmd != 0)
  {
    strcpy(buf, cmd);
    buf[3] = ':';
    sendBuf(strlen(buf) + 1);
    msleep(30);
  }
  else
  {
    for (i = 0;; i++)
    {
      int t = testIndex;

      printf("Sending packet %d\n", i);
      shift(buf, test[t].start, test[t].shiftLen
        , test[t].d, test[t].i1 * (i / test[t].i2), test[t].dLen);
      shift(buf, test[t].start - 1, -test[t].shiftLen
        , test[t].d, test[t].i1 * (i / test[t].i2), test[t].dLen);

      //fill(buf, (i % 3) * 20, (i % 4) * 15, (i % 5) * 10);
      // number of colors: 3
      // number of data bytes to shift in: 3 (number of colors * 1)
      // data index: after every NUM_LED shifts increment data index, 
      //   limit data index to sizeof(data) / number of colors.
      //   Increment in steps of 3!
      sendBuf(BUF_LEN);
      msleep(30);
    }
  }

  signal(SIGINT, SIG_DFL);
  signal(SIGQUIT, SIG_DFL);

  close(s);
  return 0;
}


void diep(char *s)
{
  perror(s);
  exit(1);
}


void sigHandler(int sig)
{
  if (sig == SIGINT)
  {
    fill(buf, 0, 0, 0);
    sendBuf(BUF_LEN);
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    exit(0);
  }
  else if (sig == SIGQUIT)
  {
    testIndex = (testIndex + 1) % (sizeof(test) / sizeof(test_t));
  }
}


void msleep(int msec)
{
  /* While not yet there; wait */
  struct timeval wait;
  wait.tv_sec = 0;
  wait.tv_usec = msec * 1000;

  /* Using select here as an accurate sleep function */
  /* Return value 0 means `normal timeout' */
  if (select(0, NULL, NULL, NULL, &wait)) 
  {
  }
}


void sendBuf(int len)
{
  if (sendto(s, buf, len, 0, (const sockaddr*) &si_other, slen) == -1)
  {
    diep((char *) "sendto()");
  }
}


void fill(char *buf, int r, int g, int b)
{
  int i;
  for (i = 0; i < NUM_LED; i++)
  {
    buf[i * 3] = r;
    buf[i * 3 + 1] = g;
    buf[i * 3 + 2] = b;
  }
}


void shift(char *buf, int start, int dir
  , char *data, int dataIndex, int dataLen)
{
  int i;

  if ((start < 0) || (start >= NUM_LED) || (dir == 0))
  {
    return;
  }

  start *= 3;
  dir *= 3;
  if (dir > 0)
  {
    for (i = 3 * NUM_LED - 1 - dir; i >= start; i--)
    {
      buf[i + dir] = buf[i];
    }
    for (i = 0; i < dir; i++)
    {
      int d = data[(dataIndex + i) % dataLen];
      buf[i + start] = d;
      printf("%02x ", d);
    }
    printf("\n");
  }
  else
  {
    for (i = -dir; i <= start + 2; i++)
    {
      buf[i + dir] = buf[i];
    }
    for (i = 0; i < -dir; i++)
    {
      int d = data[(dataIndex + (-dir - 1 - i)) % dataLen];
      buf[start  + 2 - i] = d;
      printf("%02x ", d);
    }
    printf("\n");
  }
}


