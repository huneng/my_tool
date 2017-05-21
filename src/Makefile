OBJ=face_tool.o tool.o
DEPS=-I/usr/local/include -I/usr/local/include/opencv -lopencv_core -lopencv_imgproc -lopencv_highgui
FLAGS=-O3 -fopenmp

DST= neg_patch get_face norm_face image_t blur_image color2gray extract_frame merge_show rotate_image rotate_image_color scale_image \
	show_list create_sample gen_time hashMatch

all:${DST}
	cp ${DST} ../

${DST}: %:%.cpp ${OBJ}
	g++ ${FLAGS} $^ -o $@ ${DEPS}

${OBJ}: %.o:%.cpp 
	g++ -c ${FLAGS} $^

clean:
	rm -f ${OBJ} ${DST}
