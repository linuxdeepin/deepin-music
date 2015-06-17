#!/bin/sh

normal_opacity=0
pressed_opacity=0
fpath=.
outputPathName=png
while  getopts  :n:p:f:o:  opt
do
    case  "$opt"   in
    n)
        normal_opacity=$OPTARG ;;
    p)
        pressed_opacity=$OPTARG ;;
    f)  
        fpath=$OPTARG;;
    o)
        outputPathName=$OPTARG;;
    *)  
        normal_opacity=0.8
        pressed_opacity=0
        fpath=.
        outputPathName=png;;
  esac
done

echo 'normal_opacity: '$normal_opacity
echo 'pressed_opacity: '$pressed_opacity
echo 'fpath: '$fpath
echo 'outputPathName: '$outputPathName

cd $fpath
rm -rf *.png
var=`find . -name "*.svg" `
rm -rf $outputPathName
mkdir $outputPathName
for svg in  ${var}
do
    flag='.normal'
    n1=${svg#*/}
    name=${n1%.*}$flag.png
    inkscape --export-background-opacity=$normal_opacity --export-png=$outputPathName/$name $svg
    flag='.normal'
    n1=${svg#*/}
    name=${n1%.*}$flag.png
    inkscape --export-background-opacity=$pressed_opacity --export-png=$outputPathName/$name $svg
done
