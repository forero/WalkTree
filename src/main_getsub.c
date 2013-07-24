#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "tree.h"
#include "io.h"
#include "find.h"
/*
  DESCRIPTION: 
  - For a list of consistentTrees files, finds the 
  ID of the upmost progenitor of each node (UpMostID). 
  - For each separate halo (UpMostID=-1) finds the list of substructures
  belonging to it.
  - Ranks the halos inside each UpMostID.
  - Links each halo to the next halo in the substructure list.
  - Dumps the Rank and NextID for each node of each tree in the 
  same number of input consistentTree files.

  CREATION DATE:
  Fri Feb 10 12:46:07 PST 2012

  UPDATES:

  AUTHOR:
  Jaime E. Forero-Romero
  j.e.forero.romero@gmail.com

*/

#define USAGE "./getsub.x ctrees_file_list output_file_list redshift_list"
int main(int argc, char **argv){
  /*IO filenames*/
  char filenamein[MAXLEN];
  char filenameout[MAXLEN];
  char redshift_list_filename[MAXLEN];
  char **ctrees_filename_in =NULL;
  char **filename_out =NULL;
  
  /*Raw data*/
  int nfiles_in;
  int nfiles_out;
  struct nodeData ***DataIn; /*raw consistentTreesData*/
  int *nTrees;
  int **nTotalNodes;
  float **scale_factors;
  int nsnaps; /*number of snapshots*/

  /*Processed data*/
  struct nodeData ***NewDataIn; /*reshaped input data*/

  int **nNodeSnap; /*Number of in a file at a given snapshot*/

  /*Final data*/
  int ***UpMostID; /*ID of the UpMost host halo. -1 if main halo*/
  int ***FileUpMostID; /*filenumber of the UpMostID files*/
  int ***RawSubLists; /*raw list holding the substructure lists*/
  int **RankUpMost; /*Rank inside the UpMost host halo, starts at 0 from main halo*/
  int **NextInUpMost; /*Link to the next halo inside the UpMost halo.
			-1 if last subhalo or devoided of substructure
		       A main halo points to the most massive substructure*/

  int i,n,j;
  if(argc!=4){
    fprintf(stderr, "USAGE: %s\n", USAGE);
    exit(1);
  }
  
  /*Load Filenames*/
  strcpy(filenamein, argv[1]);
  strcpy(filenameout, argv[2]);
  strcpy(redshift_list_filename, argv[3]);
  fprintf(stdout, "List filein %s\n", filenamein);
  fprintf(stdout, "List fileout %s\n", filenameout);
  fprintf(stdout, "List with the redshift %s\n", redshift_list_filename);

  /*load list of treefiles and output files*/
  load_filelist(filenamein, &(ctrees_filename_in), &nfiles_in);
  load_filelist(filenameout, &(filename_out), &nfiles_out);  
  
  fprintf(stdout, "%i files to load from %s\n", nfiles_in, filenamein);
  fprintf(stdout, "%i files to write from %s\n", nfiles_out, filenameout);
  


  
#ifdef DEBUG
  for(i=0;i<nfiles_in;i++){
    fprintf(stdout, "%s\n", ctrees_filename_in[i]);
  }
#endif

  //  nfiles_in = 2;
  //  nfiles_out = 2;
  
  /*load scale factor data*/
  load_scale_factors(redshift_list_filename, &scale_factors, &nsnaps);
#ifdef DEBUG
  //  for(i=0;i<nsnaps;i++){
  //    fprintf(stdout, "snap aexp: %f %f\n", scale_factors[i][SNAP_ID], scale_factors[i][SNAP_AEXP]);
  //  }
#endif

  /*Load Raw Data from the consistent trees*/
  if(!(DataIn = malloc(nfiles_in * sizeof(struct nodeData**)))){
    exit(1);
  }
  if(!(nTrees = malloc(nfiles_in * sizeof(int)))){
    exit(1);
}
  if(!(nTotalNodes = malloc(nfiles_in * sizeof(int **)))){
    exit(1);
}
  for(i=0;i<nfiles_in;i++){
    n = read_ctrees(scale_factors, nsnaps, 
		    ctrees_filename_in[i], &(nTrees[i]), &(DataIn[i]), &(nTotalNodes[i]));    
  }
  fprintf(stdout, "The full data has been loaded\n");

  /*Split the raw data into an array that loops ver [file][snapshot][node]*/


  /*Find the UpMostID for each node*/
  if(!(UpMostID = malloc(nfiles_in * sizeof(int **)))){
    fprintf(stderr, "problem with upmostid allocation\n");
    exit(1);
  }
  

  find_upmostID(DataIn, nTotalNodes, nsnaps, nfiles_in, &(UpMostID), &(FileUpMostID));
  
  for(i=0;i<nfiles_in;i++){
    for(j=0;j<nsnaps;j++){
      fprintf(stdout, "File %i Snap %i [Nodes %i]\n", i, j, nTotalNodes[i][j])    ;
    }
  }


  for(i=0;i<1;i++){
    dump_IDs(i, filename_out[i], DataIn, nTotalNodes[i], nsnaps, UpMostID[i], FileUpMostID[i]);
  }

  /*Create substructure list*/

  /*Rank halos in substructure list*/

  /*Fill the linked list of substructures*/

  /*Dump rank and nextID*/
  return 0;
}
