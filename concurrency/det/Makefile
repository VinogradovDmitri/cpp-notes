CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -pedantic -O3 -g -DDEBUG \
           -Wnrvo -Wpessimizing-move -Wredundant-move \
           -pthread
TARGET = det
OBJDIR = obj
SOURCES = main.cpp
OBJECTS = $(SOURCES:%.cpp=$(OBJDIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET) valgrind.txt helgrind.txt

run: $(TARGET) input.txt
	./$(TARGET)

valgrind: $(TARGET) input.txt
	valgrind  --log-file=valgrind.txt --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET) --quiet

test: $(TARGET) input.txt
	./$(TARGET) --test

helgrind: $(TARGET) input.txt
	valgrind --tool=helgrind --sim-hints=no-nptl-pthread-stackcache --log-file=helgrind.txt ./$(TARGET) --quiet

.PHONY: all clean run valgrind test helgrind 
