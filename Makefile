build:
	g++ -O3 -march=native main.cpp -o cemu
clean:
	rm -f ./cemu