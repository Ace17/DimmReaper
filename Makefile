BIN?=bin

all: $(BIN)/dimmreaper.exe

CXXFLAGS?=
CXXFLAGS+=-O3

LDFLAGS?=
LDFLAGS+=-s

CXXFLAGS+=-Wall -Wextra

HOST:=$(shell $(CXX) -dumpmachine | sed 's/.*-//')

srcs:=\
	src/main.cpp\
	src/tests.cpp\
	src/alloc_$(HOST).cpp\

-include sys-$(HOST).mk

$(BIN)/dimmreaper.exe: $(srcs)
	@mkdir -p $(dir $@)
	$(CXX) -o "$@" $(CXXFLAGS) $(LDFLAGS) $^

clean:
	rm -rf $(BIN)
