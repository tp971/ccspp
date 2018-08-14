CC=gcc
CXX=g++
LD=ar rvc

Cflags=-c -MD
CXXflags=-c -MD --std=c++14 -O3
LDflags=

Input=ccs.cpp ccsparser.cpp
ObjDir=obj
BinDir=lib
Output=libccs++.a

Objects=$(addprefix $(ObjDir)/,$(addsuffix .o, $(Input)))

all: makedirs makelibs $(BinDir)/$(Output)

.PHONY: makedirs
makedirs: $(ObjDir)/ $(BinDir)/

%/:
	mkdir -p $@

makelibs:
#	make -C ...

$(BinDir)/$(Output): $(Objects)
	$(LD) -o $(BinDir)/$(Output) $(Objects) $(LDflags)

obj/%.c.o: %.c
	$(CC) $(Cflags) -o $@ $<

obj/%.cpp.o: %.cpp
	$(CXX) $(CXXflags) -o $@ $<

-include $(ObjDir)/*.d

.IGNORE: clean
clean:
	rm $(ObjDir)/*
	rm $(BinDir)/$(Output)
