all: server client

server: B023040041ser.o
	gcc -o server B023040041ser.o -lpthread

client: 
	gcc -o client B023040041cli.c -lpthread
B023040041ser.o:
	cc -c B023040041ser.c

dep:
	gcc -M *.c > .depend

clean:
	rm -f *.o server client .depend