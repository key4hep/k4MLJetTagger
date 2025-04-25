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
from k4FWCore.parseArgs import parser

# parse the custom arguments
parser_group = parser.add_argument_group("writeJetConstObs.py custom options")
parser_group.add_argument("--inputFiles", nargs="+", metavar=("file1", "file2"), help="One or multiple input files",
                        default=["/eos/experiment/fcc/prod/fcc/ee/test_spring2024/240gev/Hbb/CLD_o2_v05/rec/00016783/000/Hbb_rec_16783_99.root"])
parser_group.add_argument("--outputFile", help="Output file name", default="output_jettags.root")
parser_group.add_argument("--onnx_model", help="Path to ONNX model used for tagging", default="/eos/experiment/fcc/ee/jet_flavour_tagging/fullsim_test_spring2024/fullsimCLD240_2mio.onnx")
parser_group.add_argument("--json_onnx_config", help="Path to JSON config file for ONNX model used for tagging", default="/eos/experiment/fcc/ee/jet_flavour_tagging/fullsim_test_spring2024/preprocess_fullsimCLD240_2mio.json")
parser_group.add_argument("--num_ev", help="Number of events to process (-1 means all)", default=-1)

args = parser.parse_known_args()[0]

svc = IOSvc("IOSvc")
svc.Input = args.inputFiles

algList = []

# run tagger
flavor_collection_names = ["RefinedJetTag_G", "RefinedJetTag_U", "RefinedJetTag_S", "RefinedJetTag_C", "RefinedJetTag_B", "RefinedJetTag_D", "RefinedJetTag_TAU"]
transformer_recojets = JetTagger("JetTagger",
                        model_path=args.onnx_model,
                        json_path=args.json_onnx_config,
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
THistSvc().Output =["rec DATAFILE='{}' TYP='ROOT' OPT='RECREATE'".format(args.outputFile)]
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
               EvtMax=args.num_ev,
               ExtSvc=[k4DataSvc("EventDataSvc")],
               OutputLevel=INFO,
               )