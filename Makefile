# make sure to install grpc, etc, and also `sudo pip3 install grpcio-tools` which helps nanopb
NANOPB_DIR = $(HOME)/workspace/nanopb

.PHONY: all clean

TAG = hello
LIB = lib$(TAG).a lib$(TAG).so
EXE = $(TAG)-client 

CC = clang
AR = ar
#CROSS_COMPILE = arm-none-eabi-
#CC = $(CROSS_COMPILE)gcc
#AR = $(CROSS_COMPILE)ar

CFLAGS :=
ARFLAGS :=
LDFLAGS :=
SOLDFLAGS :=

CFLAGS += -Wall -Wextra -Werror
CFLAGS += -O0 -g
ifeq ($(CROSS_COMPILE),arm-none-eabi-)
CFLAGS += -march=armv7e-m -mfloat-abi=softfp -msoft-float
else
CFLAGS += -fPIC
CFLAGS += -march=native -mtune=native
endif

CFLAGS += -I/usr/local/include
CFLAGS += -I$(NANOPB_DIR)/
CFLAGS += -I$(NANOPB_DIR)/extra

ARFLAGS += cr

LDFLAGS += -L$(NANOPB_DIR)
LDLIBS += -lnanopb

SOLDFLAGS += -shared -Wl,-soname,lib$(TAG).so
SOLDFLAGS += -Wl,--no-undefined

HDR :=
CSRC :=
COBJ :=

GEN_HDR :=
GEN_CSRC :=

HDR += $(shell find * -name '*.h')
CSRC += $(shell find * -name '*.c')

GEN_HDR += hello.pb.h
GEN_CSRC += hello.pb.c

GEN_HDR += hello.grpc.pb.h
GEN_CSRC += hello.grpc.pb.c

$(foreach _h,$(GEN_HDR),\
$(eval .PRECIOUS: $(_h)) \
)

$(foreach _c,$(GEN_CSRC),\
$(eval .PRECIOUS: $(_c)) \
)

HDR += $(GEN_HDR)
CSRC += $(GEN_CSRC)

COBJ += $(CSRC:.c=.o)

OBJ := $(COBJ)

all: $(LIB) $(EXE)

%.grpc.pb.h: %.proto
	protoc --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_c_plugin` $<

%.grpc.pb.c: %.grpc.pb.h
	touch $@

$(EXE): main.c $(LIB)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS) lib$(TAG).a

%.pb.h: %.proto
	$(NANOPB_DIR)/generator/nanopb_generator.py $<

%.pb.c: %.pb.h
	touch $@

%.o: %.c $(HDR)
	$(CC) $(CFLAGS) -o $@ -c $<

%.so: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SOLDFLAGS) -o $@ $^ $(LDLIBS)

%.a: $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

check:
	LD_LIBRARY_PATH=$(NANOPB_DIR) ./$(EXE)

clean:
	rm -f $(OBJ) $(GEN_CSRC) $(GEN_HDR) $(LIB) $(EXE)
