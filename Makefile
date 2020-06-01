OBJ = main.o

readP: $(OBJ)
	gcc $(OBJ) -o  MiniCron -pthread

$(OBJ):

.PHONY: clean
clean:
	rm -f *.o
	rm -f MiniCron
