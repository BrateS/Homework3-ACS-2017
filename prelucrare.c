#include <stdio.h>
#include <stdlib.h>
#include "bmp_header.h"
#include <string.h>
#include <math.h>
#define FILENAME_MAX_LENGTH 40
int get_length(char* bmp_file,char* length);
char* get_bmp_file(char* input_file);
int pixel_eq(pixel pixel1,pixel pixel2);
pixel** process_raw_img(pixel** img,char* input_file);
pixel** black_white_img(pixel** img,char* bmp_file);
void init_pixel(pixel* pixel, unsigned char r, unsigned char g,
                unsigned char b);
pixel** alloc_img(int width,int height);
pixel** init_img(char* bmp_file);
void print_img(pixel** img, int width, int height);
void free_img(pixel*** img, int height);
void write_bmp(pixel** biData,char* bmp_file,
		char* output_filename);
void apply_filter_img(pixel** img,char* bmp_file,int filter[3][3]
			,char* extension);
void apply_filters(pixel** biData,char* bmp_file);
int pixel_within_threshold(pixel pixel1,pixel pixel2,int threshold);
void write_bin(pixel** img,char* input_file);
void read_bin(char* input_file);
void add_to_positions(int line,int col,int* n,pos** positions);
void free_matrix(unsigned char ***mat, int n);
unsigned char **get_matrix(int n, int m);
int main(void){	
	char input_file[FILENAME_MAX_LENGTH]="input.txt";
	char* bmp_file=get_bmp_file(input_file);
	int width=get_length(bmp_file,"width");
	int height=get_length(bmp_file,"height");
	pixel **biData=init_img(bmp_file);
	pixel **bnw_biData=black_white_img(biData,bmp_file);
	apply_filters(bnw_biData,bmp_file);	
	pixel **procd_biData=process_raw_img(biData,input_file);
	write_bin(biData,input_file);
	read_bin(input_file);
	free_img(&biData,height);
	free_img(&procd_biData,height);
	free_img(&bnw_biData,height);
	return 0;	
}
void read_bin(char* input_file){
	int i,j;
	char bin_file[FILENAME_MAX_LENGTH];
	char* bmp_file=get_bmp_file(input_file);
	int width=get_length(bmp_file,"width");
	int height=get_length(bmp_file,"height");
	printf("%d, %d\n",width,height);
	pixel** img=alloc_img(width,height);
	pixel** reverse_img;
	FILE *in = fopen(input_file,"rt");
	if ((in = fopen(input_file, "rt")) == NULL) {
        	fprintf(stderr, "Can't open %s", input_file);
    		}
	for(i=0;i<3;i++){
		fscanf(in,"%s",bin_file);
	}
	in=fopen(bin_file,"rb");//TODO REPLACE
	bmp_fileheader file_h;
	bmp_infoheader info_h;
	fread(&file_h,sizeof(file_h),1,in);
	fread(&info_h,sizeof(info_h),1,in); 
	fseek(in, file_h.imageDataOffset, SEEK_SET);
	short x=-1,y=-1;
	pixel pixel;
	for(i=0;i <height ;i++){
		for(j=0;j<width;j++){
			if(x-1<i||j>y-1){
			fread(&x,sizeof(short),1,in);
			fread(&y,sizeof(short),1,in);
			fread(&pixel,sizeof(pixel),1,in);
			}
			init_pixel(&img[i][j],pixel.r,pixel.g,pixel.b);	
		}
	}
	reverse_img=process_raw_img(img,input_file);
        write_bmp(reverse_img,bmp_file,"decompressed.bmp");
        fclose(in);
        if(feof(in)){
                printf("Reached EOF(bin_file)!\n");
        }
	free_img(&img,height);
	free_img(&reverse_img,height);	
	
}
void check_adj_pixels(int width,int height,pixel** img,pixel pixel,
		      int threshold,int i,int j,int* n,
		      pos** positions,unsigned char** marked){
        if (i - 1 >= 0){
                if(pixel_within_threshold(img[i-1][j],img[i][j],threshold)==1
			&&marked[i-1][j]==0){

                        img[i-1][j]=pixel;
			marked[i-1][j]=1;
			(*n)++;
			(*positions)[(*n)].l=i-1;	
			(*positions)[(*n)].c=j;
                }
        }
        if (i + 1 <= height - 1){
                if(pixel_within_threshold(img[i+1][j],img[i][j],threshold)==1
			&&marked[i+1][j]==0){

                        img[i+1][j]=pixel;
			marked[i+1][j]=1;
			(*n)++;	
			(*positions)[(*n)].l=i+1;	
			(*positions)[(*n)].c=j;
                }
        }
        if (j - 1 >= 0){
                if(pixel_within_threshold(img[i][j-1],img[i][j],threshold)==1
			&&marked[i][j-1]==0){

                        img[i][j-1]=pixel;
			marked[i][j-1]=1;
			(*n)++;	
			(*positions)[(*n)].l=i;	
			(*positions)[(*n)].c=j-1;
                }
        }
        if (j + 1 <= width - 1) {
                if(pixel_within_threshold(img[i][j+1],img[i][j],threshold)==1
			&&marked[i][j+1]==0){

                        img[i][j+1]=pixel;
			marked[i][j+1]=1;
			(*n)++;	
			(*positions)[(*n)].l=i;	
			(*positions)[(*n)].c=j+1;
                }
        }
}
void write_bin(pixel** img,char* input_file){
        char bmp_file[FILENAME_MAX_LENGTH]="";
        char output_filename[FILENAME_MAX_LENGTH]="compressed.bin";
        int threshold=0;
        int i,j;
        FILE *in = fopen(input_file,"rt");
        if ((in = fopen(input_file, "rt")) == NULL) {
                fprintf(stderr, "Can't open %s", input_file);
                }
        fscanf(in,"%s",bmp_file);
        fscanf(in,"%d",&threshold);
        in=fopen(bmp_file,"rb");
        bmp_fileheader file_h;
        bmp_infoheader info_h;
        fread(&file_h,sizeof(file_h),1,in);
        fread(&info_h,sizeof(info_h),1,in);
        int width=info_h.width;
        int height=info_h.height;
        FILE *out;
        if ((out = fopen(output_filename, "wb")) == NULL) {
                fprintf(stderr, "Can't open %s\n", output_filename);
        }
        fwrite(&file_h,sizeof(file_h),1,out);
        fwrite(&info_h,sizeof(info_h),1,out);
        fseek(out, file_h.imageDataOffset, SEEK_SET);
        fclose(in);
	img=process_raw_img(img,input_file);
	for(i=0;i<height;i++){
		for(j=0;j<width;j++){
			unsigned char r,g,b;
			r=img[i][j].r;
			g=img[i][j].g;
			b=img[i][j].b;
			init_pixel(&(img[i][j]),r,g,b);	
		}
	}
	pos *positions=malloc((width*height)*sizeof(pos));
        unsigned char **marked=get_matrix(height,width);
        int zones=0;
        for(i=0;i<height;i++){
                for(j=0;j<width;j++){	
			if(marked[i][j]==0){	
				int n=1;//ultimul
				int counter=1;//primul
				positions[counter].l=i;
				positions[counter].c=j;
				marked[i][j]=1;
				while(counter<=n){
					int aux=n;
					int line=positions[counter].l;
					int col=positions[counter].c;	
					check_adj_pixels(width,height,
						img,img[i][j],
						threshold,line,col,
						&n,&positions,marked);
					//if(aux==n)printf("Niciun vecin adaugat.\n");
					//else printf("Am adaugat %d vecini.\n",n-aux);
				counter++;
				//if(line==height-1&&col==width-1)break;
				//	printf("n: %d counter %d\n",
				//		n,counter);
				//if(n>=(width*height))break;
				//printf("(%d, %d)\n",line,col);
				}
				zones++;
			}
		}
	}
	free(positions);	
	/*for(i=0;i<height;i++){
		for(j=0;j<width;j++){
			unsigned char r,g,b;
			r=img[i][j].b;
			g=img[i][j].g;
			b=img[i][j].r;
			pixel aux;
			aux.r=b;
			aux.b=g;
			aux.g=r;
			img[i][j]=aux;
		}

	}*/
	printf("Zones:%d\n",zones);
	for(i=0;i<height;i++){
		for(j=0;j<width;j++){
			if(i==0||j==0||i==height-1||j==width-1){
				j++;
				i++;
			//	printf("(%d, %d, %u, %u, %u)\n",
			//		aux-1,j,img[aux-1][j-1].r,
			//		img[aux-1][j-1].g,
			//		img[aux-1][j-1].b);	
				fwrite(&i,sizeof(short),1,out);
				fwrite(&j,sizeof(short),1,out);
				i--;
				j--;
				fwrite(&(img[i][j]),sizeof(pixel),1,out);
				//printf("%hu",img[i][j].r);
				//printf("BorderMatrix found!\n");	
			}else if((pixel_within_threshold(img[i][j],
	  			  img[i+1][j],0)==0)
			          || (pixel_within_threshold(img[i][j],
				  img[i][j+1],0)==0)){
				j++;
				i++;
			//	printf("(%d, %d, %u, %u, %u)",
			//	i,j,img[i-1][j-1].r,img[i-1][j-1].g,
			//	img[i-1][j-1].b);	
				fwrite(&i,sizeof(short),1,out);
				fwrite(&j,sizeof(short),1,out);
				j--;
				i--;
				fwrite(&(img[i][j]),sizeof(pixel),1,out);
				//	printf("Border found!\n");
			}else if((pixel_within_threshold(img[i][j],
	  			  img[i-1][j],0)==0)
			          || (pixel_within_threshold(img[i][j],
				  img[i][j-1],0)==0)){
				j++;
				i++;
			//	printf("(%d, %d, %u, %u, %u)",
			//	i,j,img[i-1][j-1].r,img[i-1][j-1].g,
			//	img[i-1][j-1].b);	
				fwrite(&i,sizeof(short),1,out);
				fwrite(&j,sizeof(short),1,out);
				j--;
				i--;
				fwrite(&(img[i][j]),sizeof(pixel),1,out);
				//	printf("Border found!\n");
			}//You could zone them here.


		}
	}
	printf("Threshold:%d\n",threshold);
	fclose(out);//WATCH OUT
}
unsigned char **get_matrix(int n, int m){
	int i, j;
	unsigned char **mat;

	mat = malloc(n * sizeof(unsigned char *));
	if (mat == NULL) {
		return NULL;
	}

	for (i = 0; i < n; i++) {
		mat[i] = calloc(m , sizeof(unsigned char));
		if (mat[i] == NULL) {
			for (j = 0; j < i; j++) {
				free(mat[j]);
				}	
	
				free(mat);
				return NULL;
			}
	}

	return mat;
}
void free_matrix(unsigned char ***mat, int n){
	int i;

	for (i = 0; i < n; i++) {
		free((*mat)[i]);
	}
	
	free(*mat);
	*mat = NULL;
}
int pixel_eq(pixel pixel1,pixel pixel2){
	if(pixel1.r==pixel2.r&&
	   pixel1.g==pixel2.g&&
	   pixel1.b==pixel2.b)return 1;
	else return 0;	
}
int pixel_within_threshold(pixel pixel1,pixel pixel2,int threshold){
	if(threshold==0){
		if(pixel_eq(pixel1,pixel2)==1)return 1;
	}else{
		if(abs(pixel1.r-pixel2.r)
		   		+(abs(pixel1.b-pixel2.b))
			   	+(abs(pixel1.g-pixel2.g))<=threshold)return 1;
		else return 0;
	}

}
pixel** process_raw_img(pixel** img,char* input_file){
	pixel **processed_img;
	char* bmp_file=get_bmp_file(input_file);
	int width=get_length(bmp_file,"width");
	int height=get_length(bmp_file,"height");
	int i,j;
	processed_img=alloc_img(width,height);
	for(i=height-1;i>=0;i--){
		for(j=0;j<width;j++){
			init_pixel(&(processed_img[height-i-1][j]),
			img[i][j].b,img[i][j].g,img[i][j].r);
		}
	}
	return processed_img;	
}	
void apply_filters(pixel** bnw_biData,char* bmp_file){
	int f1[3][3] = { {-1,-1,-1},
                        {-1,8,-1},
                        {-1,-1,-1}  };
	apply_filter_img(bnw_biData,bmp_file,f1,"_f1.bmp");
	int f2[3][3] = { {0,1,0},
                        {1,-4,1},
                        {0,1,0}  };
	apply_filter_img(bnw_biData,bmp_file,f2,"_f2.bmp");
	int f3[3][3] = { {1,0,-1},
                        {0,0,0},
                        {-1,0,1}  };
	apply_filter_img(bnw_biData,bmp_file,f3,"_f3.bmp");	
}
int get_length(char* bmp_file,char* length){
	FILE *in = fopen(bmp_file,"rb");
	if ((in = fopen(bmp_file, "rb")) == NULL) {
        	fprintf(stderr, "Can't open %s", bmp_file);
		exit(1);
    		}
	bmp_fileheader file_h;
	bmp_infoheader info_h;
	fread(&file_h,sizeof(file_h),1,in);
	fread(&info_h,sizeof(info_h),1,in);
	close(in);
	if(!strcmp(length,"width")){
		return info_h.width;
	}
	else if(!strcmp(length,"height")){
		return info_h.height;
	}
	else{
		fprintf(stderr, "Invalid length string.");
		fflush(stdout);
		return 0;
	}
}
char* get_bmp_file(char* input_file){
	char* bmp_file;
	bmp_file=malloc(FILENAME_MAX_LENGTH*sizeof(char));
	FILE *in = fopen(input_file,"rt");
	if ((in = fopen(input_file, "rt")) == NULL) {
        	fprintf(stderr, "Can't open %s", input_file);
    		}
	fscanf(in,"%s",bmp_file);
	close(in);
	return bmp_file;
}	
void apply_filter_img(pixel** img,char* bmp_file,int a[3][3]
		      ,char* extension){
	int i,j;
	int red=0,gre=0,blu=0;
	FILE *in = fopen(bmp_file,"rb");
	if ((in = fopen(bmp_file, "rb")) == NULL) {
        	fprintf(stderr, "Can't open %s", bmp_file);
    		}
	bmp_fileheader file_h;
	bmp_infoheader info_h;
	fread(&file_h,sizeof(file_h),1,in);
	fread(&info_h,sizeof(info_h),1,in);
	int width=info_h.width;
	int height=info_h.height;
	fclose(in);
	pixel **filter_img=alloc_img(width,height);
	for(i=0;i<height;i++){
		for(j=0;j<width;j++){
			red=0;
			blu=0;
			gre=0;
			if (i - 1 >= 0){
				red = red + img[i - 1][j].r*a[2][1];
				gre = gre + img[i - 1][j].g*a[2][1];
				blu = blu + img[i - 1][j].b*a[2][1];
			}
			if (i + 1 <= height - 1) {
				red = red + img[i + 1][j].r*a[0][1];
				gre = gre + img[i + 1][j].g*a[0][1];
				blu = blu + img[i + 1][j].b*a[0][1];
			}
			if (j - 1 >= 0) {
				red = red + img[i][j - 1].r*a[1][0];
				gre = gre + img[i][j - 1].g*a[1][0];
				blu = blu + img[i][j - 1].b*a[1][0];
			}
        		if (j + 1 <= width - 1) {
				red = red + img[i][j + 1].r*a[1][2];
				gre = gre + img[i][j + 1].g*a[1][2];
				blu = blu + img[i][j + 1].b*a[1][2];
			}	
			if (i - 1 >= 0 && j+1 <=width-1){
				red = red + img[i - 1][j+1].r*a[2][2];
				gre = gre + img[i - 1][j+1].g*a[2][2];
				blu = blu + img[i - 1][j+1].b*a[2][2];
			}	
			if (i + 1 <= height - 1	&& j-1>=0) {
				red = red + img[i + 1][j-1].r*a[0][0];
				gre = gre + img[i + 1][j-1].g*a[0][0];
				blu = blu + img[i + 1][j-1].b*a[0][0];
			}	
			if (j - 1 >= 0 && i-1 >=0) {
				red = red + img[i-1][j - 1].r*a[0][2];
				gre = gre + img[i-1][j - 1].g*a[0][2];
				blu = blu + img[i-1][j - 1].b*a[0][2];
			}
        		if (j + 1 <= width - 1 && i+1 <=height -1) {
				red = red + img[i+1][j + 1].r*a[2][0];
				gre = gre + img[i+1][j + 1].g*a[2][0];
				blu = blu + img[i+1][j + 1].b*a[2][0];
			}
			red =red + img[i][j].r*a[1][1];
			gre =gre + img[i][j].g*a[1][1];
			blu =blu + img[i][j].b*a[1][1];
			if(red<0)red=0;
			if(red>255)red=255;
			if(gre<0)gre=0;
			if(gre>255)gre=255;
			if(blu<0)blu=0;
			if(blu>255)blu=255;

          		init_pixel(&(filter_img)[i][j],red,gre,blu);
		}
	}
	char out_file[FILENAME_MAX_LENGTH]="";
	strncat(out_file,bmp_file, strlen(bmp_file)-4);
	strcat(out_file,extension);
	printf("%s\n",out_file);
	write_bmp(filter_img,bmp_file,out_file);
	free_img(&filter_img,height);
}
void write_bmp(pixel** biData,char* bmp_file,char* output_filename){
	int i,j;
	int width=get_length(bmp_file,"width");
	int height=get_length(bmp_file,"height");
	FILE *in=fopen(bmp_file,"rb");
	bmp_fileheader file_h;
	bmp_infoheader info_h;
	fread(&file_h,sizeof(file_h),1,in);
	fread(&info_h,sizeof(info_h),1,in);
    	FILE *out; 
	if ((out = fopen(output_filename, "wb")) == NULL) {
		fprintf(stderr, "Can't open %s\n", output_filename);
    	}
	fwrite(&file_h,sizeof(file_h),1,out);
	fwrite(&info_h,sizeof(info_h),1,out);
	fseek(out, file_h.imageDataOffset, SEEK_SET);
	int padding=0;
	if( (3*height)%4!=0 ){
		padding=(height*0)%4;	
	}
	for(i=0;i<height;i++){
		for(j=0;j<width;j++){
			fwrite(&biData[i][j],sizeof(pixel),1,out);
		}
		for(j=0;j<padding;j++){
			unsigned char pad=0;
			fwrite(&pad,sizeof(unsigned char),1,out);
		}	
	}
}
pixel** black_white_img(pixel** img,char* bmp_file){
	int i,j;
	int width=get_length(bmp_file,"width");
	int height=get_length(bmp_file,"height");
	pixel **black_white_img=alloc_img(width,height);
	for(i=0;i<height;i++){
		for(j=0;j<width;j++){
			int x=(img[i][j].r+img[i][j].b+img[i][j].g)/3;
			init_pixel(&(black_white_img[i][j]),x,x,x);
		}
	}
	char out_file[FILENAME_MAX_LENGTH]="";
	strncat(out_file,bmp_file, strlen(bmp_file)-4);
	strcat(out_file,"_black_white.bmp");
	printf("%s\n",out_file);
	write_bmp(black_white_img,bmp_file,out_file);
	return black_white_img;
}
void
free_img(pixel*** img, int height)
{
  int i;
  for (i = 0; i < height; i++) {
    free((*img)[i]);
  }
  free(*img);
  *img = NULL;
}
void
print_img(pixel** img, int width, int height)
{
  int i, j;
  printf("%d %d\n", width, height);
  for (i = 0; i < height; i++) {
    for (j = 0; j < width; j++) {
      printf("| %u %u %u |", img[i][j].r, img[i][j].g, img[i][j].b);
    }
    printf("\n");
  }
  fflush(stdout);
}
pixel** alloc_img(int width,int height){
	pixel** img;
	img = (pixel**)calloc(height, sizeof(pixel*));
	if (img == NULL)
		fprintf(stderr, "Allocation error.");
	int i, j;
	for (i = 0; i < height; i++) {
		img[i] = (pixel*)calloc(width, sizeof(pixel));
			if (img[i] == NULL) {
				for (j = 0; j < i; j++) {
					free(img[j]);
				}
				free(img);
				fprintf(stderr, "Allocation error.");
			}
	}
	return img;
}
pixel**
init_img(char* bmp_file)
{
	int i,j;
	FILE* in = fopen(bmp_file,"rb");
	if ((in = fopen(bmp_file, "rb")) == NULL) {
        	fprintf(stderr, "Can't open %s", bmp_file);
    		}
	bmp_fileheader file_h;
	bmp_infoheader info_h;
	fread(&file_h,sizeof(file_h),1,in);
	fread(&info_h,sizeof(info_h),1,in);
	printf("%c%c\n",file_h.fileMarker1,file_h.fileMarker2);
	printf("%d\n",info_h.biClrImportant);
	int width=info_h.width;
	int height=info_h.height;
	printf("%d, %d\n",width,height);
	pixel** img=alloc_img(width,height);
	int padding=0;
	if( (3*height)%4!=0 ){
		padding=(height*0)%4;	
	}
	printf("Padding :%d\n",padding);
	fseek(in, file_h.imageDataOffset, SEEK_SET);
	for(i=0;i <height ;i++){
		for(j=0;j<width;j++){
			fread(&img[i][j],sizeof(pixel),1,in);
		}
		for(j=0;j<padding;j++){
			unsigned char pad=0;
			fread(&pad,sizeof(unsigned char),1,in);
		}
	}
	fclose(in);
	if(feof(in))printf("Reached EOF(init_img)!\n");
	 return img;
}
void
init_pixel(pixel* pixel, unsigned char r, unsigned char g, unsigned char b)
{
  (*pixel).r = r;
  (*pixel).g = g;
  (*pixel).b = b;
}
