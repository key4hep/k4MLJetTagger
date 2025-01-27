import uproot 
import numpy as np
import matplotlib.pyplot as plt

from helper_rocs import *

path1 = "/afs/cern.ch/work/s/saaumill/public/tagging_performance/FCCnote/fullsimCLD240_2mio.root"
path2 = "/eos/experiment/fcc/ee/datasets/CLD_fullsim_tagging_results_key4hep/hadded/mini/results.root"
#path2 = "/eos/experiment/fcc/ee/datasets/CLD_fullsim_tagging_results_key4hep/hadded/results_key4hep_fullsim_tagging_small.root"
#path2 = "/afs/cern.ch/work/s/saaumill/public/tagging_performance/FCCnote/fastsimCLD240_noPID_2mio.root"

print("Loading data...")

data1 = load_data(path1, key='Events;1')
data2 = load_data(path2)

print("Data loaded.")

save_status = True

print("Plotting...")

#rocs 
all_rocs(data1, data2, 'FullSim CLD', 'from key4hep', save=save_status, name='roc-key4hep-check_small')

# non-binary discriminators
dic_non_bi = {"b": ["c", "b", "g"], "c": ["c", "b", "g"], "s": ["ud", "g", "s"], "g": ["ud", "b", "g"]}
dic_xlim = {"b": None, "c": [-5, 4], "s": [-3.5, 2], "g": [-4.5, 2]}
non_binary_disc(data1, data2, dic_non_bi, 'FullSim CLD', 'from key4hep', dic_xlim, save=save_status, name="discrim-non-bi_key4hep-check_small", lax=0)

print("Done.")