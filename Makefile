n = 100
s = 600

test:
	make compile
	./nbody -t 0 -r $(n) -i 0 -s $(s)
	./nbody -t 0 -r $(n) -i 1 -n 1 -s $(s)
	./nbody -t 0 -r $(n) -i 1 -n 2 -s $(s)
	./nbody -t 0 -r $(n) -i 1 -n 4 -s $(s)
	./nbody -t 0 -r $(n) -i 1 -n 8 -s $(s)
	./nbody -t 0 -r $(n) -i 1 -n 16 -s $(s)
	./nbody -t 0 -r $(n) -i 1 -n 0 -s $(s)
	./nbody -t 0 -r $(n) -i 2 -s $(s)

visualize:
	python3 src/visualize.py -s < expected_test_results/random_r$(n)_s$(s)_o2_results.txt

compile:
	nvcc -o main.o -c src/main.cc
	nvcc -o nbody.o -c src/nbody.cc
	nvcc -o nbody_cuda.o -c src/nbody.cu
	nvcc -o nbody main.o nbody.o nbody_cuda.o

commit:
	git add -A
	git commit -m "Added visualization."
	git push origin main
