#
# Copyright (c) 2020-2024 Key4hep-Project.
#
# This file is part of Key4hep.
# See https://key4hep.github.io/key4hep-doc/ for further info.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
import uproot
import numpy as np
import matplotlib.pyplot as plt

from helper_rocs import *

path1 = "/afs/cern.ch/work/s/saaumill/public/tagging_performance/FCCnote/fullsimCLD240_2mio.root"
path2 = "/eos/experiment/fcc/ee/datasets/CLD_fullsim_tagging_results_key4hep/hadded/results_key4hep.root"

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