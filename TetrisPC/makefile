CC=g++
CFLAGS=-std=c++17 -O3 -fopenmp
LIBS=-static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic
DLLFLAGS=-shared

all: pc_core.dll benchmark.exe

benchmark.exe: TetrisPC.o solver_parallel.o solver_divider.o solver.o auxiliary_tree.o basic_traverser.o simulator.o endgame_generator.o
	$(CC) -o $@ $^ $(CFLAGS)

pc_core.dll: dll_interface.o dll_entry.o solver_parallel.o solver_divider.o solver.o auxiliary_tree.o basic_traverser.o
	$(CC) -o $@ $^ $(CFLAGS) $(DLLFLAGS)


dll_interface.o: dll_interface.cpp dll_interface.h
	$(CC) -c -o $@ $< $(CFLAGS) $(INC)

dll_entry.o: dll_entry.cpp
	$(CC) -c -o $@ $< $(CFLAGS) $(INC)

TetrisPC.o: TetrisPC.cpp
	$(CC) -c -o $@ $< $(CFLAGS) $(INC)

solver_parallel.o: solver_parallel.cpp solver_parallel.h
	$(CC) -c -o $@ $< $(CFLAGS) $(INC)

solver_divider.o: solver_divider.cpp solver_divider.h
	$(CC) -c -o $@ $< $(CFLAGS) $(INC)

solver.o: solver.cpp solver.h container_wrapper.h
	$(CC) -c -o $@ $< $(CFLAGS) $(INC)

auxiliary_tree.o: auxiliary_tree.cpp auxiliary_tree.h
	$(CC) -c -o $@ $< $(CFLAGS) $(INC)

basic_traverser.o: basic_traverser.cpp basic_traverser.h
	$(CC) -c -o $@ $< $(CFLAGS) $(INC)
	
simulator.o: simulator.cpp simulator.h
	$(CC) -c -o $@ $< $(CFLAGS) $(INC)
	
endgame_generator.o: endgame_generator.cpp endgame_generator.h
	$(CC) -c -o $@ $< $(CFLAGS) $(INC)

.PHONY: clean

clean:
	rm -f *.o
