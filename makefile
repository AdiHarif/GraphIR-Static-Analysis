
.PHONY: functors run

run:
	souffle -Fsamples/multi_function -D- type_inference.dl

functors:
	g++ -c -fPIC src/functors/*.cpp
	g++ -shared -o libfunctors.so functors.o set_functors.o list_functors.o
