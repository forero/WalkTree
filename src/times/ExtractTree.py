# -*- coding: utf-8 -*-
# <nbformat>3.0</nbformat>

# <codecell>

import numpy as np
import sys
import sets

# <codecell>

def formation_epoch(mass_history, aexp_history):
    aexp_formation = 0.0
    if(np.size(mass_history)>1):
        tmp_mass_history = mass_history/mass_history[0]
        for (mass, aexp) in zip(tmp_mass_history,aexp_history):
            if(mass < 0.5):
                aexp_formation = aexp
                break

    return aexp_formation

# <codecell>

tree_file = str(sys.argv[1])
condensed_tree_file=str(sys.argv[2])
print tree_file

#tree_file = "/home/forero/work/data/Bolshoi/BDM_Trees/plain_tree_0_0_0.dat"
#condensed_tree_file="/home/forero/work/data/Bolshoi/times/times_tree_0_0_0.dat"

# <codecell>

tree_data = np.loadtxt(tree_file)

# <codecell>

np.shape(tree_data)
#tree_root_ID = tree_data[:,30]

# <codecell>

tree_root_ID = np.int_(tree_data[:,29])

# <codecell>


# <codecell>

unique_tree_root_ID = np.array(list(sets.Set(tree_root_ID)))
print np.size(unique_tree_root_ID)

# <codecell>

#for tree_ID in unique_tree_root_ID:
TREE_ROOT_ID  = 29
ORIGINAL_HALO_ID = 30
SNAP_NUM = 31
HOST_HALO_ID=5
LAST_PROGENITOR_DEPTH_FIRST = 33
DEPTH_FIRST_ID = 28
COL_MVIR = 10
COL_AEXP = 0
COL_LAST_MM = 15
COL_POS_X = 17
COL_POS_Y = 18
COL_POS_Z = 19
COL_VEL_X = 20
COL_VEL_Y = 21
COL_VEL_Z = 22
COL_J_X = 23
COL_J_Y = 24
COL_J_Z = 25
COL_SPIN = 26

out = open(condensed_tree_file, 'w')
out.write("# tree_ID(0) host_halo_ID(1) BDM_ID(2) SNAP_NUMP(3) MASS(4) AEXP_LAST_MM(5) AEXP_FORMATION(6)")
out.write("# POS_X(7) POS_Y(8) POS_Z(9) VEL_X(10) VEL_Y(11) VEL_Z(12) SPIN(13)\n")
#this selects all the halos inside a tree
for tree_ID in unique_tree_root_ID:
#for i in range(10):
#    tree_ID = unique_tree_root_ID[i]
    #print tree_ID
    good_index = np.where(tree_data[:,TREE_ROOT_ID] == tree_ID)
#    print tree_ID, size(good_index[0])
    tmp_tree = tree_data[good_index[0],:]

    # I want to know the list where LAST_PROGENITOR is equal to DEPTH_FIRST_ID. Those are 
    # the tips of the branches at high redshift
    tmp_branches = np.where((np.int_(tmp_tree[:,LAST_PROGENITOR_DEPTH_FIRST])==(np.int_(tmp_tree[:,DEPTH_FIRST_ID]))))
    tmp_branches = np.int_(tmp_branches[0])
    #print "branches", tmp_branches, int_(tmp_tree[tmp_branches,LAST_PROGENITOR_DEPTH_FIRST])

    # I select the smallest of these values, which corresponds to the tip of the main branch
    tmp_last_prog_ID = np.amin(np.int_(tmp_tree[tmp_branches,LAST_PROGENITOR_DEPTH_FIRST]))

    #this is a consistency check, the tree_ID must be the same as the halo_ID of the first halo
    tmp_ID = np.int_(tmp_tree[0,1])
    if(tmp_ID!=tree_ID):
        print "FATAL ERROR in main halo indexing"
        exit

    #this is the ID of the main halo in the depth first order
    tmp_first_ID = np.int_(tmp_tree[0,DEPTH_FIRST_ID])

    #at this point all the halos in the main branch must have IDs between tmp_first_ID and tmp_last_prog_ID
    if(tmp_first_ID>tmp_last_prog_ID):
        print "FATAL ERROR in branch indexing"
        exit
    good_branch_index = np.where((np.int_(tmp_tree[:,DEPTH_FIRST_ID])>= tmp_first_ID) & (np.int_(tmp_tree[:,DEPTH_FIRST_ID])<=tmp_last_prog_ID))
    good_branch_index = good_branch_index[0]

    #this is the final selection of the aexp, halo mass and merger_aexp
    branch_mass = tmp_tree[good_branch_index,COL_MVIR]
    branch_aexp = tmp_tree[good_branch_index,COL_AEXP]
    branch_last_mm = tmp_tree[good_branch_index, COL_LAST_MM]
    branch_pos_x = tmp_tree[good_branch_index, COL_POS_X]
#    print np.int_(tmp_tree[good_branch_index,ORIGINAL_HALO_ID]), np.int_(tmp_tree[good_branch_index,SNAP_NUM])
    
    out.write("%ld %ld %ld %d %e %f %f %f %f %f %f %f %f %f\n"%\
                (tree_ID, np.int_(tmp_tree[good_branch_index,HOST_HALO_ID][0]),\
                np.int_(tmp_tree[good_branch_index,ORIGINAL_HALO_ID][0]), np.int_(tmp_tree[good_branch_index,SNAP_NUM][0]),\
                branch_mass[0], branch_last_mm[0],formation_epoch(branch_mass, branch_aexp),\
                tmp_tree[good_branch_index,COL_POS_X][0], \
                tmp_tree[good_branch_index,COL_POS_Y][0],\
                tmp_tree[good_branch_index,COL_POS_Z][0],\
                tmp_tree[good_branch_index,COL_VEL_X][0], \
                tmp_tree[good_branch_index,COL_VEL_Y][0],\
                tmp_tree[good_branch_index,COL_VEL_Z][0],\
                tmp_tree[good_branch_index,COL_SPIN][0]
                ))
                
              
out.close()
print "Done writing"

# <codecell>

print "formation", formation_epoch(branch_mass, branch_aexp)

# <codecell>

plot(branch_aexp, branch_pos_x)

# <codecell>


# <codecell>

formation_epoch(branch_mass, branch_aexp)

# <codecell>


