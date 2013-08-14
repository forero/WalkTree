#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define USAGE "./indextree.x input_tree_file output_index_file"
#define MAXLENCHAR 2048
#define CL_SCALE        0
#define CL_ID           1
#define CL_DESC_SCALE   2
#define CL_DESC_ID      3
#define CL_NUM_PROG     4
#define CL_HOST_ID      5
#define CL_TOP_HOST_ID  6
#define CL_DESC_HOST_ID 7
#define CL_PHANTOM      8
#define CL_NCOL         27
#define TREE_ID           0
#define TREE_DESC_ID      1
#define TREE_NUM_PROG     2
#define TREE_HOST_ID      3
#define TREE_TOP_HOST_ID  4
#define TREE_DESC_HOST_ID 5
#define TREE_PHANTOM      6
#define TREE_MASS         7
#define TREE_DESC_INDEX   8
#define TREE_NCOL       9
#define F_STRING   0
#define F_LONG_INT 1
#define F_EXP      2
#define F_FLOAT    3
#define N_LINES_SKIP 25
/*
  DESCRIPTION:
  Takes as inputs the trees constructed by Rockstar in order to apply the tree indexing defined by  Lemson & Springel in order to puload trees efficiently into a databse.

  AUTHOR:
  Jaime E. Forero-Romero

  DATE:
  Wed Oct 19 16:30:05 PDT 2011  
*/
long ***load_tree(char *infile, int *n_tree);
long **read_individual_tree(FILE *input, int *n_halos);
void parse_line(char *line, long *tree_entry);
int *count_halos_in_tree(char *infile, int n_tree);
void print_check(long ***tree_index, int *n_halos_per_tree, int n_tree);
void walk_tree(long **tree, int index_start, int n_nodes);
void find_next_to_visit(int base_index, long **tree, int *visited,int n_nodes,  int *next_index);
int *make_progenitor_index_list(int base_index, long **tree, int n_nodes);
void locate_main_prog(int * prog_list, long **tree, int *main_prog_index, int *visited);
void find_index(long **tree, int n_halos_per_tree, long ID, int *index, int ID_COL);
void fill_descendant_index(long **tree, int n_halos_per_tree);

int main(int argc, char **argv){
  char infile[MAXLENCHAR];
  char outfile[MAXLENCHAR];
  int n_tree;
  int *n_halos_per_tree;
  long ***tree_index;
  int i_tree;

  if(argc!=3){
    fprintf(stderr,"USAGE: %s\n", USAGE);
    exit(1);
  }
  
  strcpy(infile, argv[1]);
  strcpy(outfile, argv[2]);

  fprintf(stdout, "Input file: %s\n", infile);
  fprintf(stdout, "Output file: %s\n", outfile);

  n_tree=0;

  /*load the relevant indexes*/
  tree_index = load_tree(infile, &n_tree);
  fprintf(stdout, "The number of loaded trees is %d\n",n_tree);
    
  /*load the number of halos per tree*/
  n_halos_per_tree = count_halos_in_tree(infile, n_tree);

  /*some consistency check*/
#ifdef DEBUG_OUT
  print_check(tree_index, n_halos_per_tree, n_tree);
#endif


  /*walk around a tree*/
  i_tree = 100;
  for(i_tree=0;i_tree<n_tree;i_tree++){
    fprintf(stdout, "Started to fill the descendent index\n");
    fill_descendant_index(tree_index[i_tree], n_halos_per_tree[i_tree]);
    fprintf(stdout, "Walking around tree %d\n", i_tree);
    walk_tree(tree_index[i_tree],0,n_halos_per_tree[i_tree]);
  }
  return 0;
}

void fill_descendant_index(long **tree, int n_halos_per_tree){
  int i;
  long desc_ID;
  int desc_index;
  for(i=0;i<n_halos_per_tree;i++){
    desc_ID = tree[i][TREE_DESC_ID];
    find_index(tree, n_halos_per_tree, desc_ID, &desc_index, TREE_DESC_ID);
    tree[i][TREE_DESC_INDEX] = (long)(desc_index);
  }
}

void find_index(long **tree, int n_halos_per_tree, long ID, int *index, int ID_COL){
  int mirror_index;
  int found;
  int i;
  found=0;
  if(ID<0){
    mirror_index = ID;
    found=1;
  }else{
    i=0;
    do{
      if(tree[i][TREE_ID]==ID){
	found=1;
	mirror_index = i;
      }else{
	i++;
      }
    }while(!found && i<n_halos_per_tree);
  }
  if(found==0){
    fprintf(stderr, "The index is not present in the array");
    exit(1);
  }
  *index = mirror_index;
}

void walk_tree(long **tree, int index_start, int n_nodes){
  /*this start walking the tree, starting and finishing 
    at the node index_start*/
  int first_index;
  int i_node;
  int num_prog;
  int *visited;
  long *ordered;
  int i;
  int my_index, next_index;
  long *prog_list;
  fprintf(stdout, "Started going around %d nodes\n",n_nodes);  
  prog_list = NULL;

  /*allocate the array checking whether the halo as been visited*/
  if(!(visited=malloc(n_nodes * sizeof(int)))){
    fprintf(stderr, "error in the allocation\n");
    exit(1);
  }

  /*allocate the array produced after the depth first order search*/
  if(!(ordered=malloc(n_nodes * sizeof(long)))){
    fprintf(stderr, "error in the allocation\n");
    exit(1);
  }

  /*initalizatoin*/
  for(i=0;i<n_nodes;i++){
    visited[i] = 0;
    ordered[i] = -1;
  }


  /*make the links first */
  i_node = 1;
  my_index = index_start;

  do{
    visited[my_index] += 1;
    ordered[i_node] = my_index;

    /* find the next halo to visit */
    find_next_to_visit(my_index, tree, visited, n_nodes, &next_index);
    
    my_index = next_index;
    i_node++;
#ifdef WALK_DEBUG
    if(!(i_node%(n_nodes/100))){fprintf(stdout, "done %d/%d\n", i_node, n_nodes);}
#endif

  }while(i_node<n_nodes);

  visited[my_index] +=1;
  ordered[i_node] = my_index;

  fprintf(stdout, "Finished going around %d out of %d\n", i_node, n_nodes);
  /*consistency checks*/
  for(i=0;i<n_nodes;i++){
    if(visited[i]==0){
      fprintf(stdout, "FAIL Node %d  wasnt visited\n", i);
    }
    if(visited[i]>1){
      fprintf(stdout, "FAIL Node %d was visited %d times!\n", i, visited[i]);
    }
  }
}

void find_next_to_visit(int base_index, long **tree, int *visited,int n_nodes,  int *next_index){
  int next;
  int *prog_index_list;
  int found;
  int main_prog_index;
  int descendant_index;
  next = -1;
  found = 0;
  descendant_index = -1;
  if(base_index<0){
    fprintf(stderr, "Negative base index\n");
    exit(1);
}

  do{
    /* find the main progenitor */
    prog_index_list = make_progenitor_index_list(base_index, tree, n_nodes);  
    locate_main_prog(prog_index_list, tree, &main_prog_index, visited);
    
    /* find the descendant */
    descendant_index = (int)(tree[base_index][TREE_DESC_INDEX]);
#ifdef DEBUG
  fprintf(stdout, "(ID Desc %d) Descendant %d - Parent %d\n", (int)(tree[base_index][TREE_DESC_ID]),descendant_index, main_prog_index);
#endif
  if(main_prog_index!=-1 && !visited[main_prog_index]){
    next = main_prog_index;
    found = 1;
    *next_index = next;
#ifdef DEBUG
    fprintf(stdout, "Picked next(0) %d\n", next);
#endif
    return;
  }else if(main_prog_index==-1|| visited[main_prog_index]){
    find_next_to_visit(descendant_index, tree, visited, n_nodes,  &next);
    found=1;
  }
  }while(!found);


  if(prog_index_list!=NULL){
    free(prog_index_list);
  }
  
#ifdef DEBUG
  fprintf(stdout, "Picked next(1) %d\n", next);
#endif
  *next_index = next;
  return;
}

void locate_main_prog(int * prog_index_list, long **tree, int *main_prog_index, int *visited){
  int prog;
  int n_prog;
  long max_val;
  int i;
  int index;
  prog = -1;
  max_val = -1;
  if(prog_index_list!=NULL){
    n_prog = prog_index_list[0];    
    for(i=0;i<n_prog;i++){
      index = prog_index_list[i+1];
      if(tree[index][TREE_MASS]>max_val && !visited[index]){
	max_val = tree[index][TREE_MASS];
	prog = index;
      }
    }
  }
  *main_prog_index = prog;
}

int *make_progenitor_index_list(int base_index, long **tree, int n_nodes){
  int i;
  int *list;
  int n_prog;
  long my_id;
  list  =NULL;
  
  n_prog = 0;
  my_id = tree[base_index][TREE_ID];
  for(i=0;i<n_nodes;i++){
    if(tree[i][TREE_DESC_ID]==my_id){
      n_prog++;
    }   
  }
#ifdef DEBUG
  fprintf(stdout, "Found %d prog for ID %ld (C-index %d)(C-desc-index %ld)\n", n_prog, my_id, base_index, tree[base_index][TREE_DESC_INDEX]);
#endif
  if(n_prog>0){
    if(!(list=malloc((n_prog+1)*sizeof(int)))){
      fprintf(stderr, "problem in list allocation\n");
      exit(1);
    }
    list[0] = n_prog;
    n_prog = 0;
    my_id = tree[base_index][TREE_ID];
    for(i=0;i<n_nodes;i++){
      if(tree[i][TREE_DESC_ID]==my_id){
	#ifdef DEBUG
	if(n_prog==0){
	  fprintf(stdout, "Firstprog ID=%ld index=%d \n", tree[i][TREE_ID], i);
	}
#endif
	list[n_prog+1] = i;
	n_prog++;
      }   
    } 
  }

  
  return list;
}

void print_check(long ***tree_index, int *n_halos_per_tree, int n_tree){
  int i;
  int n_halo;
  for(i=n_tree-10;i<n_tree;i++){
    n_halo = n_halos_per_tree[i];
      fprintf(stdout, "Tree %d\n n_halos %d \n First %ld Last %ld\n", 
	      i, n_halo, tree_index[i][0][TREE_ID], tree_index[i][n_halo-1][TREE_ID]);
  }
}

/*loads the full file of trees*/
long *** load_tree(char *infile, int *n_tree){
  FILE *in;  
  char buffer[MAXLENCHAR];
  int i;
  int n_items;
  int n_halos;
  long  ***index_tree;
  n_items = 0;

  if(!(in=fopen(infile,"r"))){
    fprintf(stderr, "Problem opening file %s\n", infile);
    exit(1);
  }

  for(i=0;i<N_LINES_SKIP;i++){
    fgets(buffer, MAXLENCHAR, in);
    
#ifdef DEBUG
    fprintf(stdout, "%s", buffer);
#endif
  }
  fgets(buffer, MAXLENCHAR, in);
  sscanf(buffer, "%d\n", &n_items);
  fprintf(stdout, "Reading %d items\n", n_items);

  /*allocate the index_tree array*/
  if(!(index_tree=malloc(n_items * sizeof(long **)))){
    fprintf(stderr, "problem with index_tree allocation\n");
    exit(1);
  }
  for(i=0;i<n_items;i++){
    #ifdef DEBUG
    fprintf(stdout, "reading %d out of %d\n", i, n_items);
#endif
    index_tree[i] = read_individual_tree(in, &n_halos);
  }
  
  *n_tree = n_items;
  fclose(in);
  return index_tree;
}

/*loads the full file of trees*/
int *count_halos_in_tree(char *infile, int n_tree){
  FILE *in;  
  char buffer[MAXLENCHAR];
  int i;
  int n_items;
  int *n_halo_in_tree;
  long **dumb;
  int n_halos;

  if(!(in=fopen(infile,"r"))){
    fprintf(stderr, "Problem opening file %s\n", infile);
    exit(1);
  }

  for(i=0;i<N_LINES_SKIP;i++){
    fgets(buffer, MAXLENCHAR, in);
  }
  fgets(buffer, MAXLENCHAR, in);
  sscanf(buffer, "%d\n", &n_items);
  fprintf(stdout, "Reading %d items\n", n_items);

  /*allocate the index_tree array*/
  if(!(n_halo_in_tree=malloc(n_tree * sizeof(int)))){
    fprintf(stderr, "problem with n_halo_in_tree allocation\n");
    exit(1);
  }

  for(i=0;i<n_items;i++){
    dumb = read_individual_tree(in, &n_halos);
    free(dumb);
    n_halo_in_tree[i] = n_halos;
  }
  
  fclose(in);
  return n_halo_in_tree;
}

long **read_individual_tree(FILE *input, int *n_halos){
  long pos_init,pos_final, pos_now;
  int n_items;
  int begin, end;
  char *n;
  char buffer[MAXLENCHAR];
  long **tree;
  int i;
  
  pos_init = ftell(input);
  n_items=0;
  begin=0;
  end=0;

  /*check for consistency of the first line*/
  fgets(buffer, MAXLENCHAR, input);
  if(buffer[0]=='#'&&begin==0){
    begin=1;
  }else{
    fprintf(stderr,"file position is wrong\n tree does not start here\n %s", buffer);
    exit(1);
  }

  /*reads all the lines corresponding to the tree*/
  n=NULL;
  do{
    pos_now = ftell(input);
    n=fgets(buffer, MAXLENCHAR, input);
    if(buffer[0]!='#'&&n!=NULL){
      n_items++;
    }else{
      pos_final=pos_now;
      end=1;
    }
  }while(begin==1&&end==0);
#ifdef DEBUG
  fprintf(stdout, "%d items in tree\n", n_items);
#endif

  /*rewind to the beginning of the tree*/
  fseek(input, pos_init, SEEK_SET);
  if(!(tree=malloc(n_items * sizeof(long *)))){    
    fprintf(stderr, "error in the allocation of individual trees\n");
    exit(1);
  }
  
  /*parse the tree information*/
  n=fgets(buffer, MAXLENCHAR, input); 
  for(i=0;i<n_items;i++){

    n=fgets(buffer, MAXLENCHAR, input);
    if(!(tree[i]=malloc(TREE_NCOL * sizeof(long)))){
      fprintf(stderr, "error in the allocation of individual trees\n");
      exit(1);
    }    
    parse_line(buffer, tree[i]);
  }
  
  /*puts the pointer ready to read the next tree*/
  fseek(input, pos_final, SEEK_SET);
  *n_halos = n_items;
  return tree;
}

void parse_line(char *line, long *tree_entry){
  char *cptr, *strtokbuf;
  float aexp;
  float mvir;


  /*parse the elements in the line*/
  cptr=strtok_r(line, " ", &strtokbuf);
  aexp = (float)(atof(cptr));

  cptr=strtok_r(NULL, " ", &strtokbuf) ;
  tree_entry[TREE_ID] = (long)(atol(cptr));

  cptr=strtok_r(NULL, " ", &strtokbuf) ;
  aexp = (float)(atof(cptr));

  cptr=strtok_r(NULL, " ", &strtokbuf) ;
  tree_entry[TREE_DESC_ID] = (long)(atol(cptr));

  cptr=strtok_r(NULL, " ", &strtokbuf) ;
  tree_entry[TREE_NUM_PROG] = (long)(atol(cptr));

  cptr=strtok_r(NULL, " ", &strtokbuf) ;
  tree_entry[TREE_HOST_ID] = (long)(atol(cptr));

  cptr=strtok_r(NULL, " ", &strtokbuf) ;
  tree_entry[TREE_TOP_HOST_ID] = (long)(atol(cptr));

  cptr=strtok_r(NULL, " ", &strtokbuf) ;
  tree_entry[TREE_DESC_HOST_ID] = (long)(atol(cptr));

  cptr=strtok_r(NULL, " ", &strtokbuf) ;
  tree_entry[TREE_PHANTOM] = (long)(atol(cptr));

  cptr=strtok_r(NULL, " ", &strtokbuf) ;
  mvir = (atof(cptr));
  tree_entry[TREE_MASS] = (long)(mvir);
}
