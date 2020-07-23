for f in /Users/danieleghisi/Documents/Max\ 7/Packages/ears/media/17088__beskhu__upright-piano-multisamples/*
do
	n=$(basename "$f" | sed 's/[0-9]*__beskhu__\([0-9][0-9]\)-[a-g]-*[0-9].aiff/\1/')
	echo $n
	sox "$f" /Users/danieleghisi/Documents/Max\ 7/Packages/ears/media/earstestpiano_$n.mp3 trim 0 4 fade 0 3 1
done