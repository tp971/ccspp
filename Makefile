CC=gcc
CXX=g++
LD=g++

Cflags=-c -MD
CXXflags=-c -MD -Idep --std=c++14 -O3
LDflags=-Ldep/cli++/lib -Llibccs++/lib -lcli++ -lccs++

Input=main.cpp cmd_graph.cpp cmd_random.cpp
ObjDir=obj
BinDir=bin
Output=ccs++

PREFIX=/usr/local

Objects=$(addprefix $(ObjDir)/,$(addsuffix .o, $(Input)))

all: makedirs makelibs $(BinDir)/$(Output)
	cd $(BinDir); ./$(Output)

.PHONY: makedirs
makedirs: $(ObjDir)/ $(BinDir)/

%/:
	mkdir -p $@

makelibs:
	if [ -f dep/cli++/Makefile ]; then make -C dep/cli++; fi
	make -C libccs++

$(BinDir)/$(Output): $(Objects) libccs++/lib/libccs++.a
	$(LD) -o $(BinDir)/$(Output) $(Objects) $(LDflags)

obj/%.c.o: %.c
	$(CC) $(Cflags) -o $@ $<

obj/%.cpp.o: %.cpp
	$(CXX) $(CXXflags) -o $@ $<

-include $(ObjDir)/*.d

.IGNORE: clean
clean:
	make -C libccs++ clean
	rm $(ObjDir)/*
	rm $(BinDir)/$(Output)

.PHONY: install
install: $(BinDir)/$(Output)
	make -C libccs++ install
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp $< $(DESTDIR)$(PREFIX)/bin/$(Output)

.PHONY: uninstall
uninstall:
	make -C libccs++ uninstall
	rm -f $(DESTDIR)$(PREFIX)/bin/$(Output)
