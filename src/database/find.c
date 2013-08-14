#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> 
#include "tree.h"
#include "find.h"

void find_upmostID(struct nodeData*** nodeArray,int **nTotalNodes, int nsnaps, int nfiles_in, 
		   int ****UpMostID, int ****FileUpMostID){
  int i,j, k,n;
  int wanted;
  int up_id, file_up_id;

  fprintf(stderr, "Starting with finding\n");
  /*allocate and initialize the data structures for the output*/

  /*most up ID*/
  fprintf(stdout, "most up id-A\n");
  if(!((*UpMostID) = malloc(nfiles_in * sizeof(int **)))){
    fprintf(stderr, "Problem with UpMostID\n");
    exit(1);
  }
  fprintf(stdout, "most up id-B\n");
  for(i=0;i<nfiles_in;i++){
    if(!((*UpMostID)[i] = malloc(nsnaps * sizeof(int *)))){
      fprintf(stderr, "Problem with UpMostID\n");
      exit(1);
    }
    for(j=0;j<nsnaps;j++){
      if(!((*UpMostID)[i][j] = malloc(nTotalNodes[i][j] * sizeof(int)))){
	fprintf(stderr, "Problem with UpMostID\n");
	exit(1);
      }
      for(k=0;k<nTotalNodes[i][j];k++){
	(*UpMostID)[i][j][k] = -1;
      }
    }
  }
  fprintf(stdout, "File most up id-A\n");
  /*File most up ID*/
  if(!((*FileUpMostID) = malloc(nfiles_in * sizeof(int **)))){
    fprintf(stderr, "Problem with UpMostID\n");
    exit(1);
  }
  fprintf(stdout, "File most up id-B\n");
  for(i=0;i<nfiles_in;i++){
    fprintf(stdout, "File most up id-C -%d\n", i);
    if(!((*FileUpMostID)[i] = malloc(nsnaps * sizeof(int *)))){
      fprintf(stderr, "Problem with UpMostID\n");
      exit(1);
    }
    fprintf(stdout, "File most up id-D -%d\n", i);
    for(j=0;j<nsnaps;j++){
      if(!((*FileUpMostID)[i][j] = malloc(nTotalNodes[i][j] * sizeof(int)))){
	fprintf(stderr, "Problem with UpMostID\n");
	exit(1);
      }
    }
    for(k=0;k<nTotalNodes[i][j];k++){
      (*FileUpMostID)[i][j][k] = -1;
    }
    fprintf(stdout, "File most up id-E -%d\n", i);
  }

  fprintf(stdout, "Finished Initialization of UpMostData\n");
  /*Look for up most ID in case of a substructure*/
  for(i=0;i<1;i++){
    for(j=0;j<nsnaps;j++){
            fprintf(stdout, "File %i Snap %i\n", i, j);
      for(k=0;k<nTotalNodes[i][j];k++){

	if(nodeArray[i][j][k].upid!=-1){

	  wanted = nodeArray[i][j][k].upid;
	  n = find_upid(wanted, 
		      nodeArray, nTotalNodes, nsnaps, nfiles_in, i, j, k, &up_id, &file_up_id);
	  if(n!=-1){ 
	    fprintf(stderr, "search for upid failed, item [%i,%i,%i]\n", i, j, k);	    
	  }
	  //	fprintf(stdout, "Node %i out of %i [search ]\n", k, nTotalNodes[i][j]);
	}else{
	  up_id = k; //nodeArray[i][j][k].id;
	  file_up_id = i;
	}
	(*UpMostID)[i][j][k] = up_id;
	(*FileUpMostID)[i][j][k] = file_up_id;
	//	fprintf(stdout, "Node %i out of %i->up id %i\n", k, nTotalNodes[i][j], up_id);
      }
    }
  }
}

int find_upid(int wanted, 
	       struct nodeData*** nodeArray,int **nTotalNodes, int nsnaps, int nfiles_in, 
	       int my_file, int my_snap, int my_node,
	       int *up_id, int *file_up_id){
  int max_level;
  int i,k, level;
  int my_wanted;
  int most_up_id;

  most_up_id = wanted;
  max_level = 4;

  *file_up_id = my_file;
  //  *up_id = nodeArray[my_file][my_snap][my_node].upid;
  *up_id = my_node;


  my_wanted = wanted;
  for(level=0;level<max_level;level++){
      for(k=0;k<nTotalNodes[my_file][my_snap];k++){	
	if(my_wanted==nodeArray[my_file][my_snap][k].id){
	  wanted = nodeArray[my_file][my_snap][k].upid;
	  if(wanted==-1){
	    
	    //	    *up_id = my_wanted;
	    *up_id = k;
	    *file_up_id = my_file;
	    return wanted; 
	  }else{
	    my_wanted = wanted;
	  }	
      }
    }
  }

  /*if I am hsre, it means that up_id is in a different file*/
  for(level=0;level<max_level;level++){
    for(i=0;i<nfiles_in;i++){

	for(k=0;k<nTotalNodes[i][my_snap];k++){	
	  if(my_wanted==nodeArray[i][my_snap][k].id){
	    wanted = nodeArray[i][my_snap][k].upid;
	    if(wanted==-1){
	      //	      *up_id = my_wanted;
	      *up_id  = k;
	      *file_up_id = i;
	      return wanted;
	    }else{
	      my_wanted = wanted;
	    }
	  }
	}
      
    }
  }
  return wanted;
}


