OUTDIR=bin
OBJ = main.o tool.o 
DST = $(OUTDIR)/3DGen

DEPS = -lGL -lGLEW -lglfw -O3 -lopencv_core -lopencv_imgproc -lopencv_highgui

all: outdir $(DST)

outdir:
	mkdir -p $(OUTDIR)

$(DST): $(OBJ)
	g++ $^ -o $@ $(DEPS)

$(OBJ): %.o:%.cpp
	g++ -c $^ 

clean: 
	rm $(OBJ) $(DST)
