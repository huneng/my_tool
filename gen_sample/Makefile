OBJ = main.o tool.o 
DST = main

DEPS = -lGL -lGLEW -lglfw -O3 -lopencv_core -lopencv_imgproc -lopencv_highgui

all: $(DST)

$(DST): $(OBJ)
	g++ $^ -o $@ $(DEPS)

$(OBJ): %.o:%.cpp
	g++ -c $^ 

clean: 
	rm $(OBJ) $(DST)
