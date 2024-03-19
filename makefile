
.PHONY: functors run

run:
	souffle -Fsamples/multi_function -D- type_inference.dl

functors: functors.cpp
	g++ functors.cpp -c -fPIC -o functors.o
	g++ -shared -o libfunctors.so functors.o
