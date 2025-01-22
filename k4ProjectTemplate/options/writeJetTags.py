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
from Configurables import JetTagWriter, JetTagger, JetMCTagger
from Configurables import k4DataSvc
from Configurables import EventDataSvc
from Configurables import CollectionMerger
from Configurables import THistSvc
from k4FWCore import ApplicationMgr, IOSvc

svc = IOSvc("IOSvc")
svc.Input = "/afs/cern.ch/work/s/saaumill/public/fullsimGEN/CLDConfig/CLDConfig/cldfullsimHbb_test2_REC.edm4hep.root"

algList = []

# run tagger
flavor_collection_names = ["RefinedJetTag_G", "RefinedJetTag_U", "RefinedJetTag_S", "RefinedJetTag_C", "RefinedJetTag_B", "RefinedJetTag_D", "RefinedJetTag_TAU"]
transformer_recojets = JetTagger("JetTagger",
                        model_path="/afs/cern.ch/work/s/saaumill/public/onnx_export/fullsimCLD240_2mio.onnx",
                        json_path="/afs/cern.ch/work/s/saaumill/public/onnx_export/preprocess_fullsimCLD240_2mio.json",
                        flavor_collection_names = flavor_collection_names, # to make sure the order and nameing is correct
                        InputJets=["RefinedVertexJets"],
                        InputPrimaryVertices=["PrimaryVertices"],
                        OutputIDCollections=flavor_collection_names,
                        )
# run MC tagger
transformer_mcjets = JetMCTagger("JetMCTagger",
                        InputJets=["RefinedVertexJets"],
                        MCParticles=["MCParticles"],
                        )

# retrieve jet PID
MyJetTagWriter = JetTagWriter("JetTagWriter")
MyJetTagWriter.InputJets = "RefinedVertexJets"
MyJetTagWriter.RefinedJetTag_G = "RefinedJetTag_G"
MyJetTagWriter.RefinedJetTag_U = "RefinedJetTag_U"
MyJetTagWriter.RefinedJetTag_D = "RefinedJetTag_D"
MyJetTagWriter.RefinedJetTag_S = "RefinedJetTag_S"
MyJetTagWriter.RefinedJetTag_C = "RefinedJetTag_C"
MyJetTagWriter.RefinedJetTag_B = "RefinedJetTag_B"
MyJetTagWriter.RefinedJetTag_TAU = "RefinedJetTag_TAU"
MyJetTagWriter.MCJetTag = "MCJetTag"
THistSvc().Output = ["rec DATAFILE='jettags.root' TYP='ROOT' OPT='RECREATE'"]
# define root output file
THistSvc().OutputLevel = WARNING
THistSvc().PrintAll = False
THistSvc().AutoSave = True
THistSvc().AutoFlush = True

# append all algorithms to algList
algList.append(transformer_recojets)
algList.append(transformer_mcjets)
algList.append(MyJetTagWriter)

ApplicationMgr(TopAlg=algList,
               EvtSel="NONE",
               EvtMax=-1,
               ExtSvc=[k4DataSvc("EventDataSvc")],
               OutputLevel=INFO,
               )