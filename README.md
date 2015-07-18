# FastTimeTestBeamAnalysis

This code is a clone of https://github.com/PFCal-dev/FastTimeTestBeamAnalysis.git and it has been modified. 


## Installation

The following is expected to work on any lxplus node.
```
git init
git clone https://github.com/cmsromadaq/H4DQM.git
git clone https://github.com/jdamgov/TBtiming15.git
cd FastTimeTestBeamAnalysis
make
```
A shared library (executable) will be produced under 
FastTimeTestBeamAnalysis/lib 
(FastTimeTestBeamAnalysis/bin)
containing the relevant functions for the reconstruction using H4DQM utils.
Before running the executable one must updated the environment variables with
```
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/lib
```

## Reconstruction step

To reconstruct events in a file run
```
./bin/RunH4treeReco input [output=H4treeRecoOutput.root]
```
where input is a csv list of input files and output is the name of the output file name.
A wrapper is provided to submit jobs to the batch
```
python scripts/runH4treeReco.py -i /store/group/dpg_ecal/alca_ecalcalib/TimingTB_H2_Jul2015/raw/DataTree/3351/ -q 8nh
```
submits the processing of all root files in the input directory to a lxbatch queue.
The output directory can be specified with -o.

## Analysis scripts

