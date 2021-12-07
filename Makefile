n = 2000
x = 15

test:
	make run
	./nbody -f tests/2_bodies.txt -s -x 10 -o > test_results/2_bodies_10s_s.txt
	./nbody -f tests/2_bodies.txt -t 1 -x 10 -o > test_results/2_bodies_10s_1t.txt
	./nbody -f tests/2_bodies.txt -t 2 -x 10 -o > test_results/2_bodies_10s_2t.txt
	./nbody -f tests/2_bodies.txt -c -x 10 -o > test_results/2_bodies_10s_c.txt
	./nbody -f tests/3_bodies.txt -s -x 10 -o > test_results/3_bodies_10s_s.txt
	./nbody -f tests/3_bodies.txt -t 1 -x 10 -o > test_results/3_bodies_10s_1t.txt
	./nbody -f tests/3_bodies.txt -t 2 -x 10 -o > test_results/3_bodies_10s_2t.txt
	./nbody -f tests/3_bodies.txt -c -x 10 -o > test_results/3_bodies_10s_c.txt
	./nbody -r -n $(n) -s -x $(x) -o > test_results/random_2000_15s_s.txt
	./nbody -r -n $(n) -t 1 -x $(x) -o  > test_results/random_2000_15s_1t.txt
	./nbody -r -n $(n) -t 2 -x $(x) -o > test_results/random_2000_15s_2t.txt
	./nbody -r -n $(n) -t 4 -x $(x) -o > test_results/random_2000_15s_4t.txt
	./nbody -r -n $(n) -t 8 -x $(x) -o > test_results/random_2000_15s_8t.txt
	./nbody -r -n $(n) -t 16 -x $(x) -o > test_results/random_2000_15s_16t.txt
	./nbody -r -n $(n) -t 0 -x $(x) -o > test_results/random_2000_15s_0t.txt
	./nbody -r -n $(n) -c -x $(x) -o > test_results/random_2000_15s_c.txt

run:
	nvcc -o nbody.o -c src/nbody.cc
	nvcc -o nbody_cuda.o -c src/nbody.cu
	nvcc -o nbody nbody.o nbody_cuda.o

commit:
	git add -A
	git commit -m "Fixed CUDA implementation bug"
	git push origin main
