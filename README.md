# Download #
git clone https://github.com/PengxinChai/multi-curve-fitting

git clone https://github.com/PengxinChai/tubulin-lattice-subtraction

# contact: 
pengxin.chai (at)yale.edu

# multi-curve fitting and tubulin-lattice signal subtraction

This manual is for MT singlet signal subtraction under cryoSPARC processing pipeline:

# Preprocessing

Run cryoSPARC or cryoSPARC Live or Relion to preprocess your data as usual. 

Pick MT particles as many as possible (8nm cutoff using template matching) and run two-three iterative 2D classification jobs to filter out junk particles. Make sure the MT is properly picked. 

Use MT templates Picking parametes: diameter set to 320Å distance ratio set to 0.25 (picked 8nm particles) extraction particles box size 512, downsample to 128 (bin4) Select MT 2D classes

Convert cryoSPARC file to RELION 3.0 Star format Navigate to the “2D classes selection job” in JXX and run the following command:

“csparc2star.py --swapxy particles_selected.cs JXX_passthrough_particles_selected.cs particles_selected.star”

This will create the “particles_selected.star” which will be used for multi-curve fitting. 

Navigate to the cryosparc session folder, for example, in “/PX/SX”

Make two folders called “ori_mics” and “sub_mics” “mkdir ori_mics sub_mics”

Move the micrographs from motioncorrecded folder into “ori_mics”: “mv motioncorredted/*doseweighted.mrc ori_mics”

Navigate into “ori_mics” folder: “cd ori_mics”

Copy the “particles_selected.star” in this folder. “cp ../../JXX/particles_oriselected.star .”

Recenter the “particles_selected.star”” file using the script from "multi-curve fitting":

“star_origin0_scale.com 4 particles_selected.star”

“4” is the scaling factor. This depends on the particle's extraction. If the box size is 512 and binned to 128, then the factor is “4”. If the box size is 512 and binned to 256, then the factor is “2”.

Split the “particles_selected_origin0.star” into into individual files using the following command:

“star_split_quick.py particles_selected_origin0.star *doseweighted.mrc”

Perform multi-curve fitting In the motion-corrected folder, run the following command:

“mcurve_fitting_2D.py -h” “mcurve_fitting_2D.py --pixel_size_ang 0.868 --poly_expon 2 XXX_split.star”

Check the results using relion_display. Make sure the MT-segments are properly sampled. 

Split each MT segment into smaller segments. It has been found that the subtraction is more clean when subtracting smaller segments for MT singlets. For MT doublet, this step is not necessary. It is because MT singlet is more heterogeneous(non-uniform diameter, helical twist) than MT doublet.

“split_segments.py XXX_split_resam_Zscore.txt”

This will generate small segments and each one has ~20 coordinates (~80nm/segment). The file end with “split_resam_Zscore_renumber.txt”

Prepare mask for MT signal subtraction Copy the mask into the folder:

“cp common_masks/XXX.mrc .”

# MT signal subtraction 

Copy subtraction script into the folder:

“cp mrc_2d_curve_weaken_one.sh .”

Edit the script using Vim or gedit to change the files names

Test the subtraction: “./mrc_2d_curve_weaken_one.sh XXX_doseweighted.mrc”

Check the results using IMOD or relion_display

Para run the subtraction: “para_run.py 32 ./mrc_2d_curve_weaken_one.sh XXX*_doseweighted.mrc”

Move subtracted micrographs into “sub_mics” folder 

Navigate to the session folder move the subtracted micrographs into sub_mics: “mv ori_mics/*sub.mrc sub_mics”

Change the name of subtracted micrographs: “rename sub.mrc .mrc sub_mics/*sub.mrc”

Link subtracted micrographs into motioncorrected micrographs: “cd motioncorrected” “ln -s ../sub_mics/*doseweighted.mrc .” “ln -s ../ori_mics/*doseweighted.mrc .” (because some micrographs don’t have MT for subtraction, we need to link these original micrographs for cryoSparc to run. )





### OLD ###
This folder contains the C code and documentation to perform tubulin lattice signal subtraction in cryo-EM images.

1. The introduction of the program:

  In the bin directory, you will find the C program to perform the actual signal subtration and a bash file to run the program.

  USAGE: mrc_2d_curve_weaken_dynamic_mask <mrc> <mask> <coords_file> <scale_factor> <left_search_start> <left_search_end>
  USAGE: mrc_2d_curve_weaken <mrc> <mask> <coords_file> <scale_factor> 

  Only support mrc file format and mrc file must end with .mrc.
  
  The coords file should have X, Y, angle and cluster information, 4-column txt file format.(From mcurve fitting results or other approaches.)
  
  The Y axis of mask should be the major axis of the filament. The length of the mask should be the length of the repeating unit, in pixel.
  
  The width of the mask should be a litter bigger than the biggest width of filament in pixels. You also need to specify the search range(only provide left half).
  
  If you are dealing with MT singlets or filaments with a similar diameter, use the "mrc_2d_curve_weaken" program. If you are dealing with MT doublets, use the "mrc_2d_curve_weaken_dynamic_mask" program. The search range is important. You can perform an initial trial for subtraction and visulize the averaged segments and then decide the search range:
 ![Slide1_crop](https://user-images.githubusercontent.com/83961552/145485780-d30d3a9f-c48a-456e-b81d-e321bad72a4b.jpg)
  
  Scale factor 0 means subtract all signals of repeating unit. 1 means no subtractoin. Usually set to 0. When testing the dynamic width result, can use negative values to visualize the mask width for each filament.

2. Test the program:
Download the files in TestData to you linux workstation.
  
Use the following command:
  
  mrc_2d_curve_weaken_dynamic_mask slot12_02441_F40_MC2_DW_shrink2.mrc MTD_mask_528_32.mrc slot12_02441_F40_MC2_DW_shrink2_MultiCurvesFit.txt 0 60 180

  
3. Visulize the results:
  
After running the program, when you open the original mrc file and the sub.mrc file, you should see something like this:
  ![image](https://user-images.githubusercontent.com/83961552/145240541-143ae9fc-c2ac-4499-a888-7d90518c007c.png)
The left is the original image, the right is the tubulin-lattice signal subtracted image.
  
4. Wait a minute! How to prepare the input files?

a. Coordinate file: We are essentially doing 2D averaging of the repeating units of each filament. That's to say, we need to know the particles positions(X,Y),in-plane rotation angles(psi) and the helical tube ID(cluster). You can use my multi-curves fitting program to prepare these coordinate files. However, as long as you can prepare the four column txt file(X,Y,Angle,Cluster), the program should be running.

b. Mask file: To perform the approatiate averaging and subtraction, we need a mask file whose lengh covers the repeating distance. The width of mask will be dynamically determined during the averaging. I prepared some common mask file for microtubule singlets and doublets with pixel size 1A. You can scale the mask file depending on your image's pixel size. 
  

  

  
