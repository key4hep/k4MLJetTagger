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
from Gaudi.Configuration import INFO
from Configurables import JetTagger
from Configurables import ApplicationMgr
from Configurables import k4DataSvc
from Configurables import PodioOutput
from Configurables import PodioInput


podioevent = k4DataSvc("EventDataSvc")
podioevent.input = "/afs/cern.ch/work/s/saaumill/public/fullsimGEN/cldfullsimHbb_REC.edm4hep.root"

inp = PodioInput()
inp.collections = [
    "RefinedVertexJets",
]

out = PodioOutput("out")
out.filename = "output_jettagging.root"


transformer = JetTagger("JetTagger",
                        InputJets=["RefinedVertexJets"],
                        OutputIDCollections=["RefinedJetTags"])

ApplicationMgr(TopAlg=[inp, transformer, out],
               EvtSel="NONE",
               EvtMax=10,
               ExtSvc=[k4DataSvc("EventDataSvc")],
               OutputLevel=INFO,
               )