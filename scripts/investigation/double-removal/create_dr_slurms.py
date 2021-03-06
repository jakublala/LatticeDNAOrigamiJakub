# -*- coding: utf-8 -*-

"Creates slurm batch job for DR simulations"

import os

filebase = 'dr_const'
template = 'inps/serial_template_slurm.sh'

def read_seqfile(filename):
    """Read seq files in ??? format and strip metadata."""
    with open(filename) as inp:
        seqs = inp.read().split('\n')
        seqs = [seq for seq in seqs if not '>' in seq]
        seqs = [seq for seq in seqs if seq != '']

    return seqs # a list of string sequences

numberOfStaples = len(read_seqfile('inps/snodin_staples.seq'))

# Create all simulation combinations
simIndices = []
for i in range(numberOfStaples):
    for j in range(numberOfStaples):
        if i >= j:
            continue
        else:
            simIndices.append([i+1, j+1])

# No staple simulation setup
simIndices.append([0, 0])

def replace_ID(tempParams, staples):
    """Replaces all ID references in the shell script for the batch job"""
    for line in enumerate(tempParams):
        tempParams[line[0]] = line[1].replace('$ID', '{}_{}'.format(staples[0], staples[1]), 3)
    for line in enumerate(tempParams):
        tempParams[line[0]] = line[1].replace('$FILEBASE', '{}'.format(filebase), 3)
    return tempParams

with open(template, 'r') as file:
    params = file.readlines()

# Make directory for outputs
if not os.path.exists('slurms'):
    os.makedirs('slurms')

for i in simIndices:
    tempParams = params[:] # copy without reference
    tempParams = replace_ID(tempParams, i)
    
    # Create the input configuration file
    output = 'slurms/{}_{}_{}.sh'.format(filebase, i[0], i[1])
    with open(output, 'w') as file:
        file.writelines(tempParams)
        
# Make directory for outputs
if not os.path.exists('sim_folders'):
    os.makedirs('sim_folders')

# Create shell script to run all batch jobs
with open('sim_folders/dr_run_all.sh', 'w') as file:
    file.write('#!/bin/sh\n')
    file.write('\n')
    for i in simIndices:
        file.write('cd {}_{}\n'.format(i[0], i[1]))
        file.write('sbatch {}_{}_{}.sh\n'.format(filebase, i[0], i[1]))
        file.write('cd ..\n')
        
