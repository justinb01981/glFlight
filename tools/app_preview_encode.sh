FFMPEG=~/Applications/ffmpeg/ffmpeg
FRAMERATE=30
INFILE=$1
OUTHEIGHT=1080
OUTWIDTH=1920
ASPECT=`printf %.10f "$((($OUTWIDTH*100000)/$OUTHEIGHT))e-5"`
OUTFILE1=$1.output1.m4v;
OUTFILE2=${OUTFILE1}.output.m4v;
OUTFILE3=${OUTFILE1}.outputcrop.m4v;

echo $ASPECT;

$FFMPEG -i $INFILE -c:v libx264 -v:crf 16 -strict -2  $OUTFILE1;
$FFMPEG -i $OUTFILE1 -r $FRAMERATE -vf "scale=$OUTWIDTH:$OUTHEIGHT" -aspect $ASPECT -strict -2 $OUTFILE2;
