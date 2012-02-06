# Get NAIAD_PATH from your shell environment
NAIAD_SERVER_PATH=$(NAIAD_PATH)/server
CXX=g++
CXXFLAGS ?= -g -O2 -m64

NAIAD_INC_DIR  = -I$(NAIAD_SERVER_PATH)/include/em
NAIAD_INC_DIR += -I$(NAIAD_SERVER_PATH)/include/Nb
NAIAD_INC_DIR += -I$(NAIAD_SERVER_PATH)/include/Ng
NAIAD_INC_DIR += -I$(NAIAD_SERVER_PATH)/include/Ni
NAIAD_INC_DIR += -I$(NAIAD_SERVER_PATH)/include/system

NAIAD_LDFLAGS  = -L$(NAIAD_SERVER_PATH)/lib

NAIAD_LIBS     = -lNb
NAIAD_LIBS    += -lNi
NAIAD_LIBS    += -liomp5
NAIAD_LIBS    += -pthread			# gcc specific

TARGET         = emp2particle

$(TARGET): emp2particle.cc
	$(CXX) $(CXXFLAGS) $(NAIAD_INC_DIR) -o $(TARGET) emp2particle.cc lz4.c $(NAIAD_LDFLAGS) $(NAIAD_LIBS)


.PHONY: clean

clean:
	rm -rf $(TARGET)
