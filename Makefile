CC = gcc
CFLAGS = -Wall -Wextra -ggdb -lX11 -lXcomposite -lXrender -lXfixes -lXext -lXdamage
TARGET = citadel

citadel: src/*
	$(CC) $(CFLAGS) src/* -o $(TARGET)
