PRJ = udpStripSend udpListen

all: $(PRJ)

%: %.c
	gcc -Wall -o $@ $^ 

clean:
	rm -f $(PRJ)

