n = 4000
x = 120

test:
	make run
	./nbody -r -n $(n) -s -x $(x)
	./nbody -r -n $(n) -t 1 -x $(x)
	./nbody -r -n $(n) -t 2 -x $(x)
	./nbody -r -n $(n) -t 4 -x $(x)
	./nbody -r -n $(n) -t 8 -x $(x)
	./nbody -r -n $(n) -t 16 -x $(x)
	./nbody -r -n $(n) -t 0 -x $(x)
	./nbody -r -n $(n) -c -x $(x)

run:
	nvcc -o nbody.o -c src/nbody.cc
	nvcc -o nbody_cuda.o -c src/nbody.cu
	nvcc -o nbody nbody.o nbody_cuda.o

commit:
	git add -A
	git commit -m "Updated Makefile"
	git push origin main
