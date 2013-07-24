#define MAXLEN 512
#define SNAP_ID   0
#define SNAP_AEXP 1
/* structure containing the tree id, nodecount,
   starting node and main node for each tree */
struct treeData {
  int id;
  int nNodes;
  int mainNodeId;
  int firstNode;
};

/* structure containing the data belonging to each
   node */
struct nodeData {
  int treeId;
  int id;
  float scale;
  /*
  float desc_scale;
  */
  int num_prog;
  int pid;
  int upid;
  int desc_pid;
  int desc_id;
  int phantom;
  float mVir;
  /*

  float mVirOrig;
  float rVir;
  float scaleRadius;
  float vRMS;

  int mmp;

  float scaleLastMM;
  float vMax;
  float pos[3];
  float v[3];
  float l[3];
  float spin;
  */
};
