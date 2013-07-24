#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ID          0
#define UP_ID       1
#define MOST_UP_ID  2
#define NCOLS       3
#define USAGE "./sort_index.x path_in path_out"
#define MAXLEN 256
/*
  DESCRIPTION: For a list of indexes coming from merger trees split
  into different files, it finds the right file where a given subhalo
  should go in order to be in the same file with its parentFOF halo.

  DATE:
  Mon Dec 19 16:25:42 PST 2011

  AUTHOR:
  Jaime E. Forero-Romero / j.e.forero.romero@gmail.com
*/
int **read_index(char *filename, int *n_points);
int find_snap(int wanted, int ***full_data, int *n_points, int n_snaps, int my_snap);
int *find_destiny(int ***full_data, int *n_points, int n_snaps, int my_snap);
void dump_destiny(int ***data, int **final_destiny, int *n_points, int n_snaps, int my_snap, FILE *out);
int main(int argc, char **argv){
  char path_in[MAXLEN];
  char path_out[MAXLEN];
  char filename[MAXLEN];
  int i,j,k;
  int max_i, max_j, max_k;
  int ***data;
  int **final_destiny;
  int n_snaps;
  int *n_points;
  int i_snap;
  FILE *out;
  FILE *list;

  max_i = 5;
  max_j = 5;
  max_k = 5;
  if(argc!=3){
    fprintf(stderr, "USAGE: %s\n", USAGE);
    exit(1);
  }
  
  strcpy(path_in,  argv[1]);
  strcpy(path_out, argv[2]);
  
  n_snaps = max_i * max_j * max_k;
  fprintf(stdout, "the total number of snaps to load is %d\n", n_snaps);
  
  if(!(data=malloc(n_snaps * sizeof(int **)))){
    fprintf(stderr, "problem in data allocation");
    exit(1);
  }
  if(!(final_destiny=malloc(n_snaps * sizeof(int *)))){
    fprintf(stderr, "problem in data allocation\n");
    exit(1);
  }

  if(!(n_points=malloc(n_snaps * sizeof(int)))){
    fprintf(stderr, "problem in npoints allocation");
    exit(1);
  }

  i_snap = 0;
  for(i=0;i<max_i;i++){
    for(j=0;j<max_j;j++){
      for(k=0;k<max_k;k++){
	sprintf(filename, "%s/index_tree_%d_%d_%d.dat", path_in, i, j, k);
	fprintf(stdout, "%s\n", filename);
	data[i_snap] = read_index(filename, &(n_points[i_snap]));	
	i_snap++;
      }
    }
  }
  

  i_snap = 0;
  for(i=0;i<max_i;i++){
    for(j=0;j<max_j;j++){
      for(k=0;k<max_k;k++){
	fprintf(stdout, "cleaning snap %d\n", i_snap);
	final_destiny[i_snap] = find_destiny(data, n_points, n_snaps, i_snap);
	i_snap++;
      }
    }
  }
  
  
  sprintf(filename, "%s/index_list_filenames.dat", path_out);
  if(!(list=fopen(filename, "w"))){
    fprintf(stdout, "Problem opening file %s\n", filename);
    exit(1);
  }

  sprintf(filename, "%s/all_index_tree_snap_transfer.dat", path_out);
  if(!(out=fopen(filename, "w"))){
    fprintf(stdout, "Problem opening file %s\n", filename);
    exit(1);
  }


  i_snap = 0;
  for(i=0;i<max_i;i++){
    for(j=0;j<max_j;j++){
      for(k=0;k<max_k;k++){
	fprintf(stdout, "Dumping %d\n", i_snap);
	sprintf(filename, "%s/index_tree_%d_%d_%d.dat", path_in, i, j, k);
	fprintf(list, "%s\n", filename);
	dump_destiny(data, final_destiny, n_points, n_snaps, i_snap, out);
	i_snap++;
      }
    }
  }

  fclose(list);
  

  
  return 0;
}

void dump_destiny(int ***data, int **final_destiny, int *n_points, int n_snaps, int my_snap, FILE *out){
  int i;

  for(i=0;i<n_points[my_snap];i++){
    fprintf(out, "%i %i %i\n", data[my_snap][i][ID], my_snap, final_destiny[my_snap][i]);
  }

}

/*
  Find for all halos in file my_snap, the file where they should land.
  Main Halos stay where they are.
  Sub halos should land where the MOST_UP_ID halo is.
*/
int *find_destiny(int ***full_data, int *n_points, int n_snaps, int my_snap){
  int *final_snap;
  int i;
  int wanted;
  int destiny;


  /*allocate the final array holding the snapshot destiny*/
  if(!(final_snap=malloc(n_points[my_snap] * sizeof(int)))){
    fprintf(stderr, "Problem in final snap allocation\n");
    exit(1);
  }
  for(i=0;i<n_points[my_snap];i++){
    //    fprintf(stdout, "%d out of %d\n", i, n_points[my_snap]);
    destiny=-1;
    wanted = full_data[my_snap][i][MOST_UP_ID];
    /*If Main halo, stay where you are*/
    if(wanted==-1){
      final_snap[i]  = my_snap;
    }else{
      /*if not main halo, find your parent*/
      final_snap[i] = find_snap(wanted,full_data, n_points, n_snaps, my_snap);
    }
    if(final_snap[i]==-1){
      fprintf(stderr, "Halo %d in snap %d\n", full_data[my_snap][i][ID], my_snap);
      fprintf(stderr, "Could not find mostup ID %d\n", wanted);
      exit(1);
    }
  }
  return final_snap;
}
int find_snap(int wanted, int ***full_data, int *n_points, int n_snaps, int my_snap){
  int j, k;
  int destiny;
  int level;
  int my_wanted;
  int max_level;
  max_level=4;
  destiny = -1;
  
  for(level=0;level<max_level;level++){
    my_wanted = wanted;
    for(k=0;k<n_points[my_snap];k++){
      if(my_wanted == full_data[my_snap][k][ID]){
	wanted = full_data[my_snap][k][MOST_UP_ID];
	if(wanted==-1){
	  destiny = my_snap;
	  return destiny;
	}else{
	  my_wanted = wanted;
	}
      }
    }
  }

  if(level==max_level){
    my_wanted = wanted;	    

      for(level=0;level<max_level;level++){
	for(j=0;j<n_snaps;j++){
	  for(k=0;k<n_points[j];k++){
	    if(my_wanted == full_data[j][k][ID]){
	      wanted = full_data[j][k][MOST_UP_ID];
	      if(wanted==-1){
		destiny = j;
		return destiny;
	      }else{
		my_wanted = wanted;
	      }
	    }
	  }
	
      }
    }    
  }
  return destiny;
}

int **read_index(char *filename, int *n_points){
  int **list;
  int nlines;
  FILE *in;
  int c;
  if(!(in=fopen(filename, "r"))){
    fprintf(stderr, "Problem opening file %s\n", filename);
    exit(1);
  }
  nlines = 0;
  do{
    c = fgetc(in);
    if(c=='\n')
      nlines++;
  }while(c!=EOF);
  rewind(in);
  fprintf(stdout, "%d lines to read\n", nlines);
  *n_points = nlines;

  if(!(list=malloc(nlines * sizeof(int *)))){
    fprintf(stderr, "Problem in list allocation of %d lines\n", nlines);
    exit(1);
  }
  
  for(c=0;c<nlines;c++){
    if(!(list[c]=malloc(NCOLS * sizeof(int)))){
      fprintf(stderr, "Problem in allocation\n");
      exit(1);
    }
    fscanf(in, "%i %i %i\n", &(list[c][ID]), &(list[c][UP_ID]), &(list[c][MOST_UP_ID]));
  }
      
  return list;
}
