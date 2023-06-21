#!/usr/bin/bash

file=$1 ## input mrc image ##

exec="/home/pc666/tubulin-lattice-subtraction/bin/mrc_2d_curve_weaken"
mask="MTD-450A_mask_angpix1.325A_box452X31.mrc"

coords_file=${file%%.mrc}_split_resam_Zscore.txt ## output from mcurve_fitting_2D.py ##

sigma=0


echo $exec $file $mask $coords_file  $sigma
$exec $file $mask $coords_file $sigma



## remove intermediate "Zave" files ##

rm ${file%%.mrc}_ave_Z*.mrc
