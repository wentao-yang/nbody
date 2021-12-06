test:
	make run
	# ./nbody -f tests/2_bodies.txt -t 2 -x 10 -o > test_results/2_bodies_2_threads.txt
	./nbody -r -n 2000 -s -x 15 -o > test_results/random_2000_sequential.txt

run:
	nvcc -o nbody.o -c src/nbody.cc
	nvcc -o nbody_cuda.o -c src/nbody.cu
	nvcc -o nbody nbody.o nbody_cuda.o

commit:
	git add -A
	git commit -m "Updated code"
	git push origin main
