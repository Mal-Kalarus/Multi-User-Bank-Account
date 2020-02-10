GCC = gcc

default: clean asn3

asn3: asn3.c
	$(GCC) $^ -pthread -o asn3.out

clean:
	rm -rf *.out