shell: 1/prog.c
	gcc -o shell 1/prog.c
clean:
	rm shell
run: shell
	./shell
