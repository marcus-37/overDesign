ifeq ($(OS),Windows_NT)
RM = del /Q
else
RM = rm -f
endif

CXX       = g++
CXXFLAGS  = -I./../SFML-3.1.0/include
LDFLAGS   = -L./../SFML-3.1.0/lib
LDLIBS    = -lsfml-graphics -lsfml-window -lsfml-system
TARGET    = theSnake
OBJS      = main.o snake.o

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $(TARGET)

main.o: main.cpp main.h snake.h render.h
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

snake.o: snake.cpp main.h
	$(CXX) $(CXXFLAGS) -c snake.cpp -o snake.o

clean:
	$(RM) *.o $(TARGET)*

