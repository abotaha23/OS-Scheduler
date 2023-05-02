build:
	gcc process_generator.c -o process_generator
	gcc clk.c -o clk
	gcc scheduler.c -o scheduler -lm
	gcc process.c -o process


clean:
	rm -f *.out  processes.txt

all: clean build

run:
	./process_generator