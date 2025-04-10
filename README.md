# MT-Signal Subtraction Manual for cryoSPARC Users

# Formatted by chatGPT

**Kai Zhang Lab, Yale**  
**Last updated: April 2025**

This manual details how to perform microtubule (MT) signal subtraction under the **cryoSPARC** processing pipeline. You will need some command-line experience to follow this workflow.

---

## 📦 Downloads

- **[pyem by asarnow](https://github.com/asarnow/pyem)**  
  Conversion tool between cryoSPARC and RELION formats.

- **[Multi-Curve Fitting Scripts](https://github.com/PengxinChai/multi-curve-fitting)**  
- **[Tubulin Lattice Subtraction Scripts](https://github.com/PengxinChai/tubulin-lattice-subtraction)**

Clone the scripts:

```bash
git clone https://github.com/PengxinChai/multi-curve-fitting
git clone https://github.com/PengxinChai/tubulin-lattice-subtraction
```

---

## 🔧 Preprocessing in cryoSPARC

1. Run cryoSPARC or cryoSPARC Live to preprocess your data.
2. Pick MT particles (template matching) and run 2–3 rounds of 2D classification.
   - Use MT templates
   - Diameter: `320Å`
   - Distance ratio: `0.25` (for 8nm particles)
   - Box size: `512`, downsample to `128` (bin4)
3. Select clean MT 2D classes.

---

## 🔁 Convert cryoSPARC to RELION `.star`

Navigate to the “2D classes selection job” in JXX and run the following command:

```bash
csparc2star.py --swapxy --relion2 particles_selected.cs JXX_passthrough_particles_selected.cs particles_selected.star
```

⚠️ Note: `--swapxy` and `--relion2` depend on your `pyem` version.  
Ensure the output `.star` file **does NOT contain optics groups**.

---

## 📁 Organize Micrographs

1. Navigate to the cryosparc session folder, for example, in “/PX/SX”. Or navigate to the motioncorrected job containing the micrographs.
2. Make two folders called “ori_mics” and “sub_mics”
3. Move the micrographs from motioncorrecded folder into “ori_mics”
4. Copy the “particles_selected.star” in this folder

```bash
mkdir ori_mics sub_mics
mv motioncorrected/*doseweighted.mrc ori_mics
cd ori_mics
cp ../../JXX/particles_selected.star .
```

---

## 🔄 Recenter Coordinates

```bash
star_origin0_scale.com 4 particles_selected.star
```

- Use `4` for bin4 (e.g. 512 → 128), `2` for bin2.

---

## 🧩 Split STAR per Micrograph

```bash
star_split.py particles_selected_origin0.star *doseweighted.mrc
```

---

## 🖼️ Check Coordinates with RELION

```bash
relion_display --pick   --i XXX_patch_aligned_doseweighted.mrc   --coords XXX_patch_aligned_doseweighted_split.star   --scale 0.3 --black 0 --white 0   --sigma_contrast 3 --particle_radius 46   --lowpass 20 --highpass 2000 --angpix 0.8
```

---

## 🧮 Multi-Curve Fitting

```bash
mcurve_fitting_2D.py --pixel_size_ang 0.868 --poly_expon 2 XXX_split.star
```

Verify with:

```bash
relion_display --pick   --i XXX_patch_aligned_doseweighted.mrc   --coords XXX_patch_aligned_doseweighted_split_resam_Zscore.star   --scale 0.3 --black 0 --white 0   --sigma_contrast 3 --particle_radius 46   --lowpass 20 --highpass 2000 --angpix 0.8 --color_label rlnParticleSelectZScore --red 0 --blue 10
```

---

## 🔍 Segment MTs into Small Pieces (Singlets Only)

It has been found that the subtraction is cleaner when subtracting smaller segments for MT singlets (https://www.biorxiv.org/content/10.1101/2022.08.17.504273v1). For MT doublet, this step is not necessary. It is because MT singlet is more heterogeneous (non-uniform diameter, helical twist) than MT doublet.

```bash
split_segments.py XXX_split_resam_Zscore.txt
```

➡️ Produces ~20 coords/segment (≈80nm).  
Output: `split_resam_Zscore_renumber.txt`

---

## 🎭 Prepare MT Mask

The mask needs to be rescaled to match the pixel size of the micrographs. For example, the following command rescaled the provided mask (1Å pixel size) to 0.8Å

```bash
cp common_masks/XXX.mrc .
relion_image_handler --i MT-250A_mask_angpix1A_box400X40.mrc   --o MT-250A_mask_angpix0.8A.mrc   --rescale_angpix 0.8
```

---

## 🧹 Subtract MT Signal (One image test)

Edit the "mrc_2d_curve_weaken_one.sh" script using Vim or gedit to change the files names and program paths.

```bash
cp mrc_2d_curve_weaken_one.sh .
chmod +x mrc_2d_curve_weaken_one.sh
./mrc_2d_curve_weaken_one.sh XXX_doseweighted.mrc
```

Check results using **IMOD** or **RELION**.

### Parallel Subtraction (Python 2 Required, large scale)

```bash
python2 para_run.py 32 ./mrc_2d_curve_weaken_one.sh XXX*_doseweighted.mrc
```

Or write your own bash script for batch processing.

---

## 📂 Finalize Subtracted Micrographs

```bash
mv ori_mics/*sub.mrc sub_mics
rename sub.mrc .mrc sub_mics/*sub.mrc
cd motioncorrected
ln -s ../sub_mics/*doseweighted.mrc .
ln -s ../ori_mics/*doseweighted.mrc .
```

⚠️ Linking original micrographs ensures cryoSPARC won’t fail due to missing MTs. Don’t worry about the warning messages during linking. 

---

## 🚀 Continue Processing in cryoSPARC

Use the new micrographs in your next cryoSPARC pipeline steps (e.g., re-extraction, refinement, etc).





---
---

### OLD ###
# Tubulin Lattice Signal Subtraction for Cryo-EM Images

This repository contains the C code and documentation to perform tubulin lattice signal subtraction in cryo-EM images.

## Program Overview

The provided C program can subtract tubulin lattice signals from cryo-EM images. There are two main programs:

- **`mrc_2d_curve_weaken_dynamic_mask`**: For processing microtubule doublets.
- **`mrc_2d_curve_weaken`**: For processing microtubule singlets or filaments with similar diameters.

## Usage

In the `bin` directory, you will find the compiled C programs and a bash script to run the program.

### Command Syntax

```bash
mrc_2d_curve_weaken_dynamic_mask <mrc> <mask> <coords_file> <scale_factor> <left_search_start> <left_search_end>
mrc_2d_curve_weaken <mrc> <mask> <coords_file> <scale_factor>
```

- **`<mrc>`**: Input MRC file (must end with `.mrc`).
- **`<mask>`**: Mask file for signal subtraction.
- **`<coords_file>`**: A text file with four columns: `X, Y, angle, cluster` (e.g., from multi-curve fitting or other methods).
- **`<scale_factor>`**: Controls the extent of signal subtraction (0 means full subtraction, 1 means no subtraction).
- **`<left_search_start>` / `<left_search_end>`**: Defines the search range for the dynamic mask (for doublets only).

### Mask Requirements

- **Y-axis** of the mask should align with the major axis of the filament.
- The **length** of the mask should match the repeating unit length (in pixels).
- The **width** of the mask should be slightly larger than the largest filament width in pixels.

### Choosing the Correct Program

- **Singlets/Similar Diameters**: Use `mrc_2d_curve_weaken`.
- **Doublets**: Use `mrc_2d_curve_weaken_dynamic_mask`.

You may need to experiment with the search range by visualizing the average segments after an initial trial to adjust for optimal subtraction.

## Test the Program

To test the program, download the example files from the `TestData` folder to your Linux workstation and run the following command:

```bash
mrc_2d_curve_weaken_dynamic_mask slot12_02441_F40_MC2_DW_shrink2.mrc MTD_mask_528_32.mrc slot12_02441_F40_MC2_DW_shrink2_MultiCurvesFit.txt 0 60 180
```

If you are dealing with MT singlets or filaments with a similar diameter, use the "mrc_2d_curve_weaken" program. If you are dealing with MT doublets, use the "mrc_2d_curve_weaken_dynamic_mask" program. The search range is important. You can perform an initial trial for subtraction and visulize the averaged segments and then decide the search range:
 ![Slide1_crop](https://user-images.githubusercontent.com/83961552/145485780-d30d3a9f-c48a-456e-b81d-e321bad72a4b.jpg)

## Visualize the Results

After running the program, open the original MRC file and the subtracted MRC file (`sub.mrc`). You should see a comparison like this:

![Original and Subtracted Images](https://user-images.githubusercontent.com/83961552/145240541-143ae9fc-c2ac-4499-a888-7d90518c007c.png)

- **Left**: Original image
- **Right**: Image after tubulin-lattice signal subtraction

## Preparing Input Files

### Coordinate File

To perform 2D averaging and signal subtraction, the program needs the coordinates of the repeating units in each filament. The coordinate file should be a 4-column text file with the following information:

- **X**: X position of the particle
- **Y**: Y position of the particle
- **Angle**: In-plane rotation angle (psi)
- **Cluster**: Helical tube ID (Cluster)

You can generate this file using the multi-curves fitting program or any other method that outputs the required format.

### Mask File

A mask file is needed for averaging and subtraction. It should cover the repeating distance of the microtubule and dynamically adjust during processing. We provide common mask files for microtubule singlets and doublets with a pixel size of 1 Å, but you can scale the mask according to the pixel size of your image.

---

This README should be structured for easy reference and use on GitHub. Let me know if you need any further adjustments!

  

  
