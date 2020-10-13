main: src/main.c
	cpplint --filter=-legal / copyright src/main.c
	gcc src/main.c -o bin/main -Wall -lm  
