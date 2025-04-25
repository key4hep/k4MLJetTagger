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
from Configurables import JetMCTagger
from Configurables import k4DataSvc
from Configurables import EventDataSvc
from Configurables import CollectionMerger
from k4FWCore import ApplicationMgr, IOSvc
from k4FWCore.parseArgs import parser

# parse the custom arguments
parser_group = parser.add_argument_group("createJetMCTag.py custom options")
parser_group.add_argument("--inputFiles", nargs="+", metavar=("file1", "file2"), help="One or multiple input files",
                        default=["/eos/experiment/fcc/prod/fcc/ee/test_spring2024/240gev/Hbb/CLD_o2_v05/rec/00016783/000/Hbb_rec_16783_99.root"])
parser_group.add_argument("--outputFile", help="Output file name", default="output_jetMCtags.root")
parser_group.add_argument("--num_ev", type=int, help="Number of events to process (-1 means all)", default=-1)

args = parser.parse_known_args()[0]

svc = IOSvc("IOSvc")
svc.Input = args.inputFiles
svc.Output = args.outputFile

transformer = JetMCTagger("JetMCTagger",
                        InputJets=["RefinedVertexJets"],
                        MCParticles=["MCParticles"],
                        )

ApplicationMgr(TopAlg=[transformer],
               EvtSel="NONE",
               EvtMax=args.num_ev,
               ExtSvc=[k4DataSvc("EventDataSvc")],
               OutputLevel=INFO,
               )