# Key4Hep Jet-Flavor Tagging implementation for CLD full simulation

This repository started from a k4-project-template to implement Jet-Flavor Tagging in key4hep for CLD full simulation at 240 GeV.
The workflow is as follows:
1. First, extract jet contituent variables (such as kinematics, track parameters, PID...) from every jet in the event.
2. Use these variables as input to a neural network (Particle Transformer). Here, we run inference on an ONNX exported trained network on 2 million jets / flavor.
3. Create a new collection that returns the probability for each flavor ($H\rightarrow u \bar{u}$, $H\rightarrow d \bar{d}$, $H\rightarrow c \bar{c}$, $H\rightarrow s \bar{s}$, $H\rightarrow b \bar{b}$, $H\rightarrow g g$, $H\rightarrow \tau^- \tau^+$).


## Dependencies

* ROOT

* PODIO

* Gaudi

* k4FWCore

## Installation

Within the `JetTagging` directory run:

``` bash
source /cvmfs/sw.hsf.org/key4hep/setup.sh
k4_local_repo
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../install -G Ninja -DPython_EXECUTABLE=$(which python3)
ninja install
```

Alternatively you can source the nightlies instead of the releases:

``` bash
source /cvmfs/sw-nightlies.hsf.org/key4hep/setup.sh
```

## Execute Examples

Make sure that `../install/lib` and `../install/python` are in `LD_LIBRARY_PATH`
and `PYTHONPATH` respectively (`k4_local_repo` should take care of this).
If they are not, they can be added by running:
``` bash
export LD_LIBRARY_PATH=$PWD/../install/lib:$LD_LIBRARY_PATH
export PYTHONPATH=$PWD/../install/python:$PYTHONPATH
```
and then run the examples like this:

``` bash
k4run ../k4ProjectTemplate/options/createHelloWorld.py
k4run ../k4ProjectTemplate/options/createExampleEventData.py
```


## References:

1. [lhcb-98-064 COMP](https://cds.cern.ch/record/691746/files/lhcb-98-064.pdf)
2. [Hello World in the Gaudi Framework](https://lhcb.github.io/DevelopKit/02a-gaudi-helloworld)
