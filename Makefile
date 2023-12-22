CC = gcc

# ldconfig -p | grep "libX.*"
# or use pkgfile
# why is this such a pain to find the object files from header names :(
CFLAGS = -Wall -Wextra -ggdb -lX11 -lXcomposite -lXdamage -lXfixes
TARGET = citadel

citadel: src/*
	$(CC) $(CFLAGS) src/* -o $(TARGET)
