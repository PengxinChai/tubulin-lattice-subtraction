#!/usr/bin/bash

file=$1 ##mrc file to be processed 
exec="mrc_2d_curve_weaken_dynamic_mask" ##program
mask="MTD_mask_528_32.mrc" ## mask
coords_file=${file%%.mrc}_MultiCurvesFit.txt ##coordinates file
sigma=0 ##scale factor
search_start=60 ##search range 
search_end=180 


echo $exec $file $mask $coords_file  $sigma $search_start $search_end
$exec $file $mask $coords_file $sigma $search_start $search_end
