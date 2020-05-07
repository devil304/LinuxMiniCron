OBJ = main.o

readP: $(OBJ)
	gcc $(OBJ) -O3 -o  MiniCron -pthread

$(OBJ):

.PHONY: clean
clean:
	rm -f *.o
	rm -f MiniCron
