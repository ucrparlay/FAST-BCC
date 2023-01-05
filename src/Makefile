ifdef BREAKDOWN
BREAK = -DBREAKDOWN
endif

ifdef NLONG
INTN = -DNLONG
endif

ifdef ELONG
INTE = -DELONG
endif

ifdef N2LONG
INTN2 = -DN2LONG
endif

ifdef CLANG
CC = clang++
else
CC = g++
endif

CPPFLAGS = -std=c++17 -Wall -Wextra -Werror $(INTN) $(INTE) $(INTN2)

INCLUDE_PATH = -I../parlaylib/include/

ifdef CILKPLUS
CPPFLAGS += -DPARLAY_CILKPLUS -DCILK -fcilkplus
else ifdef OPENCILK
CPPFLAGS += -DPARLAY_OPENCILK -DCILK -fopencilk
else ifdef SERIAL
CPPFLAGS += -DPARLAY_SEQUENTIAL
else
CPPFLAGS += -pthread
endif

ifdef DEBUG
CPPFLAGS += -DDEBUG -Og
else
CPPFLAGS += -O3 -mcx16 -march=native
endif

ifdef STDALLOC
CPPFLAGS += -DPARLAY_USE_STD_ALLOC
endif


all: FAST_BCC hopcroft_tarjan tarjan_vishkin

FAST_BCC: BCC.h BCC.cpp spanning_forest.h connectivity.h sparse_table.h LDD.h graph.h
	$(CC) $(CPPFLAGS) $(INCLUDE_PATH) BCC.cpp -o FAST_BCC

hopcroft_tarjan: hopcroft_tarjan.cpp hopcroft_tarjan.h graph.h
	$(CC) $(CPPFLAGS) $(INCLUDE_PATH) hopcroft_tarjan.cpp -o hopcroft_tarjan

tarjan_vishkin: tarjan_vishkin.cpp tarjan_vishkin.h graph.h
	$(CC) $(CPPFLAGS) $(INCLUDE_PATH) tarjan_vishkin.cpp -o tarjan_vishkin

clean:
	rm FAST_BCC hopcroft_tarjan
