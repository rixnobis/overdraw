TARGET = overdraw
TYPE = ps-exe

SRCS = \
main.cpp \
overdraw.cpp \
walk.cpp \
levels.cpp \

CXXFLAGS = -std=c++20

# make AUTOPLAY=true builds the attract version, which loads every level
# pre-solved and runs it. Objects are not flag-tracked, so this target forces a
# rebuild of main.o rather than trusting an incremental build across a define
# change - a stale object here would silently produce the wrong binary while
# reporting success.
ifeq ($(AUTOPLAY),true)
CXXFLAGS += -DOVERDRAW_AUTOPLAY
TARGET = overdraw-autoplay
endif

# Point this at a nugget checkout, or at the src/mips directory of a
# pcsx-redux tree. psyqo.mk works out its own location, so an absolute path
# here is enough and this project does not need to live inside that tree.
PSYQO_ROOT ?= third_party/nugget

include $(PSYQO_ROOT)/psyqo/psyqo.mk

# Host-side level verifier. Links the same probe, walker and level data the
# console binary does, so a level cannot be solvable here and unsolvable there.
HOST_CXX ?= g++

solver: solver.cpp overdraw.cpp walk.cpp levels.cpp overdraw.hh walk.hh levels.hh
	$(HOST_CXX) -std=c++20 -O2 -Wall -o $@ solver.cpp overdraw.cpp walk.cpp levels.cpp

.PHONY: verify
verify: solver
	./solver

clean::
	rm -f solver
