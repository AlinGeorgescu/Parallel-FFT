all: parallelFT parallelFFT

parallelFT: parallelFT.c
	gcc -o parallelFT parallelFT.c -O3 -lpthread -lm -Wall

parallelFFT: parallelFFT.c
	gcc -o parallelFFT parallelFFT.c -O3 -lpthread -lm -Wall

clean:
	rm parallelFFT parallelFT