hostd: dispatcher.c sigtrap.c
	gcc -g dispatcher.c -o dispatcher -Wall 
	gcc -g sigtrap.c -o process -Wall

.PHONY: clean
clean:
	rm -f dispatcher process
