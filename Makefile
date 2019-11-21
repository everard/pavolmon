TARGET_NAME=pavolmon

monitor:
	$(CC) -Wall -Wextra -O2 -std=c11 -lpulse -o build/$(TARGET_NAME) src/pavolmon.c

monitor-bars:
	$(CC) -Wall -Wextra -O2 -std=c11 -DPVM_PRINT="\"format_bars.c\"" -lpulse -o build/$(TARGET_NAME) src/pavolmon.c

monitor-custom-format:
	$(CC) -Wall -Wextra -O2 -std=c11 -DPVM_PRINT="\"$(FORMAT)\"" -lpulse -o build/$(TARGET_NAME) src/pavolmon.c

clean:
	rm -f build/$(TARGET_NAME)

install:
	cp build/$(TARGET_NAME) /usr/local/bin/$(TARGET_NAME)

uninstall:
	rm /usr/local/bin/$(TARGET_NAME)
