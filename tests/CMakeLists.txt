include(ExternalData)
list(APPEND ExternalData_URL_TEMPLATES
  "https://key4hep.web.cern.ch:443/testFiles/k4JetTagger/%(hash)"
)

ExternalData_Add_Test(tagger_test
        NAME createJetTags
        COMMAND k4run k4JetTagger/options/createJetTags.py --inputFiles=DATA{${CMAKE_CURRENT_SOURCE_DIR}/inputFiles/test_spring2024_240gev_Hbb_CLD_o2_v05.root})
set_test_env(createJetTags)
set_tests_properties(
  createJetTags

  PROPERTIES
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

ExternalData_Add_Target(tagger_test)