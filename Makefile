CC = gcc
CFLAGS = -Wall -Wextra -ggdb -lX11 -lXcomposite -lXrender -lXfixes -lXext
TARGET = rattle

rattle: src/*
	$(CC) $(CFLAGS) src/* -o $(TARGET)
