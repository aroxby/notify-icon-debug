SRC_DIR=src
SRCS=$(shell find $(SRC_DIR) -name '*.cpp')
OBJS=$(subst .cpp,.o,$(SRCS))
TARGET=nid.exe
CPP=g++

.PHONY: default all tidy clean

default: all

all: $(TARGET)

%.o: %.cpp $(TARGET_DIR)
	$(CPP) -c $< -o $@

$(TARGET): $(OBJS)
	$(CPP) $^ -o $@

tidy:
	rm -f $(OBJS)

clean: tidy
	rm -f $(TARGET)