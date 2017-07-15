OBJ=face_tool.o tool.o base.o
DEPS=-I/usr/local/include -I/usr/local/include/opencv -lopencv_core -lopencv_imgproc -lopencv_highgui
FLAGS=-O3 -fopenmp

DST= neg_patch get_face norm_face extract_frame merge_show rotate_image rotate_image_color \
	 gen_time hashMatch annot_rect selectImage transform annot extract_face

all:${DST}
	mkdir -p ../bin
	cp ${DST} ../bin

${DST}: %:%.cpp ${OBJ}
	g++ ${FLAGS} $^ -o $@ ${DEPS}

${OBJ}: %.o:%.cpp 
	g++ -c ${FLAGS} $^

clean:
	rm -f ${OBJ} ${DST}
