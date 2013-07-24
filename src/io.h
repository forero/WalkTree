void load_filelist(char *filename, char ***list, int *n_items);
int read_ctrees(float **scale_factor_list, int nsnaps, char * filename, int * nOutputTrees, 
		struct nodeData*** nodeArray, int **totalNodes);
void load_scale_factors(char *filename, float ***scales, int *n_items);
int find_snap(float **scale_factor_list, float aexp, int nsnap);
void dump_IDs(int my_file, char *filename_out, struct nodeData*** nodeArray, int *totalNodes, 
	      int nsnaps, int **UpMostID, int **FileUpMostID);
