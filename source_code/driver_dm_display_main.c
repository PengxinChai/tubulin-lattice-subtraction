#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include "mrc.h"

#define INITIAL_CAPACITY 10

struct _coord{
  int x;
  int y;
  int z;
  float angle;
  float angle_z;
  int cluster;
};

struct _coords{
  struct _coord *elements;
  int size;
  int capacity;
};

typedef struct _coords *coordinates;

coordinates read_coordinates(FILE *in);
coordinates read_coordinates_XYAZ(FILE *in);
void embiggen(coordinates coords);
void free_coords(coordinates coords);
void rmSubstr(char *str, const char *toRemove);

int main(int argc, char *argv[])
{
  // Processing command line 
  if (argc < 8)
  {
    fprintf(stderr, "\nUSAGE: %s <mrc> <mask> <coords_file> <scale_factor> <left_search_start> <left_search_end> <mrc>\n", argv[0]);
    fprintf(stderr, "\nOnly support mrc file format and mrc file must end with .mrc.\n");
    fprintf(stderr, "The coords file should have X, Y, angle and cluster information, 4-column txt file format.(From mcurve fitting results.)\n");
    fprintf(stderr, "The Y axis of mask should the major axis of the filament. The length of the mask should be the length of the repeating unit, in pixel.\n");
    fprintf(stderr, "The width of the mask should be a litter bigger than the biggest width of filament in pixels. You also need to specify the search range(only provide left half).\n");
    fprintf(stderr, "The search range is important. The left_search_end should always smaller than the half width of the mask. Also, the value (half-width - left_search_end) should be slightly bigger than the smallest half-width of the filament (in pixels).\n");
    fprintf(stderr, "Scale factor 0 means subtract all signals of repeating unit. 1 means no subtractoin. Usually set to 0. When testing the dynamic width result, can use negative values to visualize the mask width for each filament.\n");
    
    fprintf(stderr, "\nExample: %s Slot11_3_100_00004_F50_MC2_DW_shrink2.mrc MT_mask_520_32.mrc Slot11_3_100_00004_F50_MC2_DW_shrink2.txt 0 60 200\n", argv[0]);
    fprintf(stderr, "\nPlease email Pengxin Chai(pengxin.chai@yale.edu) from Dr. Kai Zhang lab at Yale MB&B for questions.\n\n");


    return 1;
  }

  // Read mask, tomo data, coords into memory
  FILE *mrc_path = fopen(argv[1],"rb");

  FILE *mask_path = fopen(argv[2],"rb");
  FILE *coords_path = fopen(argv[3],"r");
  char mrc_output[128];
  
  strcpy(mrc_output,argv[1]);  
  rmSubstr(mrc_output,".mrc");
  strcat(mrc_output,"_sub.mrc");  
  if (mask_path == NULL || mrc_path == NULL || coords_path == NULL){
    fprintf(stderr, "ERROR: Cannot open one of the files. Exit!\n");
    return 1;
  }

  FILE *output_path = fopen(mrc_output,"wb");
  float sigma=atof(argv[4]);
  int left_start=atoi(argv[5]);
  int left_end=atoi(argv[6]);
  FILE *mrc_dis_path = fopen(argv[7],"rb");
  printf("Preparing the data into memmory >>>>>>>>> %s\n",argv[1]);
  mrc mask=read_mrc(mask_path),template=read_mrc(mask_path),mrc_image=read_mrc(mrc_path);
  mrc mrc_dis=read_mrc(mrc_dis_path);
  coordinates coords=read_coordinates_XYAZ(coords_path);
  // clsoe files
  fclose(mrc_path);
  fclose(mask_path);
  fclose(coords_path);
  

  int X[coords->size],Y[coords->size];
  float A[coords->size];
  for (int i=0; i<coords->size; i++){
    X[i] = (int) coords->elements[i].x;
    Y[i] = (int) coords->elements[i].y;
    A[i] = coords->elements[i].angle;
  }

  printf("Total number of particles: %d\n",coords->size);
  printf("Start>>>>>>>>>>>>>>\n");

  char curve_output_name[128];
  char curve_num[3];
  int start=0,end=0, class=0;

  make_image_empty(mrc_dis);
  while(end < coords->size){
    if (coords->elements[end].cluster != coords->elements[start].cluster){
      printf("Process No.%d curve, start: %d, end: %d\n",class,start,end-1);
      
      extract_2d_curve_data(mrc_image,mask, template,X, Y, A, start, end-1);      
      subtract_repeat_signal_dm(mrc_image,mask, template,X,Y,A, start, end-1, sigma, left_start,left_end);
      subtract_repeat_signal_dm(mrc_dis,mask, template,X,Y,A, start, end-1, sigma, left_start,left_end);

      strcpy(curve_output_name,argv[1]);
      rmSubstr(curve_output_name,".mrc");
      strcat(curve_output_name,"_ave_Z");
      sprintf(curve_num,"%d",class);
      strcat(curve_output_name,curve_num);
      strcat(curve_output_name,".mrc");
      printf("Wrote: %s\n\n",curve_output_name);
      FILE *curve_out=fopen(curve_output_name,"wb");
      write_mrc(curve_out,template);
      fclose(curve_out);
      
      start = end;
      class++;      
    }else{
      end++;
    }
  }
  printf("Process No.%d curve, start: %d, end: %d\n",class,start,end-1);
  extract_2d_curve_data(mrc_image,mask, template,X, Y, A, start, end-1);
  subtract_repeat_signal_dm(mrc_image,mask, template,X,Y,A, start, end-1,sigma, left_start,left_end);
  subtract_repeat_signal_dm(mrc_dis,mask, template,X,Y,A, start, end-1, sigma, left_start,left_end);
  
  strcpy(curve_output_name,argv[1]);
  rmSubstr(curve_output_name,".mrc");
  strcat(curve_output_name,"_ave_Z");
  sprintf(curve_num,"%d",class);
  strcat(curve_output_name,curve_num);
  strcat(curve_output_name,".mrc");
  printf("Wrote: %s\n\n",curve_output_name);
  FILE *curve_out=fopen(curve_output_name,"wb");
  write_mrc(curve_out,template);
  fclose(curve_out);

  printf("Wrote: %s\n\n",mrc_output);
  write_mrc(output_path,mrc_dis); 
  //free structs 
  free_mrc(mask);
  free_mrc(template);
  free_mrc(mrc_image);
  free_mrc(mrc_dis);
  free_coords(coords);

  fclose(output_path);
  return 0;
}

coordinates read_coordinates(FILE *in){
  coordinates result = malloc(sizeof(struct _coords));
  result->size = 0;
  result->capacity = INITIAL_CAPACITY;
  result->elements = malloc(sizeof(struct _coord)*result->capacity);
  

  // Read data from txt file
  float x_tmp=0.0, y_tmp=0.0, z_tmp=0.0, angle=0.0, angle_z=0.0;
  int cluster=0;
  int i=0;
  while (fscanf(in,"%f",&x_tmp) != EOF){
    fscanf(in, "%f", &y_tmp);
    fscanf(in, "%f", &z_tmp);
    fscanf(in, "%f", &angle);
    fscanf(in, "%f", &angle_z);
    fscanf(in, "%d", &cluster);
   
    result->elements[i].x = (int)(x_tmp+0.5);
    result->elements[i].y = (int)(y_tmp+0.5);
    result->elements[i].z = (int)(z_tmp+0.5);
    result->elements[i].angle = angle;
    result->elements[i].angle_z = angle_z;
    result->elements[i].cluster = cluster;
    result->size++;
    i++;
    if (result->size == result->capacity){
      embiggen(result);
    }
  }
  return result;
}

coordinates read_coordinates_XYAZ(FILE *in){
  coordinates result = malloc(sizeof(struct _coords));
  result->size = 0;
  result->capacity = INITIAL_CAPACITY;
  result->elements = malloc(sizeof(struct _coord)*result->capacity);
  

  // Read data from txt file
  float x_tmp=0.0, y_tmp=0.0, angle=0.0;
  int cluster=0;
  int i=0;
  while (fscanf(in,"%f",&x_tmp) != EOF){
    fscanf(in, "%f", &y_tmp);
    fscanf(in, "%f", &angle);
    fscanf(in, "%d", &cluster);   
    result->elements[i].x = (int)(x_tmp+0.5);
    result->elements[i].y = (int)(y_tmp+0.5);
    result->elements[i].angle = angle;
    result->elements[i].cluster = cluster;
    result->size++;
    i++;
    if (result->size == result->capacity){
      embiggen(result);
    }
  }
  return result;
}

void embiggen(coordinates coords){
  coords->elements = realloc(coords->elements, sizeof(struct _coord)*coords->capacity*2);
  coords->capacity *=2;
}

void free_coords(coordinates coords){
  free(coords->elements);
  free(coords);
}


void rmSubstr(char *str, const char *toRemove)
{
    size_t length = strlen(toRemove);
    while((str = strstr(str, toRemove)))
    {
        memmove(str, str + length, 1 + strlen(str + length));
    }
}
