src = $(wildcard *.cpp shaders/shader.cpp)
obj = $(src:.cpp=.o)
LDFLAGS = -lGL -lGLEW -lsfml-graphics -lsfml-system -lsfml-window

animation: $(obj)
	g++ -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) animation
