
.PHONY: functors run

CFLAGS = -std=c++17 -fPIC $(EXTRA_CFLAGS)

run:
	souffle -Fsamples/multi_function -D- type_inference.dl

functors: libfunctors.so

libfunctors.so: ${addprefix obj/, functors.o set_functors.o list_functors.o ir_type_functors.o}
	g++ -shared -o $@ $+ 

obj/%.o: src/functors/%.cpp
	@mkdir -p ${dir $@}
	g++ -c $(CFLAGS) -o $@ $<
