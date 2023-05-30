# tubulin-lattice signal subtraction
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
  

  

  
