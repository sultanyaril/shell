main: src/main.c
	cpplint --filter=-legal / copyright src/$@.c
	gcc src/$@.c -o bin/$@ -Wall -lm  
