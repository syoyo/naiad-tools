NAIAD_PATH=/home/syoyo/src/naiad-0.6.0.69-x86_64/server
CXX=g++

NAIAD_INC_DIR  = -I$(NAIAD_PATH)/include/em
NAIAD_INC_DIR += -I$(NAIAD_PATH)/include/Nb
NAIAD_INC_DIR += -I$(NAIAD_PATH)/include/Ng
NAIAD_INC_DIR += -I$(NAIAD_PATH)/include/Ni
NAIAD_INC_DIR += -I$(NAIAD_PATH)/include/system

NAIAD_LDFLAGS  = -L$(NAIAD_PATH)/lib

NAIAD_LIBS     = -lNb
NAIAD_LIBS    += -lNi
NAIAD_LIBS    += -liomp5
NAIAD_LIBS    += -pthread			# gcc specific

TARGET         = emp2particle

$(TARGET): emp2particle.cc
	$(CXX) $(NAIAD_INC_DIR) -o $(TARGET) emp2particle.cc $(NAIAD_LDFLAGS) $(NAIAD_LIBS)

