# Compiler
CXX = g++

# Check C++14 compatibility, eventually use C++11
CXX_STANDARD = $(shell $(CXX) -std=c++14 2>&1 >/dev/null | grep 'c++14' >/dev/null && echo -std=c++11 || echo -std=c++14)

# Flags
CPPFLAGS = $(CXX_STANDARD) -Wall -O3 -D NDEBUG -fthreadsafe-statics
LDFLAGS = -pthread

# Directories
SRCDIR = src
OBJDIR = obj
BINDIR = bin

# Files
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS	= $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
SHARED_OBJS = $(filter-out $(OBJDIR)/client.o $(OBJDIR)/server.o, $(OBJECTS))

.PNONY: all clean ipk-client ipk-server

################################################

# Build all
all: $(BINDIR)/ipk-server $(BINDIR)/ipk-client

# Build ipk-client
ipk-client:
$(BINDIR)/ipk-client: $(OBJDIR)/client.o $(SHARED_OBJS)
	mkdir -p $(BINDIR)
	$(CXX) -o $(BINDIR)/ipk-client $(OBJDIR)/client.o $(SHARED_OBJS) $(LDFLAGS)

# Build ipk-server
ipk-server:
$(BINDIR)/ipk-server: $(OBJDIR)/server.o $(SHARED_OBJS)
	mkdir -p bin
	$(CXX) -o $(BINDIR)/ipk-server $(OBJDIR)/server.o $(SHARED_OBJS) $(LDFLAGS)

# Compile all modules
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	mkdir -p $(OBJDIR)
	$(CXX) -c $(CPPFLAGS) $< -o $@


# Clean
clean:
	rm -f $(BINDIR)/ipk-server $(BINDIR)/ipk-client $(OBJECTS)
#	rm -rf $(BINDIR)/ $(OBJDIR)/
