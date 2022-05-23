#
# Sample Makefile to generate LLVM passes
#
# make all builds the sample template as a shared library
# make test runs the library against an example IR
# make clean deletes the built files
#
# 05 May 2022  jpb  Creation.
#

CC=clang-10
CFLAGS=`llvm-config-10 --cflags` -fPIC -I/usr/include/llvm-10/
CXX=clang++-10
CXXFLAGS=`llvm-config-10 --cxxflags` -fPIC -I/usr/include/llvm-10/
LDFLAGS=`llvm-config-10 --ldflags`
OPT=opt-10


TEMPLATENAME=ConstantPropagation

all:	$(TEMPLATENAME).so

$(TEMPLATENAME).so:	$(TEMPLATENAME).o
	$(CXX) --shared -o $(TEMPLATENAME).so ${LDFLAGS} $^

test:	$(TEMPLATENAME).so
	$(OPT) -S -load ./$(TEMPLATENAME).so -constantpropagationpass < test1.ll
	$(OPT) -S -load ./$(TEMPLATENAME).so -constantpropagationpass < iftest.ll

clean:
	$(RM) $(TEMPLATENAME).o $(TEMPLATENAME).so
