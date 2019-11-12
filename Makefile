monitor:
	g++ -Wall -Wextra -O2 -std=c++17 -lpulse -o build/pvm src/main.cc

clean:
	rm -f build/pvm
