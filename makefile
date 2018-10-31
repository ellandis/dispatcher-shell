hostd: dispatcher.c sigtrap.c
	gcc -g dispatcher.c cda.c -o dispatcher -Wall 
	gcc -g sigtrap.c -o process -Wall

.PHONY: clean
clean:
	rm -f dispatcher process
