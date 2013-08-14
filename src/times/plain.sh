PATH_IN=/home/forero/work/data/Bolshoi/BDM_Trees/
PATH_OUT=/home/forero/work/data/Bolshoi/BDM_Trees/
FILE_IN=tree_0_0_0.dat 
FILE_OUT="clean_"tree_0_0_0.dat

awk < $PATH_IN/$FILE_IN  '{if($1!=0 && $1!=1.0 && $1<1.1)print $0}' > $PATH_OUT/$FILE_OUT

