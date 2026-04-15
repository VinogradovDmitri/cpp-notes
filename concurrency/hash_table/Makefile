CXX = g++
CXXFLAGS = -std=c++26 -Wall -Werror -O3 -Wpedantic \
		   -pthread -g -DDEBUG #-fsanitize=thread -fno-omit-frame-pointer
TARGET = p_unoreder_map
OBJDIR = obj
SOURCES = main.cpp
OBJECTS = $(SOURCES:%.cpp = $(OBJDIR):%.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET) valgrind.txt helgrind.txt drd.txt

run: $(TARGET)
	@TSAN_OPTIONS="disable_memory_mapping_check=1" ./$(TARGET)

valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --log-file=valgrind.txt ./$(TARGET) --quiet

helgrind: $(TARGET)
	valgrind --tool=helgrind --suppressions=valgrind.supp --sim-hints=no-nptl-pthread-stackcache --log-file=helgrind.txt ./$(TARGET) --quiet

drd: $(TARGET)
	valgrind --tool=drd --suppressions=valgrind.supp --exclusive-threshold=10 --log-file=drd.txt ./$(TARGET) --quiet

check: $(TARGET)
	make valgrind helgrind drd

.PHONY: all clean run valgrind test helgrind 
