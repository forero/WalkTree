#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tree.h"

int find_snap(float **scale_factor, float aexp, int nsnap){
  int index;
  int i;
  float delta;
  index = (int)(scale_factor[0][SNAP_ID]);
  delta = fabs(scale_factor[0][SNAP_AEXP]-aexp);

  for(i=0;i<nsnap;i++){
    if(fabs(scale_factor[i][SNAP_AEXP]-aexp)<delta){
      index = (int)(scale_factor[i][SNAP_ID]);
      delta = fabs(scale_factor[i][SNAP_AEXP]-aexp);
    }      
  }
  return index;
}



void load_scale_factors(char *filename, float ***scales, int *n_items){
  int n_lines;
  int c;
  FILE *in;
  int i;
  if(!(in=fopen(filename, "r"))){
    fprintf(stderr, "Problem opening file %s\n", filename);
    exit(1);
  }
  n_lines = 0;
  do{
    c = fgetc(in);
    if(c=='\n')
      n_lines++;
  }while(c!=EOF);
  *n_items = n_lines;
  rewind(in);
  
  if(!((*scales)=malloc(n_lines * sizeof(float *)))){
    fprintf(stderr, "problem with scale allocation");
    exit(1);
  }
  for(i=0;i<n_lines;i++){
    (*scales)[i] = malloc(2 * sizeof(float));
    fscanf(in, "%f %f\n", &((*scales)[i][0]), &((*scales)[i][1]));
  }
  fclose(in);
}

void load_filelist(char *filename, char ***list, int *n_items){
  int n_lines;
  int c;
  FILE *in;
  int i;
  if(!(in=fopen(filename, "r"))){
    fprintf(stderr, "Problem opening file %s\n", filename);
    exit(1);
  }
  n_lines=0;
  do{
    c = fgetc(in);
    if(c=='\n')
      n_lines++;
  }while(c!=EOF);
  *n_items = n_lines;
  rewind(in);

  /*Malloc the list*/
  if(!((*list)=malloc(n_lines * sizeof(char *)))){
    fprintf(stderr, "problem with list allocation\n");
    exit(1);
  }
  
  for(i=0;i<n_lines;i++){
    if(!((*list)[i]=malloc(MAXLEN * sizeof(char)))){
      fprintf(stderr, "problem with list allocation\n");
      exit(1);
    }
    fscanf(in, "%s", (*list)[i]);
  }
  

  fclose(in);
  return;
}


/* helper function to skip n lines */
void skipline(FILE* file,int n) {
  int i, j;
  char zeile[300];
  for(i=0;i<n;i++) {
    j=0;
    while((zeile[j] = fgetc(file))!='\n'){
      j++;
    }
  }  
  return;
}

/* helper function to count the nodes within */
/* a tree of the input file */
int countNodes(FILE* file) {	
  int i=0;
  char c;
  
  while(!feof(file)) {
    c=fgetc(file);
    if(c=='\n') {
      i++;
    }
    if(c=='#')
      break;
  }
  return i;
}

int read_ctrees(float **scale_factor_list, int nsnaps, char * filename, int * nOutputTrees, 
		struct nodeData*** nodeArray, int **totalNodes) {

  /* open the file obtained from consistent_trees */
  int j;
  FILE * file;
  int NNodes;
  float aexp;
  float dumb;
  int i_dumb;
  int i_aexp;
  file=fopen(filename,"r");
  if(file==0) {
    printf("Could not find file %s\n", filename);
    return 1;
  }
  int i_node;
  int i_snap;
  int *snap_halo;/*holds the snapshot number for a given node*/

  /* make a struct for the tree, as read in from the ctrees output  */
  /* this is necessary, as ctrees outputs a seperated tree for each */
  /* subhalo. Galacticus expects the subhalos to be within the tree */
  /* of their parent halo. */
  struct inputTreeData {
    int id;
    int nNodes;
    int mainNodeId;
    int parentId;
    int upId;
    int upmostId;
    int gTreeIndex;	/* index of the corresponding galacticus tree */
    int gNodeIndex;	/* nodeIndex within the corresponding galacticus tree */
    fpos_t startPos;
    fpos_t endPos;
  };

  /* get the number of data fields */
  /* for information purposes only */
  char line[800];
  int nfields=0;
  int i=0;
  while((line[i] = fgetc(file))!='\n') {
    i++;
  }
  strtok(line, " ");
  nfields=1;
  while(strtok(NULL," ")) {
    nfields++;
  }
  printf("%s has %i fields\n",filename,nfields);

  /* go to the beginning of the data section */
  /* ATTENTION! This may change with future  */
  /* versions of consistent_trees */
  skipline(file,24);

  /* get the number of trees in the ctrees output file */
  /* as subhalos have their own trees, this number     */
  /* will not correspond to the number of trees in the */
  /* galacticus input file */
  int nTreesInput;

  fscanf(file,"%i\n",&nTreesInput);

  printf("%s contains %i trees\n",filename,nTreesInput);
  *nOutputTrees = nTreesInput;

  /*Number of halos in a given snapshot*/
  if(!((*totalNodes) = malloc(nsnaps*sizeof(int)))){
      fprintf(stderr, "Problem with the node allocation\n");
      exit(1);
    }
  for(i=0;i<nsnaps;i++){
    (*totalNodes)[i] = 0;
  }

  int *totalNodesAux;
  /*Auxiliar counter of the total number of halos in a given snapshot*/
  if(!(totalNodesAux = malloc(nsnaps * sizeof(int)))){
    fprintf(stderr, "Proble  with the aux node allocation\n");
    exit(1);
  }  
  for(i=0;i<nsnaps;i++){
    totalNodesAux[i] = 0;
  }

  /*
    fill struct inputTreeData with the data from the input file 
    at the same time count on the number of halos per snapshot
   */
  NNodes = 0;
  struct inputTreeData *cTrees;
  if(!(cTrees=malloc(nTreesInput*sizeof(struct inputTreeData)))){
    fprintf(stdout, "Problem allocating cTrees\n");
    exit(1);
  }

  for(i=0; i<nTreesInput; i++) {
    cTrees[i].id=i;
    fscanf(file,"%*s %i\n",&cTrees[i].mainNodeId);
    fgetpos(file,&cTrees[i].startPos);
    fscanf(file,"%f %*i %*f %*i %*i %i %i", &aexp, &cTrees[i].parentId, &cTrees[i].upId);
    fsetpos(file, &cTrees[i].startPos);
    cTrees[i].nNodes = countNodes(file);
    fgetpos(file,&cTrees[i].endPos);    
    NNodes += cTrees[i].nNodes;
  }
  fprintf(stdout, "Total Number of nodes %i\n", NNodes);  
  if(!(snap_halo=malloc(NNodes * sizeof(int)))){
    fprintf(stderr, "Problem allocating snap_halo data\n");
    exit(1);
  }

  i_node=0;
  for(i=0;i<nTreesInput;i++){
    fsetpos(file,&cTrees[i].startPos); /* set file pointer to the start of tree i */
    for(j=0;j<cTrees[i].nNodes;j++){
      fscanf(file,"%f %*i %*f %*i %*i %i %i %*i %*i %*f %*f %*f %*f %*f %*i %*f %*f %*f %*f %*f %*f %*f %*f %*f %*f %*f %*f\n", 
	     &aexp, &cTrees[i].parentId, &cTrees[i].upId);
      i_aexp = find_snap(scale_factor_list, aexp, nsnaps);

      ((*totalNodes)[i_aexp])++ ;
      snap_halo[i_node] = i_aexp;
      i_node++;
    }
  }
  fprintf(stdout, "Finished getting cTrees data and count the number of nodes per snap\n");

  #ifdef DEBUG
  for(i=0;i<nsnaps;i++){
    fprintf(stdout, "snap %i nnodes %i\n", i, (*totalNodes)[i]);
  }
#endif


  /* allocate the memory for the node array */
  /* the node array has two dimensions, the */
  /* first for each tree, the second for    */
  /* the nodes within a tree */

  if(!(  (*nodeArray) = (struct nodeData**)malloc(sizeof(struct nodeData *)*nsnaps))){
    fprintf(stderr,"Problem with node alllocation\n");
    exit(1);
  }
  for(i=0;i<nsnaps;i++) {
    
    if(!((*nodeArray)[i] = (struct nodeData*)malloc(sizeof(struct nodeData)* (1+((*totalNodes)[i]))))){
      fprintf(stdout, "Problem with the nodeArray allocation (%i nodes) snap %i\n", (1+((*totalNodes)[i])), i);
      exit(1);
    }
  }

  i_node = 0;
  fprintf(stdout, "Started loading data\n");
  /* fill the node array */
  for(i=0;i<nTreesInput;i++) {   /* loop over the input trees */
    fsetpos(file,&cTrees[i].startPos); /* set file pointer to the start of tree i */
    for(j=0;j<cTrees[i].nNodes;j++) { /* loop over the node in each input tree */      
      i_snap = snap_halo[i_node];
      /*      fscanf(file,"%f",&(*nodeArray)[i_snap][totalNodesAux[i_snap]].scale);*/
      fscanf(file, "%f", &dumb);
      fscanf(file,"%i",&(*nodeArray)[i_snap][totalNodesAux[i_snap]].id);
      /*
            fscanf(file,"%f",&(*nodeArray)[i_snap][totalNodesAux[i_snap]].desc_scale);
      */
      fscanf(file, "%f", &dumb);
      /*
      fscanf(file,"%i",&(*nodeArray)[i_snap][totalNodesAux[i_snap]].desc_id);
      */
      fscanf(file, "%i", &i_dumb);
      /*      fscanf(file,"%i",&(*nodeArray)[i_snap][totalNodesAux[i_snap]].num_prog);*/
      fscanf(file, "%i", &i_dumb);
      fscanf(file,"%i",&(*nodeArray)[i_snap][totalNodesAux[i_snap]].pid);
      fscanf(file,"%i",&(*nodeArray)[i_snap][totalNodesAux[i_snap]].upid);
      /*
      fscanf(file,"%i",&(*nodeArray)[i_snap][totalNodesAux[i_snap]].desc_pid);
      */
      fscanf(file, "%i", &i_dumb);
      /*
      fscanf(file,"%i",&(*nodeArray)[i_snap][totalNodesAux[i_snap]].phantom);
      */
      fscanf(file, "%i", &i_dumb);

      fscanf(file,"%f",&(*nodeArray)[i_snap][totalNodesAux[i_snap]].mVir);
      /*
      fscanf(file,"%f",&(*nodeArray)[i_snap][totalNodesAux[i_snap]].mVirOrig);
      fscanf(file,"%f",&(*nodeArray)[i_snap][totalNodesAux[i_snap]].rVir);
      fscanf(file,"%f",&(*nodeArray)[i_snap][totalNodesAux[i_snap]].scaleRadius);
      fscanf(file,"%f",&(*nodeArray)[i_snap][totalNodesAux[i_snap]].vRMS);
      fscanf(file,"%i",&(*nodeArray)[i_snap][totalNodesAux[i_snap]].mmp);
      */
      fscanf(file, "%f", &dumb);
      fscanf(file, "%f", &dumb);
      fscanf(file, "%f", &dumb);
      fscanf(file, "%f", &dumb);
      fscanf(file, "%i", &i_dumb);
      /*
      fscanf(file,"%f",&(*nodeArray)[i_snap][totalNodesAux[i_snap]].scaleLastMM);
      fscanf(file,"%f",&(*nodeArray)[i_snap][totalNodesAux[i_snap]].vMax);
      fscanf(file,"%f",&(*nodeArray)[i_snap][totalNodesAux[i_snap]].pos[0]);
      fscanf(file,"%f",&(*nodeArray)[i_snap][totalNodesAux[i_snap]].pos[1]);
      fscanf(file,"%f",&(*nodeArray)[i_snap][totalNodesAux[i_snap]].pos[2]);
      fscanf(file,"%f",&(*nodeArray)[i_snap][totalNodesAux[i_snap]].v[0]);
      fscanf(file,"%f",&(*nodeArray)[i_snap][totalNodesAux[i_snap]].v[1]);
      fscanf(file,"%f",&(*nodeArray)[i_snap][totalNodesAux[i_snap]].v[2]);
      fscanf(file,"%f",&(*nodeArray)[i_snap][totalNodesAux[i_snap]].l[0]);
      fscanf(file,"%f",&(*nodeArray)[i_snap][totalNodesAux[i_snap]].l[1]);
      fscanf(file,"%f",&(*nodeArray)[i_snap][totalNodesAux[i_snap]].l[2]);
      fscanf(file,"%f",&(*nodeArray)[i_snap][totalNodesAux[i_snap]].spin);
      */
      fscanf(file, "%f", &dumb);
      fscanf(file, "%f", &dumb);
      fscanf(file, "%f", &dumb);
      fscanf(file, "%f", &dumb);
      fscanf(file, "%f", &dumb);
      fscanf(file, "%f", &dumb);
      fscanf(file, "%f", &dumb);
      fscanf(file, "%f", &dumb);
      fscanf(file, "%f", &dumb);
      fscanf(file, "%f", &dumb);
      fscanf(file, "%f", &dumb);
      fscanf(file, "%f", &dumb);
      fscanf(file,"\n");
      i_node++;
      totalNodesAux[i_snap]++;
    }
  }

  fclose(file);

  free(cTrees);
  free(snap_halo);
  fprintf(stdout, "Sinished loading data\n");
  return 0;

}



void dump_IDs(int my_file, char *filename_out, struct nodeData*** nodeArray, int *totalNodes, 
	      int nsnaps, int **UpMostID, int **FileUpMostID){
  FILE *out;
  int i,j,i_node,i_file, up_id;

  if(!(out=fopen(filename_out, "w"))){
    fprintf(stderr, "problem opening file %s\n", filename_out);
    exit(1);
  }

  for(i=0;i<nsnaps;i++){
    for(j=0;j<totalNodes[i];j++){
      i_file = FileUpMostID[i][j];
      i_node = UpMostID[i][j];
      up_id = nodeArray[i_file][i][i_node].id;
      fprintf(out, "%i %i %i %f\n", nodeArray[my_file][i][j].id, up_id, nodeArray[my_file][i][j].upid, nodeArray[my_file][i][j].mVir);
    }
  }

  fclose(out);
  return;
} 


  


