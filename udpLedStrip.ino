// Based on udpServer written by Brian Lee <cybexsoft@hotmail.com>
// EtherCard

/*
netsh interface ipv4 set address "Laptop wired" static 192.168.3.10 255.255.255.0
netsh interface ipv4 set address "Laptop wired" dhcp

python miniterm.py /dev/ttyS3 57600

test/udpStripSend cmd=76,-1,3,75,9,50,50,0,0,50,50,50,0,50,
*/


#include "WS2811.h"
#include "EtherCard.h"
#include "net.h"
#include "IPAddress.h"


// set to 1 to disable DHCP (adjust myip/gwip values below)
#define STATIC 1  

// Define the output function, using pin 0 on port b.
DEFINE_WS2811_FN(WS2811RGB, PORTB, 0)
#define NUM_LED 150
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

RGB_t rgb[NUM_LED];

#if STATIC
  // ethernet interface ip address
  static byte myip[] = {192, 168, 3, 200};
  // gateway ip address
  static byte gwip[] = {192, 168, 3, 1};
#endif

// set to 1 to disable DHCP (adjust myip/gwip values below)
// ethernet mac address - must be unique on your network
static byte mymac[] = {0x70, 0x69, 0x69, 0x2D, 0x30, 0x31};

byte Ethernet::buffer[500]; // tcp/ip send and receive buffer

bool doCmd = false;

unsigned int artnetAddress = 0x1234;

byte dip[4];
word dport;


void stripInit();
void display();
void copyRGB(const char *data);
void handleCmd(char *data);
void clear();
void handleArtNet(char *data);


//callback to handle the received message
/* 
 * raw: message length is NUM_LED * 3
 * cmd:start,dir,i1,i2,dLen,<data>
 * Art-Net.....
 */
void handleMessage(word port, byte ip[4], const char *data, word len) 
{
  IPAddress src(ip[0], ip[1], ip[2], ip[3]);
  //Serial.println(src);
  //Serial.println(port);
  //Serial.println(len);

  dip[0] = 192;
  dip[1] = 168;
  dip[2] = 3;
  dip[3] = 10;
  dport = port;

  ether.sendBufferUdp(len, 1234, dip, dport);
  // If the message contains only led data write it to the leds
  ether.printIp("Received message from ", ip);
  Serial.print("port=");
  Serial.print((int) port);
  Serial.print("  length=");
  Serial.println((int) len);

  if (len == NUM_LED * 3)
  {
    copyRGB(data);
    WS2811RGB(rgb, ARRAYLEN(rgb));
    doCmd = false;
    return;
  }

  if (strncmp(data, "cmd:", 4) == 0)
  {
    handleCmd((char *) &data[4]);
    clear();
    doCmd = true;
    return;
  }

  if (strncmp(data, "Art-Net", 7) == 0)
  {
    handleArtNet((char *) data);
    doCmd = false;
    return;
  }
}

void setup()
{
  Serial.begin(57600);
  Serial.println("\nApplication setup");

  if (ether.begin(sizeof Ethernet::buffer, mymac, 9) == 0)
    Serial.println( "Failed to access Ethernet controller");
#if STATIC
  ether.staticSetup(myip, gwip);
#else
  if (!ether.dhcpSetup())
    Serial.println("DHCP failed");
#endif

  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);
  ether.printIp("DNS: ", ether.dnsip);
  Serial.print("Art-Net: ");
  Serial.println(artnetAddress);

  Serial.println(sizeof(long));
  Serial.println(sizeof(int));
  Serial.println(sizeof(char));

  //register handleMessage() to port 1337
  ether.udpServerListenOnPort(&handleMessage, 1337);

  //register handleMessage() to port 42.
  ether.udpServerListenOnPort(&handleMessage, 42);

  stripInit();
}


void loop()
{
  //this must be called for ethercard functions to work.
  ether.packetLoop(ether.packetReceive());
  if (doCmd)
  {
    display();
  }
}


void clear()
{
  for (int i = 0; i < NUM_LED; i++) 
  {
    rgb[i].r = 20; 
    rgb[i].g = 0; 
    rgb[i].b = 20; 
  }
  WS2811RGB(rgb, ARRAYLEN(rgb));
}


void stripInit()
{
  // Configure pin for output.
  SET_BIT_HI(DDRB, 0);
  SET_BIT_LO(PORTB, 0);
  clear();
}


// Copy raw data to rgb array
void copyRGB(const char *data)
{
  for (int i = 0; i < NUM_LED; i++)
  {
    rgb[i].r = data[3 * i];
    rgb[i].g = data[3 * i + 1];
    rgb[i].b = data[3 * i + 2];
  }
}


//######## COMMAND MESSAGE ###########################################

int start;
int dir;
char cmdData[50] = {100, 0, 0, 0, 100, 0, 0, 0, 100};
int i1;
int i2;
int dLen;
int count = 0;


/* 
 * start, dir, i1, i2, dLen, <data>
 */
void handleCmd(char *data)
{
  char *cp1 = (char *) data;
  char *cp2 = (char *) data;

  count = 0;

  cp2 = strchr(cp1, ',');
  if (cp2 != 0)
  {
    *cp2 = 0;
    start = atoi(cp1);
    cp1 = cp2 + 1;
  }

  cp2 = strchr(cp1, ',');
  if (cp2 != 0)
  {
    *cp2 = 0;
    dir = atoi(cp1);
    cp1 = cp2 + 1;
  }

  cp2 = strchr(cp1, ',');
  if (cp2 != 0)
  {
    *cp2 = 0;
    i1 = atoi(cp1);
    cp1 = cp2 + 1;
  }

  cp2 = strchr(cp1, ',');
  if (cp2 != 0)
  {
    *cp2 = 0;
    i2 = atoi(cp1);
    cp1 = cp2 + 1;
  }

  cp2 = strchr(cp1, ',');
  if (cp2 != 0)
  {
    *cp2 = 0;
    dLen = atoi(cp1);
    cp1 = cp2 + 1;
  }

  for (int i = 0; i < dLen; i++)
  {
    cp2 = strchr(cp1, ',');
    if (cp2 != 0)
    {
      *cp2 = 0;
      cmdData[i] = atoi(cp1);
      cp1 = cp2 + 1;
    }
  }

  Serial.print("Cmd: ");
  Serial.print(start);
  Serial.print(",");
  Serial.print(dir);
  Serial.print(",");
  Serial.print(i1);
  Serial.print(",");
  Serial.print(i2);
  Serial.print(",");
  Serial.print((int) dLen);
  Serial.print(",");
  for (int i = 0; i < dLen; i++)
  {
    Serial.print((int) cmdData[i]);
    Serial.print(",");
  }
  Serial.println("");

  {
    char *c = (char *) "Received message ...";
    strcpy((char *) &(ether.buffer[UDP_DATA_P]), c);
    ether.sendBufferUdp(strlen(c) + 1, 1234, dip, dport);
  }
}


void shiftRGB(int start, int len, int dir)
{
  if (dir > 0)
  {
    for (int i = start + len - 1; i >= start; i--)
    {
      if ((i + dir >= 0) && (i + dir < NUM_LED))
      {
        rgb[i + dir].r = rgb[i].r;
        rgb[i + dir].g = rgb[i].g;
        rgb[i + dir].b = rgb[i].b;
      }
    }
  }
  if (dir < 0)
  {
    for (int i = start - len + 1; i <= start; i++)
    {
      if ((i + dir >= 0) && (i + dir >= 0))
      {
        rgb[i + dir].r = rgb[i].r;
        rgb[i + dir].g = rgb[i].g;
        rgb[i + dir].b = rgb[i].b;
      }
    }
  }
}


void shift(int start, int len, int dir
  , char *data, int dataIndex, int dataLen)
{
  if ((dir == 0) || (dataLen == 0))
  {
    return;
  }

  shiftRGB(start, len, dir);
  
  if (dir > 0)
  {
    for (int i = 0; i < dir; i++)
    {
      if ((start + i >= 0) && (start + i < NUM_LED))
      {
        int d = data[(dataIndex + 3 * i) % dataLen];
        rgb[start + i].r = d;
        d = data[(dataIndex + 3 * i + 1) % dataLen];
        rgb[start + i].g = d;
        d = data[(dataIndex + 3 * i + 2) % dataLen];
        rgb[start + i].b = d;
      }
    }
  }
  else
  {
    for (int i = 0; i < -dir; i++)
    {
      if ((start + i >= 0) && (start + i < NUM_LED))
      {
        int d = data[(dataIndex + 3 * i) % dataLen];
        rgb[start - i].r = d;
        d = data[(dataIndex + 3 * i + 1) % dataLen];
        rgb[start - i].g = d;
        d = data[(dataIndex + 3 * i + 2) % dataLen];
        rgb[start - i].b = d;
      }
    }
  }
}


void display()
{
  int len = MAX(start, NUM_LED - start);
  if (dir > 0)
  {
    shift(start, len - dir, dir, cmdData, i1 * (count / i2), dLen);
    shift(start - 1, len - dir, -dir, cmdData, i1 * (count / i2), dLen);
  }
  if (dir < 0)
  {
    shift(NUM_LED - 1, NUM_LED - start + dir, dir, cmdData, i1 * (count / i2), dLen);
    shift(0, start + dir, -dir, cmdData, i1 * (count / i2), dLen);
  }

  count++;
  if (count >= dLen * i2)
  {
    count = 0;
  }
  WS2811RGB(rgb, ARRAYLEN(rgb));
}


//######## ART-NET MESSAGE ###########################################

#define ARTNET_POLL  0x2000
#define ARTNET_DATA  0x5000

void handleArtNet(char *data)
{
/*
  unsigned int address = ((((unsigned int) data[11]) & 0xff) << 8)
    + (((unsigned int) data[10]) & 0xff);
  if (address != artnetAddress)
  {
    Serial.print("Wrong address: ");
    Serial.println((unsigned int) address);
    return;
  }

  int cmd = ((((int) data[9]) & 0xff) << 8) + (((int) data[8]) & 0xff);
  int sequence = ((int) data[12]) & 0xff;
  int len = ((((int) data[17]) & 0xff) << 8) + (((int) data[16]) & 0xff);
  len = 3 * (len / 3);

  Serial.println("Art-Net:");
  Serial.print("command = ");
  Serial.println((unsigned int) cmd);
  Serial.print("sequence = ");
  Serial.println((unsigned int) sequence);
  Serial.print("length = ");
  Serial.println((int) len);

  if (cmd == ARTNET_POLL)
  {
    char buf[42];
    int i = 0;
    // ID
    buf[i++] = 'A';
    buf[i++] = 'r';
    buf[i++] = 't';
    buf[i++] = '-';
    buf[i++] = 'N';
    buf[i++] = 'e';
    buf[i++] = 't';
    buf[i++] = '\0';
    // OpPollReply
    buf[i++] = 0;
    buf[i++] = 0x20;
    // Node IP
    buf[i++] = ether.myip[3];
    buf[i++] = ether.myip[2];
    buf[i++] = ether.myip[1];
    buf[i++] = ether.myip[0];
    // Port
    buf[i++] = 0x36;
    buf[i++] = 0x19;
    // Firmware revision
    buf[i++] = 0;
    buf[i++] = 1;
    // Netswitch
    buf[i++] = ;
    // Subswitch
    buf[i++] = ;
    // Oem
    buf[i++] = ;
    buf[i++] = ;
    // Ubea
    buf[i++] = 0;
    // Status 1
    buf[i++] = ;
    // Estaman
    buf[i++] = ;
    buf[i++] = ;
    // Shortname
    buf[i++] = ;
    buf[i++] = ;
    buf[i++] = ;
    buf[i++] = ;
    buf[i++] = ;
    buf[i++] = ;
    buf[i++] = ;
    buf[i++] = ;
    buf[i++] = ;
    buf[i++] = ;
    buf[i++] = ;
    buf[i++] = ;
    buf[i++] = ;
    buf[i++] = ;
    buf[i++] = ;
    buf[i++] = ;
    buf[i++] = ;
    buf[i++] = ;
    // Longname
    buf[i++] = ;
    buf[i++] = ;
    return;
  }

  if (cmd == ARTNET_DATA)
  {
    data = &data[18];

    for (int i = 0; i < len; i += 3)
    {
      rgb[i / 3].r = data[i];
      rgb[i / 3].g = data[i + 1];
      rgb[i / 3].b = data[i + 2];
    }
    WS2811RGB(rgb, ARRAYLEN(rgb));
    return;
  }
*/
}





