all: server client

server: bin/monitor

client: bin/tracer


folders:
	@mkdir -p src obj bin tmp 

obj/Lista_ligada.o: src/Lista_ligada.c | folders
	gcc -Wall -g -c src/Lista_ligada.c -o obj/Lista_ligada.o 

obj/struct.o: src/struct.c | folders
	gcc -Wall -g -c src/struct.c -o obj/struct.o

obj/monitor_func.o: src/monitor_func.c | folders
	gcc -Wall -g -c src/monitor_func.c -o obj/monitor_func.o 	

obj/monitor.o: src/monitor.c | folders
	gcc -Wall -g -c src/monitor.c -o obj/monitor.o 

obj/tracer.o: src/tracer.c | folders
	gcc -Wall -g -c src/tracer.c -o obj/tracer.o

bin/monitor: obj/monitor.o obj/Lista_ligada.o obj/struct.o obj/monitor_func.o | folders
	gcc -g obj/monitor.o obj/Lista_ligada.o obj/struct.o obj/monitor_func.o -o bin/monitor 

bin/tracer: obj/tracer.o obj/struct.o | folders
	gcc -g obj/tracer.o obj/struct.o -o bin/tracer 

clean:
	rm -f obj/* tmp/* bin/{tracer,monitor}
