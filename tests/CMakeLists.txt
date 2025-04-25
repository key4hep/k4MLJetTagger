#[[
Copyright (c) 2020-2024 Key4hep-Project.

This file is part of Key4hep.
See https://key4hep.github.io/key4hep-doc/ for further info.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
]]
include(ExternalData)
list(APPEND ExternalData_URL_TEMPLATES
  "https://key4hep.web.cern.ch:443/testFiles/k4MLJetTagger/%(hash)"
)

ExternalData_Add_Test(tagger_test
        NAME createJetTags
        COMMAND k4run k4MLJetTagger/options/createJetTags.py --inputFiles=DATA{${CMAKE_CURRENT_SOURCE_DIR}/inputFiles/test_spring2024_240gev_Hbb_CLD_o2_v05.root} --onnx_model=DATA{${CMAKE_CURRENT_SOURCE_DIR}/inputFiles/fullsimCLD240_2mio.onnx} --json_onnx_config=DATA{${CMAKE_CURRENT_SOURCE_DIR}/inputFiles/preprocess_fullsimCLD240_2mio.json})
set_test_env(createJetTags)
set_tests_properties(
  createJetTags

  PROPERTIES
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

ExternalData_Add_Target(tagger_test)