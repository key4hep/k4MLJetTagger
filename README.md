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
cd /afs/cern.ch/work/s/saaumill/public/JetTagging/
source ./setup.sh
k4_local_repo
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../install -G Ninja -DPython_EXECUTABLE=$(which python3)
ninja install
```


## Execute Examples

Make sure that `../install/lib` and `../install/python` are in `LD_LIBRARY_PATH`
and `PYTHONPATH` respectively (`k4_local_repo` should take care of this).
If they are not, they can be added by running:
``` bash
export LD_LIBRARY_PATH=$PWD/../install/lib:$LD_LIBRARY_PATH
export PYTHONPATH=$PWD/../install/python:$PYTHONPATH
```
and then run the tagger like this:

``` bash
k4run ../k4ProjectTemplate/options/createJetTagger.py
```

or these dummy examples:
```
k4run ../k4ProjectTemplate/options/createHelloWorld.py
k4run ../k4ProjectTemplate/options/createExampleEventData.py
```

## Extras

In the `extras` folder you can find helpful resources. 

### Changing the inference model

If you wish to use a different model for tagging, you will need to export it to [ONNX](https://onnx.ai/). Here, we discribe how to transform a model with [weaver](https://github.com/hqucms/weaver-core) from `.pt` to `.onnx`. 

To export your favorite model `best_model.pt` (e.g. [Particle Transformer](https://arxiv.org/abs/2202.03772)) using `weaver` to onnx, run:

```bash
python3 -m weaver.train \
-c myConfigFromTraining.auto.yaml \
-n /path-to/particle_transformer/networks/example_ParticleTransformer.py \
-m /path-to/best_model.pt \
--export-onnx my-onnx-model.onnx
```

For that, we need an appropriate envirnoment as the one provided by `weaver` does not work for the conversion to ONNX. The envirnoment can be set-up with the YAML file in `extras` like: 

```bash
conda env create -f env_for_onnx.yml
conda activate weaver
```

`torch_geometric` is still not supported by this envirnoment (not needed for using Particle Transformer).



## References:

1. [lhcb-98-064 COMP](https://cds.cern.ch/record/691746/files/lhcb-98-064.pdf)
2. [Hello World in the Gaudi Framework](https://lhcb.github.io/DevelopKit/02a-gaudi-helloworld)
