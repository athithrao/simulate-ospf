HEADERS 		= Manager.h Router.h logger.h Graph.h
MANAGER_SRC		= Manager.cpp logger.cpp
ROUTER_SRC		= Router.cpp logger.cpp Graph.cpp
ManEXE       	= manager
RouEXE			= router
	
CC        		= g++
CC_FLAGS  		= -Wall -std=c++11 -O2 -pthread

################################################################################

# Linking step
all: $(HEADERS) $(MANAGER_SRC) $(ROUTER_SRC)
	$(CC) $(CC_FLAGS) $(ROUTER_SRC) -o $(RouEXE)
	$(CC) $(CC_FLAGS) $(MANAGER_SRC) -o $(ManEXE)

manager: $(HEADERS) $(MANAGER_SRC)
	$(CC) $(CC_FLAGS) $(MANAGER_SRC) -o $(ManEXE)

router: $(HEADERS) $(ROUTER_SRC)
	$(CC) $(CC_FLAGS) $(ROUTER_SRC) -o $(RouEXE)

# Clean the directory in order to later build from scratch
clean:
	rm -rf *.o $(EXE)
