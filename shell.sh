if [ -z $1 ] || [ ! -d $1 ];then
    echo "Usage: $0 [image dir]"
    exit;
fi

if [ -d $html ];then 
    rm -rf html;
fi 
mkdir html;


imgdir=`readlink -f $1`
let count=0;
let index=0;
outfile=
for img in `ls -v $imgdir/|grep jpg`
do
    if [ $count -eq 0 ];then 
        outfile=html/image_${index}.html
        let index+=1;
        echo "<html><body><table>" > $outfile
    fi

    echo "<tr><td><img src=\"$imgdir/$img\" width=\"576\"/><p>$img</p></td><tr>" >> $outfile 
    let count+=1;

    if [ $count -eq 99 ];then
        echo "</table></body></html>" >> $outfile 
        let count=0;
    fi
done


