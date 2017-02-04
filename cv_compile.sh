[ ! -f $1 ] && exit

srcfile=$1

srcname=${1##*/};
g++ -O3 -fopenmp $srcfile -o ${srcname%.*} `pkg-config --cflags --libs opencv`
