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
from Configurables import JetTaggingAlg
from k4FWCore import ApplicationMgr, IOSvc

iosvc = IOSvc("IOSvc")
#iosvc.input = "/eos/experiment/fcc/prod/fcc/ee/test_spring2024/240gev/Hbb/CLD_o2_v05/rec/00016562/010/Hbb_rec_16562_10058.root"
iosvc.Input = "/afs/cern.ch/work/s/saaumill/public/fullsimGEN/cldfullsimHbb_REC.edm4hep.root"
iosvc.Output = "output_jettagging.root"
# iosvc.outputCommands = ["keep *"]

tagger = JetTaggingAlg(
    RefinedVertexJets="RefinedVertexJets",
)
tagger.PerEventPrintMessage = "Hi it's Sara"

ApplicationMgr(TopAlg=[tagger],
               EvtSel="NONE",
               EvtMax=3,
               ExtSvc=[iosvc],
               OutputLevel=INFO,
               )
