monitor:
	$(CXX) -Wall -Wextra -O2 -std=c++17 -lpulse -o build/pvm src/main.cc

clean:
	rm -f build/pvm

install:
	cp build/pvm /usr/local/bin/pvm

uninstall:
	rm /usr/local/bin/pvm
