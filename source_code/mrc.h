#ifndef __MRC_H__
#define __MRC_H__

#include <stdio.h>
#include <string.h>
#include <math.h>

#define SURRONDING_PIXEL 6
typedef struct _mrc_header *mrc_header;
typedef struct _mrc *mrc;
typedef struct _dimension dimension;

mrc_header read_mrc_header(FILE *in);
void print_mrc_header(mrc m);

mrc read_mrc(FILE *in);
void free_mrc(mrc m);
mrc_header get_mrc_header(mrc m);
float ***get_mrc_data(mrc m);
void write_mrc(FILE *out, mrc m);
dimension get_mrc_dimension(mrc m);

void print_mrc_data(mrc m,int i,int j, int k);

// 3D signal weakening for one point 
void signal_subtraction_around_mask_3D(mrc mask, mrc tomo,int a, int b, int c, float angle, float scalefactor, float background_value);
// extract sub volume, normal extraction, no angle information
// modify the data in sub_tomo
void extract_sub_volume(mrc tomo, mrc sub_tomo, int x, int y, int z);
void extract_sub_volume_with_mask(mrc mask,mrc tomo, mrc sub_tomo, int x, int y, int z);
void extract_sub_volume_with_angle_mask(mrc mask,mrc tomo, mrc sub_tomo, int x, int y, int z, float angle);
float estimate_background(FILE *in, mrc m);

// extract 2D image data with angle
void extract_2d_curve_data(mrc ref,mrc mask, mrc template,int X[], int Y[],float An[], int start, int end);
void subtract_repeat_signal(mrc ref, mrc mask, mrc template, int X[], int Y[], float An[],int start, int end, float sigma);

void subtract_repeat_signal_dm(mrc ref, mrc mask, mrc template, int X[], int Y[], float An[],int start, int end, float sigma, int search_edge_left, int search_edge_right);

void extract_3d_curve_data(mrc ref,mrc mask, mrc template,int X[], int Y[],int Z[], float An[], int start, int end);
void subtract_3D_repeat_signal(mrc ref, mrc mask, mrc template, int X[], int Y[], int Z[], float An[],int start, int end, float sigma);
void make_image_empty(mrc mrc_image);

//particle replacement
void particle_replacement(mrc m, mrc particle, int x, int y, int z, int f); 








#endif
