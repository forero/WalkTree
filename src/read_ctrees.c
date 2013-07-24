/* Converter for Peter Behroozis merger tree */
/* Markus Haider 17.08.2011                  */
/* Check whether units are all set right!    */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "galacticus_tree.h"

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

int read_ctrees(char *, struct treeData **, int *, struct nodeData ***, int *);
int read_ctrees(char * filename, struct treeData ** galacticusTrees, int * nOutputTrees,
		struct nodeData*** nodeArray, int * totalNodes) {
  /*  */
  /* open the file obtained from consistent_trees */
  FILE * file;
  file=fopen(filename,"r");
  if(file==0) {
    printf("Could not find file %s\n", filename);
    return 1;
  }

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


  /* fill struct inputTreeData with the data from the input file */
  struct inputTreeData cTrees[nTreesInput];
  for(i=0; i<nTreesInput; i++) {
    cTrees[i].id=i;
    fscanf(file,"%*s %i\n",&cTrees[i].mainNodeId);
    fgetpos(file,&cTrees[i].startPos);
    fscanf(file,"%*f %*i %*f %*i %*i %i %i", &cTrees[i].parentId, &cTrees[i].upId);
    fsetpos(file, &cTrees[i].startPos);
    cTrees[i].nNodes = countNodes(file);
    fgetpos(file,&cTrees[i].endPos);
  }

    int nTrees;
  nTrees = 0;
  /* get the number of trees (without subhalo trees) */
  for(i=0; i<nTreesInput; i++) {
    if(cTrees[i].parentId==-1)
      nTrees++;
  }
  printf("%s contains %i distinct trees\n", filename,nTrees);

  /* make the array of galacticus trees */
  (*galacticusTrees) = malloc(nTrees*sizeof(struct treeData));

  if((*galacticusTrees)==NULL){
    fprintf(stdout, "Problem with the allocation\n");
    exit(1);
}

  /* give the galacticus trees an id */
  for(i=0; i<nTrees; i++) {
    (*galacticusTrees)[i].id=i;
  }

  /* fill the galacticus tree struct     */
  /* with the non-subhalo tree data from */
  /* the input file */
  int j=0;
  for(i=0; i<nTreesInput; i++) {
    if(cTrees[i].parentId==-1) { /* true if cTree is a non subhalo tree */
      (*galacticusTrees)[j].mainNodeId=cTrees[i].mainNodeId;
      (*galacticusTrees)[j].nNodes=cTrees[i].nNodes;
      cTrees[i].gTreeIndex=j;
      cTrees[i].gNodeIndex=0;
      j++;
    }
  }


  for(i=0; i<nTreesInput;i++) {
    cTrees[i].upmostId=cTrees[i].upId;
  }
  /* Get upmost Id, because a subhalo can have an upId         */
  /* which is not a distinct halo if the subhalo lies out      */
  /* of the virial radius of the host halo of the upId halo    */
  /* therefore make a loop to determine the really upmost halo */
  /* attention, several levels might be necessary              */

  int levels;
  int n_changed;
  int top_found;
  n_changed=0;
  for(levels=0; levels<4; levels++) {

    for(i=0; i<nTreesInput; i++) {
    top_found =0;
#ifdef PATCH
      if(cTrees[i].upmostId!=-1) {
#else
      if(cTrees[i].upId!=-1) {
#endif
	for(j=0; j<nTreesInput; j++) {
	  if(cTrees[i].upId==cTrees[j].mainNodeId) {
	    top_found=1;
	    if(cTrees[j].upId!=-1) {
	      cTrees[i].upmostId=cTrees[j].upId;
	      n_changed++;
	    }
	  }
	}
      
#ifdef PATCH
      if(top_found==0){
	fprintf(stderr, "upmostId for itree %i not found\n",i);
	exit(1);
      }
#endif
      }
    }

      
  }

    fprintf(stdout, "FInished the upmost ID calculation: %d changes\n",n_changed);


  /* correct the nNodes field for subhalo trees belonging */
  /* to this galacticus tree */
  for(j=0; j<nTrees; j++) {
    for(i=0; i<nTreesInput; i++) {
      if(cTrees[i].upmostId==(*galacticusTrees)[j].mainNodeId) {
	cTrees[i].gNodeIndex=(*galacticusTrees)[j].nNodes;
	(*galacticusTrees)[j].nNodes+=cTrees[i].nNodes;
      }
    }
  }
  fprintf(stdout, "Fnished correction 1\n");
  /* set the galacticus tree index in the inputTreeData struct */
  for(i=0; i<nTreesInput; i++) {
    if(cTrees[i].parentId!=-1) { /* true if tree is a subhalo tree */
      for(j=0; j<nTrees; j++) {
	if((*galacticusTrees)[j].mainNodeId==cTrees[i].upmostId)
	  cTrees[i].gTreeIndex=j;
      }
    }

  }
  fprintf(stdout, "Fnished correction 2\n");

  /* set the first node field for the galacticus tree data */
  /* needed, for galacticus to know, where a tree starts   */
  /* in the node data array */
  (*galacticusTrees)[0].firstNode=0;
  for(i=1; i<nTrees; i++) {
    (*galacticusTrees)[i].firstNode=(*galacticusTrees)[i-1].firstNode+(*galacticusTrees)[i-1].nNodes;
  }
  fprintf(stdout, "Fnished correction 3\n");

  /* allocate the memory for the node array */
  /* the node array has two dimensions, the */
  /* first for each tree, the second for    */
  /* the nodes within a tree */
  (*nodeArray) = (struct nodeData**)malloc(sizeof(struct nodeData *)*nTrees);
  if((*nodeArray)==NULL){
    fprintf(stderr,"Problem with node alllocation\n");
    exit(1);
  }
  for(i=0;i<nTrees;i++) {
    (*nodeArray)[i] = (struct nodeData*)malloc(sizeof(struct nodeData)*(*galacticusTrees)[i].nNodes);
    if((*nodeArray)[i]==NULL){
      fprintf(stdout, "Problem with the nodeArray allocation (%i nodes)\n", (*galacticusTrees)[i].nNodes);
      exit(1);
    }
  }
  fprintf(stdout, "Fnished allocation 3\n");
  /* fill the node array */
  for(i=0;i<nTreesInput;i++) {   /* loop over the input trees */
    /*
    fprintf(stdout, "%i %i [%i,%i]\n", i, cTrees[i].nNodes, 
	    cTrees[i].gTreeIndex, cTrees[i].gNodeIndex);
    */
    fsetpos(file,&cTrees[i].startPos); /* set file pointer to the start of tree i */
    for(j=0;j<cTrees[i].nNodes;j++) { /* loop over the node in each input tree */
      //      fprintf(stdout,"%i %i %f\n",j,cTrees[i].nNodes,&(*nodeArray)[cTrees[i].gTreeIndex][cTrees[i].gNodeIndex+j].scale);
      fscanf(file,"%f",&(*nodeArray)[cTrees[i].gTreeIndex][cTrees[i].gNodeIndex+j].scale);
      fscanf(file,"%i",&(*nodeArray)[cTrees[i].gTreeIndex][cTrees[i].gNodeIndex+j].id);
      fscanf(file,"%f",&(*nodeArray)[cTrees[i].gTreeIndex][cTrees[i].gNodeIndex+j].desc_scale);
      fscanf(file,"%i",&(*nodeArray)[cTrees[i].gTreeIndex][cTrees[i].gNodeIndex+j].desc_id);
      fscanf(file,"%i",&(*nodeArray)[cTrees[i].gTreeIndex][cTrees[i].gNodeIndex+j].num_prog);
      fscanf(file,"%i",&(*nodeArray)[cTrees[i].gTreeIndex][cTrees[i].gNodeIndex+j].pid);
      fscanf(file,"%i",&(*nodeArray)[cTrees[i].gTreeIndex][cTrees[i].gNodeIndex+j].upid);
      fscanf(file,"%i",&(*nodeArray)[cTrees[i].gTreeIndex][cTrees[i].gNodeIndex+j].desc_pid);
      fscanf(file,"%i",&(*nodeArray)[cTrees[i].gTreeIndex][cTrees[i].gNodeIndex+j].phantom);
      fscanf(file,"%f",&(*nodeArray)[cTrees[i].gTreeIndex][cTrees[i].gNodeIndex+j].mVir);
      fscanf(file,"%f",&(*nodeArray)[cTrees[i].gTreeIndex][cTrees[i].gNodeIndex+j].mVirOrig);
      fscanf(file,"%f",&(*nodeArray)[cTrees[i].gTreeIndex][cTrees[i].gNodeIndex+j].rVir);
      fscanf(file,"%f",&(*nodeArray)[cTrees[i].gTreeIndex][cTrees[i].gNodeIndex+j].scaleRadius);
      fscanf(file,"%f",&(*nodeArray)[cTrees[i].gTreeIndex][cTrees[i].gNodeIndex+j].vRMS);
      fscanf(file,"%i",&(*nodeArray)[cTrees[i].gTreeIndex][cTrees[i].gNodeIndex+j].mmp);
      fscanf(file,"%f",&(*nodeArray)[cTrees[i].gTreeIndex][cTrees[i].gNodeIndex+j].scaleLastMM);
      fscanf(file,"%f",&(*nodeArray)[cTrees[i].gTreeIndex][cTrees[i].gNodeIndex+j].vMax);
      fscanf(file,"%f",&(*nodeArray)[cTrees[i].gTreeIndex][cTrees[i].gNodeIndex+j].pos[0]);
      fscanf(file,"%f",&(*nodeArray)[cTrees[i].gTreeIndex][cTrees[i].gNodeIndex+j].pos[1]);
      fscanf(file,"%f",&(*nodeArray)[cTrees[i].gTreeIndex][cTrees[i].gNodeIndex+j].pos[2]);
      fscanf(file,"%f",&(*nodeArray)[cTrees[i].gTreeIndex][cTrees[i].gNodeIndex+j].v[0]);
      fscanf(file,"%f",&(*nodeArray)[cTrees[i].gTreeIndex][cTrees[i].gNodeIndex+j].v[1]);
      fscanf(file,"%f",&(*nodeArray)[cTrees[i].gTreeIndex][cTrees[i].gNodeIndex+j].v[2]);
      fscanf(file,"%f",&(*nodeArray)[cTrees[i].gTreeIndex][cTrees[i].gNodeIndex+j].l[0]);
      fscanf(file,"%f",&(*nodeArray)[cTrees[i].gTreeIndex][cTrees[i].gNodeIndex+j].l[1]);
      fscanf(file,"%f",&(*nodeArray)[cTrees[i].gTreeIndex][cTrees[i].gNodeIndex+j].l[2]);
      fscanf(file,"%f",&(*nodeArray)[cTrees[i].gTreeIndex][cTrees[i].gNodeIndex+j].spin);
      fscanf(file,"\n");
    }
  }

  fclose(file);
  /* set nOutputTrees and totalNodes */
  /* which are needed, so, that the  */
  /* other functions know about the  */
  /* length of the arrays */
  *nOutputTrees = nTrees;
  *totalNodes = 0;
  for(i=0;i<nTrees;i++) {
    *totalNodes += (*galacticusTrees)[i].nNodes;
  }
  printf("%s contains %i nodes\n",filename,*totalNodes);


#ifdef PATCH
  fprintf(stdout, "started the new ID\n");
  /*calculate the parent ID to be the corresponding most up ID*/
  /*
  int l, m;
  for(i=0;i<nTrees;i++){
    for(j=0;j<(*galacticusTrees)[i].nNodes;j++){
      (*nodeArray)[i][j].pid = (*nodeArray)[i][j].upid;
    }
  }
  
  fprintf(stdout, "started the new ID for subs\n");
  for(i=0;i<nTrees;i++){
    for(j=0;j<(*galacticusTrees)[i].nNodes;j++){

      if((*nodeArray)[i][j].pid!=-1){
	levels = 0;
	do{
	  for(l=0;l<nTrees;l++){
	    for(m=0;m<(*galacticusTrees)[l].nNodes;m++){
	      if((*nodeArray)[i][j].pid==(*nodeArray)[l][m].id){		
		(*nodeArray)[i][j].pid = (*nodeArray)[l][m].upid;
		if((*nodeArray)[l][m].upid==-1){
		  levels = 5;
		}
	      }
	    }
	  }
	  levels++;
	}while(levels<4);
	if(levels!=6){
	  fprintf(stdout, "the host wast not found for tree %i node %i (%i)levels %i\n", 
		  i, j, (*nodeArray)[i][j].pid, levels);
	  exit(1);
	}
      }
    }
  }
  */  
#endif

#ifdef PATCH
  int search;
  int found;
  int l;
  int notfound;
  notfound=0;
  for(i=0;i<nTrees;i++){
    for(j=0;j<(*galacticusTrees)[i].nNodes;j++){
      search = (*nodeArray)[i][j].pid;
      if(search!=-1){

	found = 0;
	l = 0;
	do{
	  if((*nodeArray)[i][l].id==search)
	    found = 1;
	  l++;
	}while(!found && l<(*galacticusTrees)[i].nNodes);
	
	if(!found){
	  //	  fprintf(stderr, "Couldn't find pid %d for halo %d\n", search, (*nodeArray)[i][j].id);
	  (*nodeArray)[i][j].pid = -1;
	  notfound++;
	}	      
      }
    }
  }
  fprintf(stdout, "The number of not found parent ids %d\n", notfound);
  
 notfound=0;
  for(i=0;i<nTrees;i++){
    for(j=0;j<(*galacticusTrees)[i].nNodes;j++){
      search = (*nodeArray)[i][j].desc_id;
      if(search!=-1){

	found = 0;
	l = 0;
	do{
	  if((*nodeArray)[i][l].id==search)
	    found = 1;
	  l++;
	}while(!found && l<(*galacticusTrees)[i].nNodes);
	
	if(!found){
	  //	  fprintf(stderr, "Couldn't find pid %d for halo %d\n", search, (*nodeArray)[i][j].id);
	  (*nodeArray)[i][j].desc_id = -1;
	  notfound++;
	  fprintf(stdout, "Not found desc in tree %i - node %i out of %i\n", 
		  i, j,(*galacticusTrees)[i].nNodes);
	}	      
      }
    }
  }
  fprintf(stdout, "The number of not found desc ids %d\n", notfound);
  
#endif

  for(i=0;i<nTrees;i++) {
    for(j=0;j<(*galacticusTrees)[i].nNodes;j++) {
      /* if host node has no parent node, consistent_trees sets parent id field to -1  */
      /* galacticus, in such a case expects parent id field to be equal to the node id */
      if((*nodeArray)[i][j].pid==-1) {
	(*nodeArray)[i][j].pid=(*nodeArray)[i][j].id;
      }
    }
  }



  return 0;

}
