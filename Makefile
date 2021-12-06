test:
	make run
	./nbody -f tests/2_bodies.txt -t 2 -x 10 -o > test_results/2_bodies_2_threads.txt
	# ./nbody -r -n 2000 -t 8 -x 15

run:
	nvcc -o nbody.o -c src/nbody.cc
	nvcc -o nbody_cuda.o -c src/nbody.cu
	nvcc -o nbody nbody.o nbody_cuda.o

commit:
	git add -A
	git commit -m "Updated code"
	git push origin main
