void find_upmostID(struct nodeData*** nodeArray,int **nTotalNodes, int nsnaps, int nfiles_in, int ****XsUpMostID, int ****FileUpMostID);
int find_upid(int wanted, 
	       struct nodeData*** nodeArray,int **nTotalNodes, int nsnaps, int nfiles_in, 
	       int my_file, int my_snap, int my_node,
	       int *up_id, int *file_up_id);
