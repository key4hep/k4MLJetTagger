# Key4Hep Jet-Flavor Tagging implementation for CLD full simulation

This in an implementationg of jet-flavor tagging to key4hep for CLD full simulation at 240 GeV using the [k4-project Template](https://github.com/key4hep/k4-project-template).

We build an Gaudi Tranformer `JetTagger` (in `JetTagging/k4ProjectTemplate/src/components/JetTagger.cpp`) that follows these steps:
1. First, extract jet contituent variables (such as kinematics, track parameters, PID...) from every jet in the event.
2. Use these variables as input to a neural network ([Particle Transformer](https://arxiv.org/abs/2202.03772)). Here, we run inference on an [ONNX](https://onnx.ai/) exported trained network on 2 million jets / flavor using [weaver](https://github.com/hqucms/weaver-core).
3. Create $N$ new collections `RefinedJetTag_X` that saves the probability for each flavor $X$ ($H\rightarrow u \bar{u}$, $H\rightarrow d \bar{d}$, $H\rightarrow c \bar{c}$, $H\rightarrow s \bar{s}$, $H\rightarrow b \bar{b}$, $H\rightarrow g g$, $H\rightarrow \tau^- \tau^+$).


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


## Execution

Run the tagger by including the tranformer `JetTagger` in a steering file like this:

``` bash
k4run ../k4ProjectTemplate/options/createJetTagger.py
```

## Extras

You may find helpful resources in the `extras` folder. 

### Changing the inference model - exporting the model to ONNX

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

`torch_geometric` is still not supported by this envirnoment (not needed for using Particle Transformer, but e.g. for L-GATr).

### Changing the inference model - adjusting the code

- you need to change the paths to the model and its json config file in the steering file (here: `JetTagging/k4ProjectTemplate/options/createJetTagger.py`) by setting `model_path` and `json_path` in the `JetTagger` transformer initialization. 
- You should not need to change anything apart from the steering file assuming: 
    - You adopted the `flavor_collection_names` in the steering file `createJetTagger.py` matching the **order, label and size** that the network expects. E.g. if the network expects the first output to represent the probability of a $b$-jet, then the first item in the list `flavor_collection_names` need to be `yourCollectionName_B`. If your network distinuighishes between $n$ flavors, make sure to provide $n$ collection names. 
    - You used weaver to train your model. (If not, you need to adapts lots. Start building your own `WeaverInterface` header and source file, adopt the way the Struct `Jet` is transformed to fit the input formated expexted by your network (here done in `Helpers` with this function: `from_Jet_to_onnx_input`) and change the handling of the `json` config file if needed, including the extraction of all necessary inputs in the `tagger` function in `JetTagger.cpp`)
    - the `output_names` of the model in the json contig file have the format `yourname_isX`. If this changes (e.g. to `_X`), you need to adopt the `check_flavors` function and the `to_PDGflavor` map in `Helpers`. 
    - The naming of the input observables follows the FCCAnalyses convention. (As I don't like it, I use my own. Therefore, I have written a `VarMapper` class in `Helpers` that converts into my own key4hep convention. If you work with other convention, just update the `VarMapper`)
    - You use the same (or less) input parameter to the network. In case you want to extract more, have look at `JetObservablesRetriever` and modify the `Pfcand` Struct in `Structs.h`

### Adding new input observables for tagging

- You need to train a new model. 
- Export the model to ONNX as described above. 
- Extract the wanted parameter in `JetObservablesRetriever` and modify the `Pfcand` Struct in `Structs.h` by adding the new observables as an attribute. 

### When do I need to train a new model and exchange it here? 
- If you want to run at a different energy.
- If you want to use different input observables for tagging.
- If you want to use a different detector setup. 

So generally speaking, if the input to the network changes. *This* network implemented was trained on CLD full simulation at 240 GeV (`/eos/experiment/fcc/prod/fcc/ee/test_spring2024/240gev/Hbb/CLD_o2_v05/rec/`). Check out the performance in this [publication](https://repository.cern/records/4pcr6-r0d06).


## Further links:

- [ONNX implemention in FCCAnalyses](https://github.com/HEP-FCC/FCCAnalyses/tree/master/addons/ONNXRuntime): Strongly inspired this code.
- [k4-project-template](https://github.com/key4hep/k4-project-template)
