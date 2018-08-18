#CC=gcc
#CXX=g++

Cflags=-c -MD
CXXflags=-c -MD -Idep --std=c++14 -O3
LDflags=-Ldep/cli++/lib -Lccs++/lib -lcli++ -lccs++

Input=main.cpp cmd_graph.cpp cmd_random.cpp cmd_ttr.cpp
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
	make -C ccs++

$(BinDir)/$(Output): $(Objects) ccs++/lib/libccs++.a
	$(CXX) -o $(BinDir)/$(Output) $(Objects) $(LDflags)

obj/%.c.o: %.c
	$(CC) $(Cflags) -o $@ $<

obj/%.cpp.o: %.cpp
	$(CXX) $(CXXflags) -o $@ $<

-include $(ObjDir)/*.d

.IGNORE: clean
clean:
	make -C ccs++ clean
	rm $(ObjDir)/*
	rm $(BinDir)/$(Output)

.PHONY: install
install: $(BinDir)/$(Output)
	make -C ccs++ install
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp $< $(DESTDIR)$(PREFIX)/bin/$(Output)

.PHONY: uninstall
uninstall:
	make -C ccs++ uninstall
	rm -f $(DESTDIR)$(PREFIX)/bin/$(Output)
