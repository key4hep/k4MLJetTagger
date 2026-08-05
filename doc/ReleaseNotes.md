# v00.01.02

* 2026-05-23 Juan Miguel Carceller ([PR#15](https://github.com/key4hep/k4MLJetTagger/pull/15))
  - Remove k4DataSvc that no longer exists in k4FWCore, following https://github.com/key4hep/k4FWCore/pull/392

* 2026-04-28 Juan Miguel Carceller ([PR#14](https://github.com/key4hep/k4MLJetTagger/pull/14))
  - Remove a few unused imports in Python files

* 2026-02-04 Juan Miguel Carceller ([PR#13](https://github.com/key4hep/k4MLJetTagger/pull/13))
  - Fix crash when running with onnxruntime 1.23.2 by using `Ort::MemoryInfo` which is preferred in C++ compared to `OrtMemoryInfo` for C (and they should not be used together).

* 2025-11-07 Thomas Madlener ([PR#9](https://github.com/key4hep/k4MLJetTagger/pull/9))
  - Cleanup a few (unused) includes
  - Make sure that all member variables follow the convention of being `m_` prefixed and then using camelCase

# v00-01

* This file is also automatically populated by the tagging script