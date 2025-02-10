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
 * @class JetObsWriter
 * @brief This class is a Gaudi algorithm for writing jet observables that are used for tagging to a TTree.
 *
 * The algorithm follows the Gaudi framework's lifecycle, with the initialize() method being called at the start,
 * execute() method being called for each event, and finalize() method being called at the end. The algorithm also
 * provides methods for initializing and cleaning the TTree.
 *
 * The execute function loops over all jets in the events and retrieves the observables for tagging with the JetObservablesRetriever. 
 * It then dumps all the information into a TTree.
 * The output root file can be used for training a neural network for jet tagging.
 * 
 * @note The naming convention for the observables follows the key4hep implementation (see Structs.h and for the conversion to the old FCCAnalyses convention Helpers.cpp).
 *
 * @author Sara Aumiller
 */
class JetObsWriter : public Gaudi::Algorithm {

    public:
    /// Constructor.
    JetObsWriter(const std::string& name, ISvcLocator* svcLoc);
    /// Destructor.
    ~JetObsWriter() {

        delete pfcand_erel_log;
        delete pfcand_thetarel;
        delete pfcand_phirel;
        delete pfcand_e;
        delete pfcand_p;
        delete pfcand_type;
        delete pfcand_charge;
        delete pfcand_isEl;
        delete pfcand_isMu;
        delete pfcand_isGamma;
        delete pfcand_isChargedHad;
        delete pfcand_isNeutralHad;
        delete pfcand_dndx;
        delete pfcand_tof;
        delete pfcand_cov_omegaomega;
        delete pfcand_cov_tanLambdatanLambda;
        delete pfcand_cov_phiphi;
        delete pfcand_cov_d0d0;
        delete pfcand_cov_z0z0;
        delete pfcand_cov_d0z0;
        delete pfcand_cov_phid0;
        delete pfcand_cov_tanLambdaz0;
        delete pfcand_cov_d0omega;
        delete pfcand_cov_d0tanLambda;
        delete pfcand_cov_phiomega;
        delete pfcand_cov_phiz0;
        delete pfcand_cov_phitanLambda;
        delete pfcand_cov_omegaz0;
        delete pfcand_cov_omegatanLambda;
        delete pfcand_d0;
        delete pfcand_z0;
        delete pfcand_Sip2dVal;
        delete pfcand_Sip2dSig;
        delete pfcand_Sip3dVal;
        delete pfcand_Sip3dSig;
        delete pfcand_JetDistVal;
        delete pfcand_JetDistSig;

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
        mutable DataHandle<edm4hep::ReconstructedParticleCollection> inputJets_handle {"InputJets", Gaudi::DataHandle::Reader, this};
        mutable DataHandle<edm4hep::VertexCollection> inputPrimaryVertices_handle {"InputPrimaryVertices", Gaudi::DataHandle::Reader, this};
        mutable DataHandle<edm4hep::MCParticleCollection> inputMCParticles_handle {"InputMCParticles", Gaudi::DataHandle::Reader, this};

        SmartIF<ITHistSvc> m_ths;  ///< THistogram service

        mutable TTree* t_jetcst{nullptr};

        mutable std::vector<float> *pfcand_erel_log = nullptr;
        mutable std::vector<float> *pfcand_thetarel = nullptr;
        mutable std::vector<float> *pfcand_phirel = nullptr;
        mutable std::vector<float> *pfcand_e = nullptr;
        mutable std::vector<float> *pfcand_p = nullptr;
        mutable std::vector<int> *pfcand_type = nullptr;
        mutable std::vector<int> *pfcand_charge = nullptr;
        mutable std::vector<int> *pfcand_isEl = nullptr;
        mutable std::vector<int> *pfcand_isMu = nullptr;
        mutable std::vector<int> *pfcand_isGamma = nullptr;
        mutable std::vector<int> *pfcand_isChargedHad = nullptr;
        mutable std::vector<int> *pfcand_isNeutralHad = nullptr;
        mutable std::vector<float> *pfcand_dndx = nullptr;
        mutable std::vector<float> *pfcand_tof = nullptr;
        mutable std::vector<float> *pfcand_cov_omegaomega = nullptr;
        mutable std::vector<float> *pfcand_cov_tanLambdatanLambda = nullptr;
        mutable std::vector<float> *pfcand_cov_phiphi = nullptr;
        mutable std::vector<float> *pfcand_cov_d0d0 = nullptr;
        mutable std::vector<float> *pfcand_cov_z0z0 = nullptr;
        mutable std::vector<float> *pfcand_cov_d0z0 = nullptr;
        mutable std::vector<float> *pfcand_cov_phid0 = nullptr;
        mutable std::vector<float> *pfcand_cov_tanLambdaz0 = nullptr;
        mutable std::vector<float> *pfcand_cov_d0omega = nullptr;
        mutable std::vector<float> *pfcand_cov_d0tanLambda = nullptr;
        mutable std::vector<float> *pfcand_cov_phiomega = nullptr;
        mutable std::vector<float> *pfcand_cov_phiz0 = nullptr;
        mutable std::vector<float> *pfcand_cov_phitanLambda = nullptr;
        mutable std::vector<float> *pfcand_cov_omegaz0 = nullptr;
        mutable std::vector<float> *pfcand_cov_omegatanLambda = nullptr;
        mutable std::vector<float> *pfcand_d0 = nullptr;
        mutable std::vector<float> *pfcand_z0 = nullptr;
        mutable std::vector<float> *pfcand_Sip2dVal = nullptr;
        mutable std::vector<float> *pfcand_Sip2dSig = nullptr;
        mutable std::vector<float> *pfcand_Sip3dVal = nullptr;
        mutable std::vector<float> *pfcand_Sip3dSig = nullptr;
        mutable std::vector<float> *pfcand_JetDistVal = nullptr;
        mutable std::vector<float> *pfcand_JetDistSig = nullptr;
        // debug variables
        mutable float jet_PV_x;
        mutable float jet_PV_y;
        mutable float jet_PV_z;
        mutable int jet_PV_id;
        mutable double jet_MCPV_x;
        mutable double jet_MCPV_y;
        mutable double jet_MCPV_z;

        mutable std::int32_t evNum;

};

#endif // JETOBSWRITER_H