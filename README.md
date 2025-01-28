# Key4Hep Jet-Flavor Tagging Implementation for CLD Full Simulation

This in an implementationg of jet-flavor tagging to key4hep for CLD full simulation at 240 GeV using the [k4-project Template](https://github.com/key4hep/k4-project-template).

We build an Gaudi Transformer `JetTagger` (in `JetTagging/k4ProjectTemplate/src/components/JetTagger.cpp`) that works as follows:
1. First, it extracts jet contituent variables (such as kinematics, track parameters, PID...) from every jet in the event with `JetObservablesRetriever`.
2. Then, it uses these variables as input to a neural network ([Particle Transformer](https://arxiv.org/abs/2202.03772)). Here, we run inference on an [ONNX](https://onnx.ai/) exported trained network on 2 million jets / flavor using [weaver](https://github.com/hqucms/weaver-core). The code is in `WeaverInterface` and `ONNXRuntime`.
3. Create $N$ (here: 7) new collections `RefinedJetTag_X` that saves the probability for each flavor $X$ ($H\rightarrow u \bar{u}$, $H\rightarrow d \bar{d}$, $H\rightarrow c \bar{c}$, $H\rightarrow s \bar{s}$, $H\rightarrow b \bar{b}$, $H\rightarrow g g$, $H\rightarrow \tau^- \tau^+$).

This code base also allows you to 
- extract the MC jet flavor **assuming H(jj)Z(vv)** events by checking the PDG of the daughter particles of the Higgs Boson created. This is also implemented as a Gaudi Transformer `JetMCTagger`. 
- write the jet constituent observables used for tagging into a root file (e.g. for retraining a model) using `JetObsWriter` that accesses the observables retrieved in `JetObservablesRetriever`. 
- write the jet tags (MC and reco) into a root file (e.g. to create ROC curves) using `JetTagWriter`. 



## Dependencies

* ROOT

* PODIO

* Gaudi

* k4FWCore

## Installation

Within the `JetTagging` directory run:

``` bash
cd /your/path/to/this/repo/JetTagging/
source ./setup.sh
k4_local_repo
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../install -G Ninja -DPython_EXECUTABLE=$(which python3)
ninja install
```


## Execution

Run the tagger by including the tranformer `JetTagger` in a steering file like `createJetTags.py` and run it like this:

``` bash
k4run ../k4ProjectTemplate/options/createJetTags.py
```

This will return your edm4hep input file with the added `RefinedJetTag_X` collections. 

## Infomation about the steering files provided. 

There are 4 steering files provided in this repo in `/JetTagging/k4ProjectTemplate/options/`. They either start with `create` which refers to a steering file that will append a new collection to the input edm4hep files provided or they start with `write` and only produce root files as an output. 

- `createJetTags.py`: tags every jet using ML and appends 7 new PID collections `RefinedJetTag_X` with `X` being the 7 flavors (U, D, S, C, B, G, TAU). 
- `createJetMCTag.py`: appends one PID collection `MCJetTag` that refers to the MC jet flavor. **Warning**: This **assumes H(jj)Z(vv)** events as it checks the PDG of the daughter particles of the Higgs Boson in the event.
- `writeJetConstObs.py`: creates a root file with jet constituents observables which can be used for training a model or plotting the input parameters to the network for insights about the data. 
- `writeJetTags.py`: creates a root file with reco and MC jet tags which can be used for creating ROC curves.

## For Analysers: How to use the JetTagger output

If you want to use the Jet Tagger for your analyses, you most likely only need use a setup like in the steeringfile `createJetTags.py` which includes the Tagger in your steering file. As being said, this will attach 7 new PID collections to your edm4hep input file. You can then access the PID collections with a [PIDHandler](https://edm4hep.web.cern.ch/md_doc_2_p_i_d_handler.html) like being done in `JetTagWriter` (check it out). 

Every jet has *one* PID collection for *every flavor*. Each jet has a certain probability associated for being of each flavor. E.g. if you want to use all jets that have at least a probability $X$ being of flavor $b$ then you could write something like:

```
// loop over all jets and get the PID likelihood of being a b-jet
for (const auto jet : jet_coll) {
    auto jetTags_B = jetTag_B_Handler.getPIDs(jet);
    score_recojet_isB = jetTags_B[0].getLikelihood() // [0] because there should only be ONE b-tag PID collection associated to one jet 
    if(score_recojet_isB > X){
        // your code here
    }
}
```

## Information about the source files

Here a quick overview of the source files in this repo: 

*Gaudi Transformer:*
- `JetTagger.cpp`: Gaudi Transformer to attach jet tags (7) as PID collections to the input edm4hep file
- `JetMCTagger.cpp`: Gaudi Transformer to attach jet MC tag as PID collection to the input edm4hep file
*Gaudi Algorithms:*
- `JetTagWriter`: Gaudi Algorithm to write reco and MC jet tags into a root file
- `JetObsWriter`: Gaudi Algorithm to write jet constituent observables into a root file
*Other C++ Helpers*:
- `JetObservablesRetriever`: Defines a class to retrieve jet constituent observables from jet collection and vertex collection.
- `ONNXRuntime`: Interacts with ONNX model for inference.
- `WeaverInterface`: Wrapper around ONNXRuntime to match expected format from training the network with `weaver`. 
- `Structs.h`: Defines the structs `Pfcand` for saving information about the jet constituents, the struct `Helix` for saving track parameters and the struct `Jet` which is a vector of `Pfcand`.
- `Helpers`: Other helpers

## Retraining a model

### When do I need to train a new model and exchange it here? 
- If you want to run at a different energy.
- If you want to use different input observables for tagging (checkout [this section](#adding-new-input-observables-for-tagging)).
- If you want to use a different detector setup. 

So generally speaking, if the input to the network changes. *This* network implemented was trained on CLD full simulation at 240 GeV (`/eos/experiment/fcc/prod/fcc/ee/test_spring2024/240gev/Hbb/CLD_o2_v05/rec/`). Check out the performance in this [publication](https://repository.cern/records/4pcr6-r0d06).

<img src="./extras/flowchart.png" alt="JetTagger-flowchart" width="70%">

### How to retrain a model
0. Do some changed that require retraining.
1. Create a dataset to train the model on. The data should be stored in a root file and you can use the `JetObsWriter` to create them. You normally need ~1-2 mio jets/flavor to train a new model, that is why I recommend to create the root file using condor. Follow the instructions [here](#extra-section) to do so. 
2. I recommend using [weaver](https://github.com/doloresgarcia/weaver-core) to retrain a model and to make use of Dolores implementation of the ParticleTransformer using her repository/version of weaver. The branch `Saras-dev` also has the implementation of the L-GATr network which does not outperform the ParticleTransformer however. Here is how to set up the training with weaver:
- clone [weaver](https://github.com/doloresgarcia/weaver-core).
- clone your choice of network e.g. the [ParT](https://github.com/jet-universe/particle_transformer)
- set up the envirnoment by using a docker. First, create the `.sif` file with `singularity pull docker://dologarcia/gatr:v0` (or `:v` for L-GATr).You only need to do this once. Then export the cache with `export APPTAINER_CACHEDIR=/your/path/to/cache`. You can then activate the env with `singularity  shell -B /your/bindings/ --nv colorsinglet.sif `. Bindings could be `-B /eos -B /afs` if running on CERN resources.
- create a `.yaml` file to specify how to train the network. To use the jet observable convention used here, I have created a dummy config file in the `extras` section: `config_for_weaver_training.yaml`. Please check out the [open issues](#open-problems--further-work) to adapt the convention in the source code, but I *highly recommend* using the convention used in key4hep / here / in the dummy config when retraining the model and make the changes in the source code.
- create a [wandb](https://wandb.ai/) account. You need to connect your docker env *once* with your wandb account: Activate your env with `singularity  shell -B /your/bindings/ --nv colorsinglet.sif ` and then do `wandb login`. 

3. Run the training

- Go to a machine of your choice which at least one gpu. The code below shows how to run on 4 gpus.
- (Optional: Export your cache directory and) Activate your env with `singularity  shell -B /your/bindings/ --nv colorsinglet.sif `
- Go to `/path/to/weaver-core/`
- Run following command:

```
torchrun --standalone --nnodes=1 --nproc_per_node=4 /your/path/to/weaver-core/weaver/train.py \
--data-train /your/path/to/data/*.root \
--data-config /your/path/to/configs/config_for_weaver_training.yaml \
--network-config /your/path/to/particle_transformer/networks/example_ParticleTransformer.py \
--model-prefix /your/path/to/model_weights/mykey4hepmodel/ \
--num-workers 0 --gpus 0,1,2,3 \
--batch-size 2048 --start-lr 1e-3 --num-epochs 60 \
--optimizer ranger --fetch-step 0.01 \
--log-wandb --wandb-displayname myfirsttraining \
--wandb-projectname FCC-tagging-with-key4hep \
--lr-scheduler reduceplateau --backend nccl
```

The `--nproc_per_node=4` specifies that you have 4 gpus and `--gpus 0,1,2,3` which ones you would like to use. For every training on a different dataset, create a *new* config file and use the `.auto.yaml` created by weaver to run inference later. Create a folder where to store the model weights `--model-prefix /your/path/to/model_weights/mykey4hepmodel/`. You can load model weights by adding `--load-model-weights /your/path/to/model_weights/old_training/_epoch-X_state.pt \` to the command with `X` being the epoch you want to load. To find out more check out the [wiki](https://github.com/doloresgarcia/weaver-core/wiki/Training). 

4. Once satisfied with the training, you should have your model weights saved like `/your/path/to/model_weights/mykey4hepmodel/_best_epoch_state.pt`. 
5. Optional: You can also run inference using weaver. Please use other data than you used for training to run inference. Use the auto generated config file from training to run the inference. You can use the code provided in `extras/plotting` to check if your `results.root` file matches the performances expected.

```
python3 -m weaver.train --predict --data-test /your/path/to/test-data/*.root \
--data-config your/path/to/configs/config_for_weaver_training.248dcd877468b36a361a73654bb4e913.auto.yaml \
--network-config /your/path/to/particle_transformer/networks/example_ParticleTransformer.py \
--model-prefix /your/path/to/model_weights/mykey4hepmodel/_best_epoch_state.pt \
--gpus 0 --batch-size 64 \
--predict-output /your/path/to/mykey4heptagging/results.root
```

6. Export your `.pt` model to ONNX. Follow [these instructions](#changing-the-inference-model---exporting-the-model-to-onnx).
7. Check if you need to do adjustements on the source code. Follow [these instructions](#changing-the-inference-model---adjusting-the-code).
8. Run interence and check the performance. Help yourself using the scripts in `extras/plotting`. 
9. Be proud of yourself. Well done :) 




### Changing the inference model - exporting the model to ONNX

If you wish to use a different model for tagging, you will need to export the trained model to [ONNX](https://onnx.ai/). Here, we discribe how to transform a model with [weaver](https://github.com/hqucms/weaver-core) from `.pt` to `.onnx`. 

To export your favorite model `best_model.pt` (e.g. [Particle Transformer](https://arxiv.org/abs/2202.03772)) using `weaver` to onnx, run:

```bash
python3 -m weaver.train \
-c myConfigFromTraining.auto.yaml \
-n /path-to/particle_transformer/networks/example_ParticleTransformer.py \
-m /path-to/best_model.pt \
--export-onnx my-onnx-model.onnx
```

For that, we need an appropriate envirnoment as the one provided by `weaver` does not work for the conversion to ONNX. The envirnoment can be set-up with the YAML file in `extras` folder like: 

```bash
conda env create -f env_for_onnx.yml
conda activate weaver
```

`torch_geometric` is still not supported by this envirnoment (not needed for using Particle Transformer, but e.g. for L-GATr).

### Changing the inference model - adjusting the code

- you need to change the paths to the model and its json config file in the steering file (here: `JetTagging/k4ProjectTemplate/options/createJetTags.py`) by setting `model_path` and `json_path` in the `JetTagger` transformer initialization. 
- You should not need to change anything apart from the steering file assuming: 
    - You adopted the `flavor_collection_names` in the steering file `createJetTags.py` matching the **order, label and size** that the network expects. E.g. if the network expects the first output to represent the probability of a $b$-jet, then the first item in the list `flavor_collection_names` need to be `yourCollectionName_B`. If your network distinuighishes between $n$ flavors, make sure to provide $n$ collection names. 
    - You used weaver to train your model. (If not, you need to adapts lots. Start building your own `WeaverInterface` header and source file, adopt the way the Struct `Jet` is transformed to fit the input formated expexted by your network (here done in `Helpers` with this function: `from_Jet_to_onnx_input`) and change the handling of the `json` config file if needed, including the extraction of all necessary inputs in the `tagger` function in `JetTagger.cpp`)
    - the `output_names` of the model in the json contig file have the format `yourname_isX`. If this changes (e.g. to `_X`), you need to adopt the `check_flavors` function and the `to_PDGflavor` map in `Helpers`. 
    - The naming of the input observables follows the FCCAnalyses convention. (As I don't like it, I use my own. Therefore, I have written a `VarMapper` class in `Helpers` that converts into my own key4hep convention. If you work with other convention, just update the `VarMapper`). I hope that in the future, people will adopt my convention for training the network too, then `VarMapper` is not needed anymore. Read [this section](#open-problems--further-work) to find out how to adopt the code. 
    - You use the same (or less) input parameter to the network. In case you want to extract more, have look at `JetObservablesRetriever` and modify the `Pfcand` Struct in `Structs.h`

### Adding new input observables for tagging

- Extract the wanted parameter in `JetObservablesRetriever` and modify the `Pfcand` Struct in `Structs.h` by adding the new observables as an attribute.
- Modify `JetObsWriter` and add your new observable to be saved in the output root file.
- Retrieve a root file (default `jetconst_obs.root`) by running `k4run ../k4ProjectTemplate/options/writeJetConstObs.py` which uses the `JetObsWriter`. To create larger data, submit the jobs to condor (see `extras/submit_to_condor`) explained [here](#extra-section). 
- Use the root output (`jetconst_obs.root`, or to be more precise, the root files from your condor submit because you need plenty of data to retrain a model) to _retrain the model_.
- Convert your trained model to ONNX as explained [above](#changing-the-inference-model---exporting-the-model-to-onnx).


## Extra section

You may find helpful resources in the `extras` folder. 

- *Creation of a conda env for onnx export*: To export a model trained with weaver you need an appropriate envirnoment. You can set it up with `env_for_onnx.yml`. See [this section](#changing-the-inference-model---exporting-the-model-to-onnx).

- *Example weaver config file*: The `config_for_weaver_training.yaml` is an example config file to be used for training a network with weaver using the key4hep convention for jet constituent observables names. 

- *Submitting jobs to condor*: You will find `jettagswriter.sub` which submits the `writeJetTags.py` steering file to condor to run it on large data samples. It produces root files with the saved jet tags (reco from the tagger and MC). **Please** run `python write_sub_JetTagsWriter.py` to create the `.sub` file with **your** output paths and data etc. by **modifiying** the python script. Same for `jetobswriter.sub` which submits a job to condor to run the `writeJetConstObs.py` steering file. For more information about condor jobs, see this [documentation](https://batchdocs.web.cern.ch/local/quick.html).

```
# go on lxplus
cd /path/to/JetTagging/extras/submit_to_condor/
# MODIFY the python script to match your data, paths and job
python write_sub_JetObsWriter.py
# check if jetobswriter.sub looks ok

condor_submit jetobswriter.sub
```

- *Plots*: You can find some plotting scripts in `extras/plotting`: 
    - `jetobs_comparison.ipynb` is a notebook that plots the distribution of jet constituent observables used for tagging retrieved with a steering file like `writeJetConstObs.py` that uses the Gaudi algorithm `JetObsWriter`. The notebook compares the distributions of two jet observables from different root files. The helper functions for this notebook are defined in `helper_jetobs.py`
    - `rocs_comparison.ipynb` is a notebook that compares two ROC curves for different flavors. The data used for the ROCs should come from two root files retrieved with a steering file like `writeJetTags.py` that uses the Gaudi algorithm `JetTagsWriter`. The helper functions for this notebook are defined in `helper_rocs.py`. If you don't like jupyter notebooks and just want to save the plots use `save_rocs.py` and adopt the path where you want to save the plots in `helper_rocs.py`



## Open problems / further work

- The magnetic field $B$ of the detector is needed at one point to calculate the helix parameters of the tracks with respect to the primary vertex. The magnetic field is hard coded at the moment. It would be possible to retrieve it from the detector geometry (code already added, see the `Helper` file) but therefore one must load the detector in the steering file, e.g. like [this](https://github.com/key4hep/CLDConfig/blob/ae99dbed8e34390036e29ca09897dc0ed7759030/CLDConfig/CLDReconstruction.py#L61-L66). As we use the v05 version of CLD at the moment, loading the detector is slow and not worth it to only set $Bz=2.0$ (in my opinion). With a newer detector version (e.g. v07) this might be worth investigating.
- Currently, the network used was trained using the [FCCAnalyses convention](https://github.com/HEP-FCC/FCCAnalyses/blob/fa672d4326bcf2f43252d3554a138b53dcba15a4/examples/FCCee/weaver/config.py#L31) for naming the jet constituents observables. The naming is quite confusing, this is why I used the my own convention that matches the [key4hep convention](https://github.com/key4hep/EDM4hep/blob/997ab32b886899253c9bc61adea9a21b57bc5a21/edm4hep.yaml#L195-L199). The class `VarMapper` in `Helpers` helps to switch between the two conventions. In the future, if retraining a model, I highly suggest switching to the convention used here when training the model to get rid of the FCCAnalyses convention. To do so, train the network with a yaml file like `extras/config_for_weaver_training.yaml` and root files created with `writeJetConstObs.py` which use the key4hep convention. For running inference here in key4hep, you only need to modify the function `from_Jet_to_onnx_input` in `Helpers` where the `VarMapper` is used. Remove it, there should be no need for converting conventions anymore. 


## Further links:

- [ONNX implemention in FCCAnalyses](https://github.com/HEP-FCC/FCCAnalyses/tree/master/addons/ONNXRuntime): Strongly inspired this code.
- [k4-project-template](https://github.com/key4hep/k4-project-template)
