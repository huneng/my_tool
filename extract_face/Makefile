OBJ=face_tool.o tool.o
DEPS=-I/usr/local/include -I/usr/local/include/opencv -lopencv_core -lopencv_imgproc -lopencv_highgui
FLAGS=-O3

ODIR=bin
DST = $(ODIR)/main
	

all:outdir ${DST}

outdir:
	mkdir -p $(ODIR)

${DST}: main.cpp ${OBJ}
	g++ ${FLAGS} $^ -o $@ ${DEPS}

${OBJ}: %.o:%.cpp 
	g++ -c ${FLAGS} $^

clean:
	rm -f ${OBJ} ${DST}
