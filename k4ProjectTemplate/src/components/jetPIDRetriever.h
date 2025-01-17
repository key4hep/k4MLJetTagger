#ifndef JETOBSWRITER_H
#define JETOBSWRITER_H

#include "k4FWCore/DataHandle.h"
#include "k4FWCore/MetaDataHandle.h"
#include "Gaudi/Algorithm.h"
#include "GaudiKernel/ITHistSvc.h"

#include <edm4hep/ReconstructedParticleCollection.h>
#include <edm4hep/VertexCollection.h>
#include <edm4hep/EventHeaderCollection.h>

#include "TH1F.h"
#include "TGraph.h"

/**
 * @class JetPIDRetriever
 * @brief This class is a Gaudi algorithm for writing jet PIDs to a TTree.
 *
 * The algorithm follows the Gaudi framework's lifecycle, with the initialize() method being called at the start,
 * execute() method being called for each event, and finalize() method being called at the end. The algorithm also
 * provides methods for initializing and cleaning the TTree.
 *
 * The execute function ...  
 *
 * The output root file can be used for creating ROC curves to check the tagging performance.
 * 
 * @author Sara Aumiller
 */
class JetPIDRetriever : public Gaudi::Algorithm {

    public:
    /// Constructor.
    JetPIDRetriever(const std::string& name, ISvcLocator* svcLoc);
    /// Destructor.
    ~JetPIDRetriever() {

        delete recojet_isG;
        delete score_recojet_isG;
        delete recojet_isU;
        delete score_recojet_isU;
        delete recojet_isD;
        delete score_recojet_isD;
        delete recojet_isS;
        delete score_recojet_isS;
        delete recojet_isC;
        delete score_recojet_isC;
        delete recojet_isB;
        delete score_recojet_isB;
        delete recojet_isTAU;
        delete score_recojet_isTAU;

    };
    /// Initialize.
    virtual StatusCode initialize();
    /// Initialize tree.
    void initializeTree();
    /// Clean tree.
    void cleanTree() const;
    /// Execute function.
    virtual StatusCode execute(const EventContext&) const;
    /// Finalize.
    virtual StatusCode finalize();


    private:
        mutable DataHandle<edm4hep::EventHeaderCollection> ev_handle {"EventHeader", Gaudi::DataHandle::Reader, this};
        mutable DataHandle<edm4hep::ParticleIDCollection> reco_jettag_isG_handle {"RefindJetTag_isG", Gaudi::DataHandle::Reader, this};
        mutable DataHandle<edm4hep::ParticleIDCollection> reco_jettag_isU_handle {"RefindJetTag_isU", Gaudi::DataHandle::Reader, this};
        mutable DataHandle<edm4hep::ParticleIDCollection> reco_jettag_isD_handle {"RefindJetTag_isD", Gaudi::DataHandle::Reader, this};
        mutable DataHandle<edm4hep::ParticleIDCollection> reco_jettag_isS_handle {"RefindJetTag_isS", Gaudi::DataHandle::Reader, this};
        mutable DataHandle<edm4hep::ParticleIDCollection> reco_jettag_isC_handle {"RefindJetTag_isC", Gaudi::DataHandle::Reader, this};
        mutable DataHandle<edm4hep::ParticleIDCollection> reco_jettag_isB_handle {"RefindJetTag_isB", Gaudi::DataHandle::Reader, this};
        mutable DataHandle<edm4hep::ParticleIDCollection> reco_jettag_isTAU_handle {"RefindJetTag_isTAU", Gaudi::DataHandle::Reader, this};
        mutable DataHandle<edm4hep::ParticleIDCollection> mc_jettag_handle {"MCJetTag", Gaudi::DataHandle::Reader, this};


        SmartIF<ITHistSvc> m_ths;  ///< THistogram service

        mutable TTree* t_jettag{nullptr};

        std::vector<float> *recojet_isG = nullptr;
        std::vector<float> *score_recojet_isG = nullptr;
        std::vector<float> *recojet_isU = nullptr;
        std::vector<float> *score_recojet_isU = nullptr;
        std::vector<float> *recojet_isD = nullptr;
        std::vector<float> *score_recojet_isD = nullptr;
        std::vector<float> *recojet_isS = nullptr;
        std::vector<float> *score_recojet_isS = nullptr;
        std::vector<float> *recojet_isC = nullptr;
        std::vector<float> *score_recojet_isC = nullptr;
        std::vector<float> *recojet_isB = nullptr;
        std::vector<float> *score_recojet_isB = nullptr;
        std::vector<float> *recojet_isTAU = nullptr;
        std::vector<float> *score_recojet_isTAU = nullptr;

};

#endif // JETOBSWRITER_H