reset

### png
set terminal pngcairo size 1280, 1280 enhanced font 'Verdana,10'
set output './plt/heatmap.png'

unset key

#### border
set style line 11 lc rgb '#808080' lt 1
#set border 3 front ls 11
set tics nomirror out scale 0.75

### Colorbar
### disable colorbar tics
#set cbtics scale 0

### matlab palette colors
# set palette defined ( 0 "#000090",\
#                       1 "#000fff",\
#                       2 "#0090ff",\
#                       3 "#0fffee",\
#                       4 "#90ff70",\
#                       5 "#ffee00",\
#                       6 "#ff7000",\
#                       7 "#ee0000",\
#                       8 "#7f0000")
set palette defined (0 '#ffffcc', 0.25 '#c2e699', 0.5 '#78c679', 0.75 '#31a354', 1 '#006837')


set xrange [0:1280]
set yrange [1280:0] ##这里反转了y轴，一边图与数据一一对应。
set xlabel 'x'
set ylabel 'y'
set cblabel "counts"

plot 'heatmap.dat' matrix with image palette