CXX = g++
CXXFLAGS = -std=c++20 -Wall -Werror -O3 -Wpedantic \
           -pthread -g -DDEBUG # -fsanitize=thread -fno-omit-frame-pointer

QUEUE_TYPE ?= 1
TARGET = logger_app
OBJDIR = obj
SOURCES = src/main.cpp
OBJECTS = $(addprefix $(OBJDIR)/, $(notdir $(SOURCES:.cpp=.o)))

CXXFLAGS += -DQUEUE_TYPE=$(QUEUE_TYPE) -Iinclude

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $^

$(OBJDIR)/%.o: src/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET) valgrind.txt helgrind.txt drd.txt .log

clean_log:
	rm -f .log

run: $(TARGET)
	@TSAN_OPTIONS="disable_memory_mapping_check=1" ./$(TARGET)

valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
	--log-file=valgrind.txt ./$(TARGET)

helgrind: $(TARGET)
	valgrind --tool=helgrind --sim-hints=no-nptl-pthread-stackcache --log-file=helgrind.txt ./$(TARGET)

drd: $(TARGET)
	valgrind --tool=drd --exclusive-threshold=10 --log-file=drd.txt ./$(TARGET)

check: $(TARGET)
	@make valgrind helgrind drd

.PHONY: all clean run valgrind helgrind drd check