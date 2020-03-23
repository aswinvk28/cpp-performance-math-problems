CXX=g++
CXXFLAGS=-c -fopenmp 
LDFLAGS=-fopenmp -lmkl_core -lmkl
INC=-I/usr/include -I/opt/intel/compilers_and_libraries_2020.0.166/linux/mkl/include/ -I/home/aswin/Documents/Courses/Udacity/Intel-Edge/Repository/cpp-performance-dispersion-intervals/autodiff
LIB=-L/home/aswin/anaconda3/envs/enscalo_test/lib/

OBJECTS=main.o worker.o

default : app

worker.o : worker.cc
	${CXX} -c ${CXXFLAGS} $(LDFLAGS) $(INC) -o "$@" "$<"

app: main.cc worker.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LIB) $(INC) -o app $(OBJECTS)

all:	app

run:	all
	./app

clean:
	rm -f *.optrpt *.o app
