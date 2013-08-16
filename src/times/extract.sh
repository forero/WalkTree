DATA_PATH_PLAIN=/home/forero/work/data/Bolshoi/BDM_Trees/
DATA_PATH_TIMES=/home/forero/work/data/Bolshoi/times/

TREE_FILE=tree_0_0_0.dat
PLAIN_FILE=plain_$TREE_FILE
TIMES_TILE=time_$TREE_FILE

python ExtractTree.py $DATA_PATH_PLAIN$PLAIN_FILE $DATA_PATH_TIMES$TIMES_FILE
