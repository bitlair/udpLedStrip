#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFLEN 512
#define PORT 1235

void sigHandler(int sig);

void diep(char *s)
{
  perror(s);
  exit(1);
}

int main(int argc, char *argv[])
{
  struct sockaddr_in si_me;
  struct sockaddr_in si_other;
  int s;
  int i;
  int slen = sizeof(si_other);
  char buf[BUFLEN];
  int port = PORT;

  if (argc == 2)
  {
    port = atoi(argv[1]);
  }
  printf("Listening to port %d\n", port);

  if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
  {
    diep("socket");
  }

  signal(SIGINT, sigHandler);
  signal(SIGQUIT, sigHandler);
 
  memset((char *) &si_me, 0, sizeof(si_me));
  si_me.sin_family = AF_INET;
  si_me.sin_port = htons(port);
  si_me.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(s, (struct sockaddr *) &si_me, sizeof(si_me)) == -1)
  {
    diep("bind");
  }

  for (i = 0;; i++)
  {
    if (recvfrom(s, buf, BUFLEN, 0
      , (struct sockaddr *) &si_other, &slen) == -1)
    {
      diep("recvfrom()");
    }
    printf("Received packet from %s:%d\nData: %s\n\n"
      , inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);
  }

  close(s);
  return 0;
}


void sigHandler(int sig)
{
  if (sig == SIGINT)
  {
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    exit(0);
  }
  else if (sig == SIGQUIT)
  {
  }
}


