#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

#include "mrc.h"
#define BOLDBLACK   "\033[1m\033[37m"      /* Bold Black */
#define RESET   "\033[0m"

struct _dimension{
  size_t nx; // col number
  size_t ny; // row number
  size_t nz; // sec number
};



/**
 * mrc object contains a header and a 3D matrix data
 * 3D matrix data is float *** no matter the datatype is
 * need to convert the datatype to float
 * data[sec:Z][row:Y][col:X]
 */
struct _mrc{
  mrc_header hdr;
  float ***data;
};

/**
 * mrc header has 1024 bytes,see 2014 mrc file format 
 * https://www.ccpem.ac.uk/mrc_format/mrc2014.php
 */
struct _mrc_header{
  /**
   * col,row,sec number
   */
  int32_t nx; int32_t ny;  int32_t nz;

  /**
   * datatype
     0 8-bit signed integer (range -128 to 127)
     1 16-bit signed integer
     2 32-bit signed real
     6 16-bit unsigned integer
  */
  int32_t dtype;

  /**
   * unit cell parameters:
   * for EM image/volume, mx,my,mz = nx,ny,nz
   */
  int32_t nx_start;  int32_t ny_start;  int32_t nz_start;
  int32_t mx;  int32_t my;  int32_t mz;
  float mx_ang;  float my_ang;  float mz_ang;
  float angle_x;  float angle_y;  float angle_z;

  /**
   * axis information
   * 1->X
   * 2->Y
   * 3->Z
   */
  int32_t axis_c;  int32_t axis_r;  int32_t axis_s;

  /**
   * representative density values
   * the type is dynamically determined
   */
  void *dmin;  void *dmax;  void *dmean;

  /**
   * space group number
   */
  int32_t ispg;

  /**
   * extended header
   * not useful
   * nsymbt: size of extended number 0
   * extra_space: 0
   */
  int32_t nsymbt;  char extra_space[100];

  /**
   * phase origins
   * not useful
   */
  float phase_x;  float phase_y;  float phase_z;

  /**
   * file type and machine stamp
   * always 'MAP'
   */
  char file_type[4];  int32_t stamp;
  
  /**
   * rms
   */
  float rms;

  /**
   * notes
   */
  int32_t num_label;  char labels[10][80];
};

mrc_header read_mrc_header(FILE *in){
  mrc_header hdr = malloc(sizeof(struct _mrc_header));
  fseek(in,0,SEEK_SET);

  //nx,ny,nz
  fread(&hdr->nx,4,1,in);  fread(&hdr->ny,4,1,in);  fread(&hdr->nz,4,1,in);
  
  //dtype
  fread(&hdr->dtype,4,1,in);
  
  //unit cell
  fread(&hdr->nx_start,4,1,in);  fread(&hdr->ny_start,4,1,in);  fread(&hdr->nz_start,4,1,in);
  fread(&hdr->mx,4,1,in);  fread(&hdr->my,4,1,in);  fread(&hdr->mz,4,1,in);
  fread(&hdr->mx_ang,4,1,in);  fread(&hdr->my_ang,4,1,in);  fread(&hdr->mz_ang,4,1,in);
  fread(&hdr->angle_x,4,1,in);  fread(&hdr->angle_y,4,1,in);  fread(&hdr->angle_z,4,1,in);
  
  //axis
  fread(&hdr->axis_c,4,1,in);  fread(&hdr->axis_r,4,1,in);  fread(&hdr->axis_s,4,1,in);
  
  //some data values
  switch (hdr->dtype)
  {
    case 0:
      hdr->dmin = malloc(sizeof(int8_t));
      hdr->dmax = malloc(sizeof(int8_t));
      hdr->dmean = malloc(sizeof(int8_t));
      break;
    case 1:
      hdr->dmin = malloc(sizeof(int16_t));
      hdr->dmax = malloc(sizeof(int16_t));
      hdr->dmean = malloc(sizeof(int16_t));
      break;
    case 2:
      hdr->dmin = malloc(sizeof(float));
      hdr->dmax = malloc(sizeof(float));
      hdr->dmean = malloc(sizeof(float));
      break;
    case 6:
      hdr->dmin = malloc(sizeof(uint16_t));
      hdr->dmax = malloc(sizeof(uint16_t));
      hdr->dmean = malloc(sizeof(uint16_t));
      break;
    default:
      printf("ERROR: Unsupported data type\n");
      return NULL;
  }
  fread(hdr->dmin,4,1,in);  fread(hdr->dmax,4,1,in);  fread(hdr->dmean,4,1,in);
  
  //space group and extended header
  fread(&hdr->ispg,4,1,in);  fread(&hdr->nsymbt,4,1,in);  fread(hdr->extra_space,100,1,in);

  //phase origin
  fread(&hdr->phase_x,4,1,in);  fread(&hdr->phase_y,4,1,in);  fread(&hdr->phase_z,4,1,in);

  //file type and stamp
  fread(hdr->file_type,4,1,in);  fread(&hdr->stamp,4,1,in);
  hdr->file_type[3]='\0';

  //rms
  fread(&hdr->rms,4,1,in);

  //labels
  fread(&hdr->num_label,4,1,in);
  int i;
  for (i=0; i<hdr->num_label;i++){
    fread(hdr->labels[i],80,1,in);
    hdr->labels[i][strlen(hdr->labels[i])]='\0';
  }
                                                           
  return hdr;
}
                          
void print_mrc_header(mrc m){
  mrc_header hdr = m->hdr;
  //data shape
  printf(BOLDBLACK "image/volume shape is(x y z; columns,rows,sections): " RESET );
  printf("%d %d %d\n",hdr->nx,hdr->ny,hdr->nz);

  //data type
  printf(BOLDBLACK "datatype is: " RESET );
  printf("(mode %d) ",hdr->dtype);
  switch (hdr->dtype)
  {
    case 0:
      printf("8-bit signed integer\n");
      break;
    case 1:
      printf("16-bit signed integer\n");
      break;
    case 2:
      printf("32-bit signed real\n");
      break;
    case 6:
      printf("16-bit unsigned integer\n");
      break;
    default:
      printf("ERROR: Unsupported data type\n");
  }

  //unit cell
  printf(BOLDBLACK "unit cell parameters:\n" RESET );
  printf("start position:%d %d %d\n",hdr->nx_start,hdr->ny_start,hdr->nz_start);
  printf("dimensions in pixels(x y z): %d %d %d\n",hdr->mx,hdr->my,hdr->mz);
  printf("dimensions in angstroms(x y z): %.2f %.2f %.2f\n",hdr->mx_ang,hdr->my_ang,hdr->mz_ang);
  printf("pixel size(x y z): %.2f %.2f %.2f\n",hdr->mx_ang/hdr->mx,hdr->my_ang/hdr->my,hdr->mz_ang/hdr->mz);
  printf("angles in degree(x y z): %.2f %.2f %.2f\n",hdr->angle_x,hdr->angle_y,hdr->angle_z);

  
  //axis
  printf(BOLDBLACK "axis for col,row,sec; (1,2,3 for X,Y,Z): " RESET );
  printf("%d %d %d\n",hdr->axis_c,hdr->axis_r,hdr->axis_s);

  //data values
  printf(BOLDBLACK "density min max mean: " RESET );
  printf("%.3f %.3f %.3f\n",*((float *)(hdr->dmin)),*((float *)(hdr->dmax)),*((float *)(hdr->dmean)));
  //printf("%.3f %.3f %.3f\n",*(hdr->dmin),*(hdr->dmax),*(hdr->dmean));
  printf(BOLDBLACK "rms: " RESET );
  printf("%f\n",hdr->rms);

  //space group
  //printf(BOLDBLACK "space group; extender header size and content: " RESET );
  //printf("%d %d %d\n",hdr->ispg,hdr->nsymbt,atoi(hdr->extra_space));

  //phase
  printf(BOLDBLACK "phase origin(x y z): " RESET );
  printf("%.3f %.3f %.3f\n",hdr->phase_x,hdr->phase_y,hdr->phase_z);

  //notes
  printf(BOLDBLACK "number of labels: " RESET );
  printf("%d\n",hdr->num_label);
  for (int i=0; i<hdr->num_label;i++){
    printf(BOLDBLACK "labels[%d]: " RESET,i );
    printf("%s\n",hdr->labels[i]);
  }
}

mrc read_mrc(FILE *in){
  mrc m=malloc(sizeof(struct _mrc));
  m->hdr = read_mrc_header(in);
  int32_t num_sec=m->hdr->nz,num_col=m->hdr->nx,num_row=m->hdr->ny;

  m->data = malloc(sizeof(float **)*num_sec);
  for (int32_t s=0; s<num_sec; s++){
    m->data[s] = malloc(sizeof(float *)*num_row);
    for(int32_t r=0; r<num_row;r++){
      m->data[s][r] = malloc(sizeof(float)*num_col);
    }
  }

  //allocate space for tmp depending on the datatype
  size_t elt_size;
  void ***tmp;
  switch (m->hdr->dtype)
  {
    case 0:
      elt_size = sizeof(int8_t);
      tmp = malloc(sizeof(int8_t **)*num_sec);
      for (int32_t s=0; s<num_sec; s++){
        tmp[s] = malloc(sizeof(int8_t *)*num_row);
        for(int32_t r=0; r<num_row;r++){
          tmp[s][r] = malloc(sizeof(int8_t)*num_col);
        }
      }
      break;
    case 1:
      elt_size = sizeof(int16_t);
      tmp = malloc(sizeof(int16_t **)*num_sec);
      for (int32_t s=0; s<num_sec; s++){
        tmp[s] = malloc(sizeof(int16_t *)*num_row);
        for(int32_t r=0; r<num_row;r++){
          tmp[s][r] = malloc(sizeof(int16_t)*num_col);
        }
      }
      break;
    case 2:
      elt_size = sizeof(float);
      tmp = malloc(sizeof(float **)*num_sec);
      for (int32_t s=0; s<num_sec; s++){
        tmp[s] = malloc(sizeof(float *)*num_row);
        for(int32_t r=0; r<num_row;r++){
          tmp[s][r] = malloc(sizeof(float)*num_col);
        }
      }
      break;
    case 6:
      elt_size = sizeof(uint16_t);
      
      tmp = malloc(sizeof(uint16_t **)*num_row);
      for (int32_t r=0; r<num_row; r++){
        tmp[r] = malloc(sizeof(uint16_t *)*num_col);
        for(int32_t c=0; c<num_col;c++){
          tmp[r][c] = malloc(sizeof(uint16_t)*num_sec);
        }
      }
      
      break;
    default:
      printf("ERROR: Unsupported data type\n");
      return NULL;
  }

  //read data
  fseek(in,1024,SEEK_SET);
  //  printf("Current file pointer position: %lu; expected: 1024\n",ftell(in));
  
  for(int32_t s=0; s<num_sec;s++){
    for (int32_t r=0; r<num_row; r++){
      for(int32_t c=0; c<num_col;c++){      
        fread((char *)tmp[s][r]+c*elt_size, 4, 1 , in);
        m->data[s][r][c] =  * (float *)((char *)tmp[s][r]+c*elt_size);
        //printf("%.3f ", * (float *)((char *)tmp[s][r]+c*elt_size) );
        //printf("%.3f\n",m->data[s][r][c]);
      } 
    }
  }
  

  //free tmp
  for (int32_t s=0; s<num_sec; s++){
    for(int32_t r=0; r<num_row;r++){
      free(tmp[s][r]);      
    }
    free(tmp[s]);
  }
  free(tmp);
  
  return m;
}

void free_mrc(mrc m){
  free(m->hdr->dmin);
  free(m->hdr->dmax);
  free(m->hdr->dmean);
  int32_t num_sec=m->hdr->nz,num_row=m->hdr->ny;
  
  for (int32_t s=0; s<num_sec; s++){
    for(int32_t r=0; r<num_row;r++){
      free(m->data[s][r]);      
    }
    free(m->data[s]);
  }
  free(m->data);
  free(m->hdr);
  free(m);
}


mrc_header get_mrc_header(mrc m){
  return m->hdr;
}
float ***get_mrc_data(mrc m){
  return m->data;
}

void write_mrc(FILE *out, mrc m){
  //write header
  mrc_header hdr = m->hdr;
  float ***data = m->data;
  fseek(out,0,SEEK_SET);
  //printf("Current file pointer position: %lu; expected: 0\n",ftell(out));

  // data shape
  fwrite(&hdr->nx,4,1,out);fwrite(&hdr->ny,4,1,out);fwrite(&hdr->nz,4,1,out);

  // datatype always float 2
  hdr->dtype = 2;
  fwrite(&hdr->dtype,4,1,out);
  //printf("Current file pointer position: %lu; expected: 16\n",ftell(out));

  //unit cell
  fwrite(&hdr->nx_start,4,1,out); fwrite(&hdr->ny_start,4,1,out); fwrite(&hdr->nz_start,4,1,out);
  fwrite(&hdr->mx,4,1,out);  fwrite(&hdr->my,4,1,out);  fwrite(&hdr->mz,4,1,out);
  fwrite(&hdr->mx_ang,4,1,out);  fwrite(&hdr->my_ang,4,1,out);  fwrite(&hdr->mz_ang,4,1,out);
  fwrite(&hdr->angle_x,4,1,out);  fwrite(&hdr->angle_y,4,1,out);  fwrite(&hdr->angle_z,4,1,out);
  //printf("Current file pointer position: %lu; expected: 64\n",ftell(out));

  //axis
  fwrite(&hdr->axis_c,4,1,out);  fwrite(&hdr->axis_r,4,1,out);  fwrite(&hdr->axis_s,4,1,out);
  

  //some data values,need to recalculate
  fwrite(hdr->dmin,4,1,out);  fwrite(hdr->dmax,4,1,out);  fwrite(hdr->dmean,4,1,out);
  
  // space group
  fwrite(&hdr->ispg,4,1,out);  fwrite(&hdr->nsymbt,4,1,out);  fwrite(hdr->extra_space,100,1,out);
   //phase origin
  fwrite(&hdr->phase_x,4,1,out);  fwrite(&hdr->phase_y,4,1,out);  fwrite(&hdr->phase_z,4,1,out);
  //printf("Current file pointer position: %lu; expected: 208\n",ftell(out));
  
  //file type and stamp
  fwrite(hdr->file_type,4,1,out);  fwrite(&hdr->stamp,4,1,out);
  //printf("Current file pointer position: %lu; expected: 216\n",ftell(out));
  //rms,need to recalculate
  fwrite(&hdr->rms,4,1,out);

  //labels
  hdr->num_label = 1;
  char *tmp = "Pengxin Chai";
  for(int i=0; i<strlen(tmp);i++){
    hdr->labels[0][i] = tmp[i];
  }
  hdr->labels[0][strlen(tmp)]='\0';
  fwrite(&hdr->num_label,4,1,out);
  fwrite(hdr->labels[0],80,1,out);

  //data
  fseek(out,1024,SEEK_SET);
  int32_t num_sec=m->hdr->nz,num_col=m->hdr->nx,num_row=m->hdr->ny;
  for(int32_t s=0; s<num_sec;s++){
    for (int32_t r=0; r<num_row; r++){
      for(int32_t c=0; c<num_col;c++){      
        fwrite(data[s][r]+c,4,1,out);        
      }
    }
  }  
}

dimension get_mrc_dimension(mrc m){
  dimension result;
  result.nx = m->hdr->nx;
  result.ny = m->hdr->ny;
  result.nz = m->hdr->nz;
  return result;
}

float estimate_background(FILE *in, mrc m){
  float sum=0.0;
  int cnt=0;
  int c,r,s;
  while (fscanf(in,"%d",&c)!=EOF){
    fscanf(in,"%d",&r);
    fscanf(in,"%d",&s);
    for (int i=r-SURRONDING_PIXEL;i<r+SURRONDING_PIXEL;i++){
      for (int j=c-SURRONDING_PIXEL;j<c+SURRONDING_PIXEL;j++){
        for (int k=s-SURRONDING_PIXEL;k<s+SURRONDING_PIXEL;k++){
          if (i>0 && j>0 && k >0 && i <m->hdr->ny && j<m->hdr->nx && k < m->hdr->nz){
            sum += m->data[k][i][j];
            cnt++;            
          }
        }
      }
    }
  }
  return sum/cnt;
}

void signal_subtraction_around_mask_3D(mrc mask, mrc tomo,int x, int y, int z, float angle, float scalefactor, float background_value){
  dimension dimen=get_mrc_dimension(mask);
  dimension tomo_dimen = get_mrc_dimension(tomo);
  int row_num=dimen.ny, col_num=dimen.nx, sec_num=dimen.nz;
  int row_center = row_num/2,col_center = col_num/2, sec_center = sec_num/2;
  int new_x,new_y,new_z;
  float factor;
  float pi = 3.14159;
  float A = sin((angle)*pi/180);
  float B = cos((angle)*pi/180);
  
  
    for (int r=0; r<row_num; r++){
      for (int c=0; c<col_num; c++){
        new_x = (int)( (c-col_center)*A + (r-row_center)*B + x + 0.5);
        new_y = (int)( (r-row_center)*A - (c-col_center)*B + y + 0.5);
        for (int s=0; s<sec_num; s++){
          new_z = s - sec_center + z;             
          if ( 0<new_x && new_x < tomo_dimen.nx && 0<new_y && new_y < tomo_dimen.ny && 0<new_z && new_z< tomo_dimen.nz){      
            factor = (1-mask->data[s][r][c]*scalefactor);            
            tomo->data[new_z][new_y][new_x] = (tomo->data[new_z][new_y][new_x] - background_value)*factor + background_value;
          }
        }
      }
  }
}

void extract_sub_volume(mrc tomo, mrc sub_tomo, int x, int y, int z){
  x -=1;
  y -=1;
  z -=1;
  dimension dimen=get_mrc_dimension(sub_tomo);
  dimension tomo_dimen = get_mrc_dimension(tomo);
  int row_num=dimen.ny, col_num=dimen.nx, sec_num=dimen.nz;
  int row_center = row_num/2,col_center = col_num/2, sec_center = sec_num/2;
  int new_x, new_y, new_z;

  float sum=0,min=99999999,max=-99999999;
  float value;

  for (int r=0; r<row_num; r++){
    for (int c=0; c<col_num; c++){
      for (int s=0; s<sec_num;s++){
        new_y = r - row_center + y;
        new_x = c - col_center + x;
        new_z = s - sec_center + z;
        sub_tomo->data[s][r][c] = 0;
        if ( 0<new_x && new_x < tomo_dimen.nx && 0<new_y && new_y < tomo_dimen.ny && 0<new_z && new_z< tomo_dimen.nz){
          sub_tomo->data[s][r][c] = tomo->data[new_z][new_y][new_x];
        }
        value = sub_tomo->data[s][r][c];
        sum += value;
        if (value < min){
          min = value;
        }
        if (value > max){
          max = value;
        }        
      }
    }
  }
  *(float *)(sub_tomo->hdr->dmean) = sum/(row_num*col_num*sec_num);
  *(float *)(sub_tomo->hdr->dmax) =  max;
  *(float *)(sub_tomo->hdr->dmin) =  min;
  sub_tomo->hdr->rms = tomo->hdr->rms;
  
}

void extract_sub_volume_with_mask(mrc mask, mrc tomo, mrc sub_tomo, int x, int y, int z){
  x -=1;
  y -=1;
  z -=1;
  dimension dimen=get_mrc_dimension(sub_tomo);
  dimension tomo_dimen = get_mrc_dimension(tomo);
  int row_num=dimen.ny, col_num=dimen.nx, sec_num=dimen.nz;
  int row_center = row_num/2,col_center = col_num/2, sec_center = sec_num/2;
  int new_x, new_y, new_z;
  
  
  for (int r=0; r<row_num; r++){
    for (int c=0; c<col_num; c++){
      for (int s=0; s<sec_num;s++){
        new_y = r - row_center + y;
        new_x = c - col_center + x;
        new_z = s - sec_center + z;
        if ( 0<new_x && new_x < tomo_dimen.nx && 0<new_y && new_y < tomo_dimen.ny && 0<new_z && new_z< tomo_dimen.nz){
          
          sub_tomo->data[s][r][c] = tomo->data[new_z][new_y][new_x]*mask->data[s][r][c];
          
          
        }
        
      }
    }
  }
}

void extract_sub_volume_with_angle_mask(mrc mask,mrc tomo, mrc sub_tomo, int x, int y, int z, float angle){
  x -=1;
  y -=1;
  z -=1;
  dimension dimen=get_mrc_dimension(sub_tomo);
  dimension tomo_dimen = get_mrc_dimension(tomo);
  int row_num=dimen.ny, col_num=dimen.nx, sec_num=dimen.nz;
  int row_center = row_num/2,col_center = col_num/2, sec_center = sec_num/2;
  int new_x, new_y, new_z;

  float pi = 3.14159;
  float A = sin((angle)*pi/180);
  float B = cos((angle)*pi/180);

  *(float *)(sub_tomo->hdr->dmean) = *(float *)(tomo->hdr->dmean);
  *(float *)(sub_tomo->hdr->dmax) =  *(float *)(tomo->hdr->dmax);
  *(float *)(sub_tomo->hdr->dmin) =  *(float *)(tomo->hdr->dmin);
  sub_tomo->hdr->rms = tomo->hdr->rms;
  
  
  for (int r=0; r<row_num; r++){
    for (int c=0; c<col_num; c++){
      new_x = (int)( (c-col_center)*A + (r-row_center)*B + x + 0.5);
      new_y = (int)( (r-row_center)*A - (c-col_center)*B + y + 0.5);
      for (int s=0; s<sec_num;s++){      
        new_z = s - sec_center + z;
        
        sub_tomo->data[s][r][c] = 0;
        if ( 0<new_x && new_x < tomo_dimen.nx && 0<new_y && new_y < tomo_dimen.ny && 0<new_z && new_z< tomo_dimen.nz){
          sub_tomo->data[s][r][c] = mask->data[s][r][c]*tomo->data[new_z][new_y][new_x];
        }
        
      }
    }
  }
}

void print_mrc_data(mrc m,int r,int c, int s){
  printf("%f\n",m->data[s][r][c]); 
}

void extract_2d_curve_data(mrc ref,mrc mask, mrc template,int X[], int Y[],float An[], int start, int end){
  dimension dimen=get_mrc_dimension(template);
  dimension ref_dimen = get_mrc_dimension(ref);
  int row_num=dimen.ny, col_num=dimen.nx, sec_num=dimen.nz;
  int row_center = row_num/2,col_center = col_num/2;
  int new_x, new_y;
  int num_points=end-start+1;
  float value;

  float pi = 3.14159;
  float B,A,angle;
  int x,y;
  float max=-999,min=999,sum=0;

  // Initilize template data
  for (int r=0; r<row_num; r++){
      for (int c=0; c<col_num; c++){
        for (int s=0; s<sec_num;s++){
          template->data[s][r][c] = 0;
        }
      }
  }    

  // extract and add data
  for (int i=start; i <= end; i++){
    x = X[i]-1;
    y = Y[i]-1;
    angle = An[i];
 
    A = sin((angle)*pi/180);
    B = cos((angle)*pi/180);
    for (int r=0; r<row_num; r++){
      for (int c=0; c<col_num; c++){
        new_x = (int)( (c-col_center)*A + (r-row_center)*B + x + 0.5);
        new_y = (int)( (r-row_center)*A - (c-col_center)*B + y + 0.5);
        for (int s=0; s<sec_num;s++){                        
          if ( 0<new_x && new_x < ref_dimen.nx && 0<new_y && new_y < ref_dimen.ny ){
            value = ref->data[s][new_y][new_x];            
            template->data[s][r][c] +=value;
          }
	  else{
	    template->data[s][r][c] += *(float *)ref->hdr->dmean;
	  }
        }
      }
    }    
  }

  // average template data
  for (int r=0; r<row_num; r++){
      for (int c=0; c<col_num; c++){
        for (int s=0; s<sec_num;s++){
          
          template->data[s][r][c] /=num_points;
          sum +=template->data[s][r][c];
          if (template->data[s][r][c] > max){
            max = template->data[s][r][c];
          }
          if (template->data[s][r][c] < min){
            min = template->data[s][r][c];
          }
        }
      }
  }
  *(float *)template->hdr->dmax = max;
  *(float *)template->hdr->dmin = min;
  *(float *)template->hdr->dmean = sum/row_num/col_num/sec_num;
}


void extract_3d_curve_data(mrc ref,mrc mask, mrc template,int X[], int Y[],int Z[], float An[], int start, int end){
  dimension dimen=get_mrc_dimension(template);
  dimension ref_dimen = get_mrc_dimension(ref);
  int row_num=dimen.ny, col_num=dimen.nx, sec_num=dimen.nz;
  int row_center = row_num/2,col_center = col_num/2,sec_center = sec_num/2;
  int new_x, new_y, new_z;
  int num_points=end-start+1;
  float value;

  float pi = 3.14159;
  float B,A,angle;
  int x,y,z;
  float max=-999,min=999,sum=0;

  // Initilize template data
  for (int r=0; r<row_num; r++){
      for (int c=0; c<col_num; c++){
        for (int s=0; s<sec_num;s++){
          template->data[s][r][c] = 0;
        }
      }
  }    

  // extract and add data
  for (int i=start; i <= end; i++){
    x = X[i]-1;
    y = Y[i]-1;
    z = Z[i]-1;
    angle = An[i];
 
    A = sin((angle)*pi/180);
    B = cos((angle)*pi/180);
    for (int r=0; r<row_num; r++){
      for (int c=0; c<col_num; c++){
        new_x = (int)( (c-col_center)*A + (r-row_center)*B + x + 0.5);
        new_y = (int)( (r-row_center)*A - (c-col_center)*B + y + 0.5);
        for (int s=0; s<sec_num;s++){
	  new_z = (int) (s - sec_center + z + 0.5);
          if ( 0<new_x && new_x < ref_dimen.nx && 0<new_y && new_y < ref_dimen.ny && 0<new_z && new_z < ref_dimen.nz){
            value = ref->data[new_z][new_y][new_x];            
            template->data[s][r][c] +=value;
          }        
        }
      }
    }    
  }

  // average template data
  for (int r=0; r<row_num; r++){
      for (int c=0; c<col_num; c++){
        for (int s=0; s<sec_num;s++){
          
          template->data[s][r][c] /=num_points;
          sum +=template->data[s][r][c];
          if (template->data[s][r][c] > max){
            max = template->data[s][r][c];
          }
          if (template->data[s][r][c] < min){
            min = template->data[s][r][c];
          }
        }
      }
  }
  *(float *)template->hdr->dmax = max;
  *(float *)template->hdr->dmin = min;
  *(float *)template->hdr->dmean = sum/row_num/col_num/sec_num;
}

void subtract_repeat_signal(mrc ref, mrc mask, mrc template, int X[], int Y[], float An[],int start, int end, float sigma){
  dimension dimen=get_mrc_dimension(template);
  dimension ref_dimen = get_mrc_dimension(ref);
  int row_num=dimen.ny, col_num=dimen.nx, sec_num=dimen.nz;
  int row_center = row_num/2,col_center = col_num/2;
  int new_x, new_y;
  float value;

  float pi = 3.14159;
  float B,A,angle;
  int x,y;
  float back=0.0;
  int cnt=0;

  //local background estimation
  for (int r=0; r<row_num; r++){
      for (int c=0; c<col_num; c++){        
        for (int s=0; s<sec_num;s++){
	  if (mask->data[s][r][c]<0.00001){
	    back += template->data[s][r][c];
	    cnt++;
	  }
	}
      }
  }
  back /=cnt;
  printf("Local background value is estimated as:%f\n",back);
  
  // subtraction
  for (int i=start; i <= end; i++){
    x = X[i]-1;
    y = Y[i]-1;
    angle = An[i];
 
    A = sin((angle)*pi/180);
    B = cos((angle)*pi/180);
    for (int r=0; r<row_num; r++){
      for (int c=0; c<col_num; c++){
        new_x = (int)( (c-col_center)*A + (r-row_center)*B + x + 0.5);
        new_y = (int)( (r-row_center)*A - (c-col_center)*B + y + 0.5);
        for (int s=0; s<sec_num;s++){                        
          if ( 0<new_x && new_x < ref_dimen.nx && 0<new_y && new_y < ref_dimen.ny ){
	    
            value = template->data[s][r][c] - back;
	    value = mask->data[s][r][c]*value*(1-sigma);
            ref->data[s][new_y][new_x] -= value;	    
          }        
        }
      }
    }    
  }
}


// Dynamic mask version
void subtract_repeat_signal_dm(mrc ref, mrc mask, mrc template, int X[], int Y[], float An[],int start, int end, float sigma, int search_edge_left, int search_edge_right){
  dimension dimen=get_mrc_dimension(template);
  dimension ref_dimen = get_mrc_dimension(ref);
  int row_num=dimen.ny, col_num=dimen.nx, sec_num=dimen.nz;
  int row_center = row_num/2,col_center = col_num/2;
  int new_x, new_y;
  float value;

  float pi = 3.14159;
  float B,A,angle;
  int x,y;
  float back=0.0;
  int cnt=0;

  //dynamic mask estimation
  printf("Start to estimate the mask width:\n");
  int r_start=0,c_start=0,s_start=0;
  int r_end=row_num,c_end=col_num,s_end=sec_num;


  int extend=0;
  int extend_flag=1;
  
  int min_index;
  float min_value=999999.9;
  float dynamic_mask[sec_num][row_num][col_num]; // new mask
  for (int r=r_start; r<r_end; r++){
    for (int c=c_start; c<c_end; c++){        
      for (int s=s_start; s<s_end;s++){
	dynamic_mask[s][r][c] = 0;
	if (extend_flag){
	  if (mask->data[s][r][c] > 0.999){
	    extend=c;
	    extend_flag=0;
	  }
	}
      }
    }
  }
  extend += (int)(extend*0.3);
  printf("Hald search range in pixels: %d-%d\n",search_edge_left,search_edge_right);
  printf("Extend pixels: %d\n",extend);
  //line averageing
  float line[col_center];
  for (int c=0; c< col_center; c++){
    for (int r=0;r<row_num;r++){
      line[c] += template->data[0][r][c];
      line[c] += template->data[0][r][col_num - c -1];
    }
    line[c] /= row_num;
  }

  // min value search 
  for (int c=search_edge_left; c< search_edge_right;c++) {
    if (line[c] < min_value){
      min_index=c;
      min_value=line[c];
    }
  }

  if (min_index - extend <=0){
    min_index = 0;
  }else{
    min_index -= extend;
  }

  c_start = min_index;
  c_end -= min_index;
  printf("Estimated range and width of mask(in pixels): %d-%d %d\n",c_start,c_end,c_end - c_start);
  for (int r=r_start; r<r_end; r++){
      for (int c=min_index; c<col_center; c++){        
        for (int s=s_start; s<s_end;s++){
	  dynamic_mask[s][r][c] = mask->data[s][r][c-min_index];
	  dynamic_mask[s][r][col_num-c-1] = mask->data[s][r][c-min_index];
	}
      }
  }
  
  
  //local background estimation
  for (int r=r_start; r<r_end; r++){
      for (int c=min_index; c<c_end; c++){        
        for (int s=s_start; s<s_end;s++){
	  if (dynamic_mask[s][r][c]<0.00001){
	    back += template->data[s][r][c];
	    cnt++;
	  }
	}
      }
  }
  back /=cnt;
  printf("Local background value is estimated as:%f\n",back);
  
  // subtraction
  for (int i=start; i <= end; i++){
    x = X[i]-1;
    y = Y[i]-1;
    angle = An[i];
 
    A = sin((angle)*pi/180);
    B = cos((angle)*pi/180);
    for (int r=r_start; r<r_end; r++){
      for (int c=c_start; c<c_end; c++){
        new_x = (int)( (c-col_center)*A + (r-row_center)*B + x + 0.5);
        new_y = (int)( (r-row_center)*A - (c-col_center)*B + y + 0.5);
        for (int s=s_start; s<s_end;s++){                        
          if ( 0<new_x && new_x < ref_dimen.nx && 0<new_y && new_y < ref_dimen.ny ){
	    
            value = template->data[s][r][c] - back;
	    value = dynamic_mask[s][r][c]*value*(1-sigma);
            ref->data[s][new_y][new_x] -= value;
          }        
        }
      }
    }    
  }
}



void subtract_3D_repeat_signal(mrc ref, mrc mask, mrc template, int X[], int Y[], int Z[], float An[],int start, int end, float sigma){
  dimension dimen=get_mrc_dimension(template);
  dimension ref_dimen = get_mrc_dimension(ref);
  int row_num=dimen.ny, col_num=dimen.nx, sec_num=dimen.nz;
  int row_center = row_num/2,col_center = col_num/2, sec_center = sec_num/2;
  int new_x, new_y,new_z;
  float value;

  float pi = 3.14159;
  float B,A,angle;
  int x,y,z;
  float back=0.0;
  int cnt=0;

  //local background estimation
  for (int r=0; r<row_num; r++){
      for (int c=0; c<col_num; c++){        
        for (int s=0; s<sec_num;s++){
	  if (mask->data[s][r][c]<0.00001){
	    back += template->data[s][r][c];
	    cnt++;
	  }
	}
      }
  }
  back /=cnt;
  printf("Local background value is estimated as:%f\n",back);
  
  // subtraction
  for (int i=start; i <= end; i++){
    x = X[i]-1;
    y = Y[i]-1;
    z = Z[i]-1;
    angle = An[i];
 
    A = sin((angle)*pi/180);
    B = cos((angle)*pi/180);
    for (int r=0; r<row_num; r++){
      for (int c=0; c<col_num; c++){
	new_x = (int)( (c-col_center)*A + (r-row_center)*B + x + 0.5);
	new_y = (int)( (r-row_center)*A - (c-col_center)*B + y + 0.5);
        for (int s=0; s<sec_num;s++){	  	  	  
	  new_z = (int) (s - sec_center + z + 0.5);
	  if (0<new_x && new_x < ref_dimen.nx && 0<new_y && new_y < ref_dimen.ny && 0 < new_z && new_z < ref_dimen.ny){	    
	    value = template->data[s][r][c] - back;
	    value = mask->data[s][r][c]*value*(1-sigma);
	    ref->data[new_z][new_y][new_x] -= value;
	    
          }        
        }
      }
    }    
  }
}

void make_image_empty(mrc mrc_image){
  dimension dimen=get_mrc_dimension(mrc_image);
  int row_num=dimen.ny, col_num=dimen.nx, sec_num=dimen.nz;
  for (int r=0; r<row_num; r++){
      for (int c=0; c<col_num; c++){
        for (int s=0; s<sec_num;s++){	  	  	  
	  mrc_image->data[s][r][c]=0;
          }
      }
  }
}


void particle_replacement(mrc m, mrc particle, int x, int y, int z,int f){
  dimension ref_dimen = get_mrc_dimension(m);
  
  dimension dimen=get_mrc_dimension(particle);
  int row_num=dimen.ny, col_num=dimen.nx, sec_num=dimen.nz;
  int row_center = row_num/2,col_center = col_num/2, sec_center = sec_num/2;
  
  int new_x, new_y, new_z;
  for (int r=0; r<row_num; r++){
    for (int c=0; c<col_num; c++){
      for (int s=0; s<sec_num;s++){
	if (fabs(particle->data[s][r][c]) > 0.001){
	  new_y = y -row_center + r;
	  new_x = x - col_center + c;
	  new_z = z - sec_center + s;
	  if (new_y >0 && new_y < ref_dimen.ny && new_z > 0 && new_z < ref_dimen.nz && new_x > 0 && new_x < ref_dimen.nx) m->data[new_z][new_y][new_x] = f*particle->data[s][r][c];
  
	}
      }
    }
  }
  

  
}
