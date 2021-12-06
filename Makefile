test:
	make run
	./nbody -f tests/2_bodies.txt -o

run:
	nvcc -o nbody.o -c src/nbody.cc
	nvcc -o nbody_cuda.o -c src/nbody.cu
	nvcc -o nbody nbody.o nbody_cuda.o

commit:
	git add -A
	git commit -m "Updated code"
	git push origin main
