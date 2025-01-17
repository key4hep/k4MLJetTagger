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
from Gaudi.Configuration import INFO, WARNING
from Configurables import JetObsWriter
from Configurables import k4DataSvc
from Configurables import EventDataSvc
from Configurables import CollectionMerger
from Configurables import THistSvc
from k4FWCore import ApplicationMgr, IOSvc

svc = IOSvc("IOSvc")
svc.Input = "/afs/cern.ch/work/s/saaumill/public/fullsimGEN/CLDConfig/CLDConfig/cldfullsimHbb_test2_REC.edm4hep.root"
#svc.Output = "output_jettagging.root"

algList = []

# retrieve jet constituent observables for tagging

MyJetObsWriter = JetObsWriter("MyJetObsWriter")
MyJetObsWriter.InputJets = "RefinedVertexJets"
MyJetObsWriter.InputPrimaryVertices = "PrimaryVertices"
THistSvc().Output = ["rec DATAFILE='jetconst_obs.root' TYP='ROOT' OPT='RECREATE'"]
# define root output file
THistSvc().OutputLevel = WARNING
THistSvc().PrintAll = False
THistSvc().AutoSave = True
THistSvc().AutoFlush = True

algList.append(MyJetObsWriter)

ApplicationMgr(TopAlg=algList,
               EvtSel="NONE",
               EvtMax=-1,
               ExtSvc=[k4DataSvc("EventDataSvc")],
               OutputLevel=INFO,
               )