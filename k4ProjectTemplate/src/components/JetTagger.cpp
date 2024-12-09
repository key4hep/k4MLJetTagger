/*
 * Copyright (c) 2020-2024 Key4hep-Project.
 *
 * This file is part of Key4hep.
 * See https://key4hep.github.io/key4hep-doc/ for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// #include "JetTagger.h" 

// #include <vector>

//#include "GAUDI_VERSION.h"
//#include <TLorentzVector.h>
//#include "ROOT/RVec.hxx"

#include "TLorentzVector.h"
#include "TVector3.h"

#include "GaudiKernel/MsgStream.h"
#include "k4FWCore/Transformer.h"
#include <edm4hep/ParticleIDCollection.h>
#include <edm4hep/ReconstructedParticleCollection.h>
#include <edm4hep/VertexCollection.h>

#include <torch/torch.h>

#include <random>


// define the structures for jet constituents (pfcand) and jets

struct Pfcand {
  /**
  Structure to store the observables of a particle / jet constituent. These observables will be used as input features to the neural network for jet flavor tagging with the Particle Transformer. For CLD, these are 33 observables including kinematics, PID and track parameters.
  * NOTE: regarding the cov matrix, we use following convenstion compared to FCCAnalysis: 
  * HERE | FCCAnalysis | explanation
  * d0  | xy or dxy | transverse impact parameter
  * phi | dphi| azimuthal angle 
  * tanLambda | ctngtheta or deta or dlambda | lambda is the dip angle of the track in r-z
  * c | dpt | signed curvature of the track.[c] = 1/s. To convert to key4hep convention, multiply by 2/c [1/m]. 
  * z0 | dz | longitudinal impact parameter
  */

  // kinematics
  float pfcand_erel_log, pfcand_thetarel, pfcand_phirel;
  // PID
  int pfcand_type;
  int pfcand_charge;
  int pfcand_isEl, pfcand_isMu, pfcand_isGamma, pfcand_isChargedHad, pfcand_isNeutralHad;

  // track params
  // cov matrix
  float pfcand_cov_cc, pfcand_cov_tanLambdatanLambda, pfcand_cov_phiphi, pfcand_cov_d0d0, pfcand_cov_z0z0; 
  float pfcand_cov_d0z0, pfcand_cov_phid0, pfcand_cov_tanLambdaz0, pfcand_cov_d0c, pfcand_cov_d0tanLambda, pfcand_cov_phic, pfcand_cov_phiz0, pfcand_cov_phitanLambda, pfcand_cov_cz0, pfcand_cov_ctanLambda;
  // IP
  float pfcand_d0, pfcand_z0;
  float pfcand_Sip2dVal, pfcand_Sip2dSig;
  float pfcand_Sip3dVal, pfcand_Sip3dSig;
  float pfcand_JetDistVal, pfcand_JetDistSig;
};

struct Jet {
    std::vector<Pfcand> constituents; 
};

struct Helix{
  /**
  Structure to store the helix parameters of a track. We use following convention (similar to https://github.com/key4hep/EDM4hep/blob/997ab32b886899253c9bc61adea9a21b57bc5a21/edm4hep.yaml#L195C9-L200 ):
  * - d0: transverse impact parameter
  * - phi: azimuthal angle 
  * - c: signed curvature of the track.[c] = 1/s. To convert to key4hep convention, multiply by 2/c [1/m]. 
  * - z0: longitudinal impact parameter
  * - tanLambda: lambda is the dip angle of the track in r-z
  */
  float d0, phi, c, z0, tanLambda;
};

// helper functions

float get_relative_erel(const edm4hep::ReconstructedParticle& jet, const edm4hep::ReconstructedParticle& particle) {
    /**
    * Calculate the relative energy of a particle with respect to a jet.
    * @param jet: the jet
    * @param particle: a particle of the jet
    * @return: the relative energy
    */
    const auto& jet_E = jet.getEnergy();
    const auto& pfcand_E = particle.getEnergy();
    float val = (jet_E > 0.) ? pfcand_E / jet_E : 1.;
    return float(std::log10(val));
}

float get_relative_angle(const edm4hep::ReconstructedParticle& jet, const edm4hep::ReconstructedParticle& particle, std::string whichangle) {
    /**
    * Calculate the relative angle (phi or theta) of a particle with respect to a jet.
    * @param jet: the jet
    * @param particle: a particle of the jet
    * @param whichangle: "phi" or "theta"
    * @return: the relative angle
    */
    TLorentzVector jet_4mom;
    jet_4mom.SetXYZM(jet.getMomentum()[0], jet.getMomentum()[1], jet.getMomentum()[2], jet.getMass());
    TLorentzVector pfcand_4mom;
    pfcand_4mom.SetXYZM(particle.getMomentum()[0], particle.getMomentum()[1], particle.getMomentum()[2], particle.getMass());

    // rotate pfcand_4mom to the jet rest frame
    pfcand_4mom.RotateZ(-jet_4mom.Phi());
    pfcand_4mom.RotateY(-jet_4mom.Theta());

    // if whichangle == "phi", return the phi angle, else return the theta angle
    if (whichangle == "phi") {
        return pfcand_4mom.Phi();
    } else if (whichangle == "theta") {
        return pfcand_4mom.Theta();
    } else {
        throw std::invalid_argument("whichangle must be either 'phi' or 'theta'");
    }
}

Pfcand fill_track_params_neutral(Pfcand p){
  /**
  * Fill the track parameters for a neutral particle with dummy values.
  * The value dummy value -9 comes from fast sim https://github.com/HEP-FCC/FCCAnalyses/blob/d39a711a703244ee2902f5d2191ad1e2367363ac/analyzers/dataframe/src/JetConstituentsUtils.cc#L495, however the signicance that is chosen here is -200 to lie outside the distribution. 
  * @param p: the particle object to fill
  * @return: the particle object with filled track parameters for neutrals
  */

  // cov matrix
  p.pfcand_cov_cc = -9;
  p.pfcand_cov_tanLambdatanLambda = -9;
  p.pfcand_cov_phiphi = -9;
  p.pfcand_cov_d0d0 = -9;
  p.pfcand_cov_z0z0 = -9;
  p.pfcand_cov_d0z0 = -9;
  p.pfcand_cov_phid0 = -9;
  p.pfcand_cov_tanLambdaz0 = -9;
  p.pfcand_cov_d0c = -9;
  p.pfcand_cov_d0tanLambda = -9;
  p.pfcand_cov_phic = -9;
  p.pfcand_cov_phiz0 = -9;
  p.pfcand_cov_phitanLambda = -9;
  p.pfcand_cov_cz0 = -9;
  p.pfcand_cov_ctanLambda = -9;
  // IP
  p.pfcand_d0 = -9;
  p.pfcand_z0 = -9;
  p.pfcand_Sip2dVal = -9;
  p.pfcand_Sip2dSig = -200;
  p.pfcand_Sip3dVal = -9;
  p.pfcand_Sip3dSig = -200;
  p.pfcand_JetDistVal = -9;
  p.pfcand_JetDistSig = -200;
  
  return p;
}

Pfcand pid_flags(Pfcand p, const edm4hep::ReconstructedParticle& particle){
  /**
  * Fill the PID flags for a particle.
  * @param p: the particle object to fill
  * @param particle: the particle / jet constituent from which to extract the PID flags
  * @return: the particle object with filled PID flags
  */

  int n_tracks = particle.getTracks().size();
  int p_type = particle.getPDG();

  int el = 0, mu = 0, gamma = 0, chad = 0, nhad = 0;
  if (p_type == 11 || p_type == -11) {
    el = 1;
  } else if (p_type == 13 || p_type == -13) {
    mu = 1;
  } else if (p_type == 22) {
    gamma = 1;
  } else  {
    if (n_tracks == 1) {
      chad = 1;
    } else if (n_tracks == 0) {
      nhad = 1;
    } else {
      throw std::invalid_argument("Particle has more than one track");
    }
  }

  // PID flags
  p.pfcand_isEl = el;
  p.pfcand_isMu = mu;
  p.pfcand_isGamma = gamma;
  p.pfcand_isChargedHad = chad;
  p.pfcand_isNeutralHad = nhad;

  return p;
}

Pfcand fill_cov_matrix(Pfcand p, const edm4hep::ReconstructedParticle& particle){
  /**
  * Fill the covariance matrix for a charged particle.
  * The covariance matrix is a 5 dim matrix, therefore we have 15 distinct values. 
  * On the diagonal it's: d0 = xy, phi, omega = pt, z0, tanLambda = eta.
  * @param p: the particle object to fill
  * @param particle: the particle / jet constituent from which to extract the covariance matrix
  * @return: the particle object with filled covariance matrix
  */

  // get the track
  auto track = particle.getTracks()[0].getTrackStates()[0]; // get info at interaction point
  // diagonal elements
  p.pfcand_cov_d0d0 = track.covMatrix[0];
  p.pfcand_cov_phiphi = track.covMatrix[2];
  p.pfcand_cov_cc = track.covMatrix[5]; // omega
  p.pfcand_cov_z0z0 = track.covMatrix[9];
  p.pfcand_cov_tanLambdatanLambda = track.covMatrix[14]; // tanLambda
  // off-diagonal elements
  p.pfcand_cov_d0z0 = track.covMatrix[6];
  p.pfcand_cov_phid0 = track.covMatrix[1];
  p.pfcand_cov_tanLambdaz0 = track.covMatrix[13];
  p.pfcand_cov_d0c = track.covMatrix[3];
  p.pfcand_cov_d0tanLambda = track.covMatrix[10];
  p.pfcand_cov_phic = track.covMatrix[4];
  p.pfcand_cov_phiz0 = track.covMatrix[7];
  p.pfcand_cov_phitanLambda = track.covMatrix[11];
  p.pfcand_cov_cz0 = track.covMatrix[8];
  p.pfcand_cov_ctanLambda = track.covMatrix[12];

  return p;
}

const edm4hep::Vector3f get_primary_vertex(const edm4hep::VertexCollection& prim_vertex){
  /**
  * Get the primary vertex of the event.
  * @param prim_vertex: the primary vertex collection of the event
  * @return: the primary vertex
  */

  // get primary vertex
  edm4hep::Vector3f dummy; // A dummy variable to use for initialization
  edm4hep::Vector3f& pv_pos = dummy;
  int i = 0;
  for (const auto& pv : prim_vertex) {
    if (i>0) {
        throw std::invalid_argument("More than one primary vertex found");
    } else {
      pv_pos = pv.getPosition();
      i++;
    }
  }
  if (i==0) {
    throw std::invalid_argument("No primary vertex found");
  }

  return pv_pos;
}

Helix calculate_helix_params(const edm4hep::ReconstructedParticle& particle, const edm4hep::Vector3f& pv_pos){
  /**
  * we must extract the helix parametrisation of the track with respect to the PRIMARY VERTEX. 
  * This is done like in: https://github.com/HEP-FCC/FCCAnalyses/blob/63d346103159c4fc88cdee7884e09b3966cfeca4/analyzers/dataframe/src/ReconstructedParticle2Track.cc#L64 
  * @param particle: the charged particle / jet constituent which track should be parametrized
  * @param pv_pos: the primary vertex position of the event
  * @return: helix object filled with track parametrization with respect to the primary vertex
  */

  // get track
  auto track = particle.getTracks()[0].getTrackStates()[0]; // get info at interaction point
  // get other needed parameters
  const int q = particle.getCharge();
  const edm4hep::Vector3f& p = particle.getMomentum();
  // constants
  const float Bz = 2.0; // magnetic field in T
  const float cSpeed = 2.99792458e8 * 1.0e-9; // speed of light 

  // calculate helpers
  const edm4hep::Vector3f point_on_track = edm4hep::Vector3f(- track.D0 * std::sin(track.phi), track.D0 * std::cos(track.phi), track.Z0); // point on particle track
  const edm4hep::Vector3f x = edm4hep::Vector3f(point_on_track.x - pv_pos.x, point_on_track.y - pv_pos.y, point_on_track.z - pv_pos.z); //point_on_track - pv_pos; // vector from primary vertex to point on track
  const float pt = std::sqrt(p.x*p.x + p.y*p.y); // transverse momentum of particle
  const float a = - q * Bz * cSpeed; // Lorentz force on particle in magnetic field
  const float r2 = x.x*x.x + x.y*x.y;
  const float cross = x.x * p.y - x.y * p.x;
  const float discrim = pt*pt - 2 * a * cross + a*a * r2;

  // helix parameters
  Helix h;

  // calculate d0
  if (discrim>0){
    if (pt<10.0){
      h.d0 = (std::sqrt(discrim) - pt) / a;
    } else {
      h.d0 = (-2 * cross + a * r2) / (std::sqrt(discrim) + pt);
    }
  } else {
    h.d0 = -9;
  }

  // calculate c
  h.c = a/ (2 * pt);

  // calculate z0
  float b = h.c * std::sqrt(std::max(r2 - h.d0*h.d0, float(0))/ (1 + 2 * h.c * h.d0));
  if (std::abs(b)>1){
    b = std::signbit(b);
  }
  const float st = std::asin(b) / h.c;
  const float ct = p.z / pt;
  const float dot = x.x * p.x + x.y * p.y;
  if (dot>0){
    h.z0 = x.z - st * ct;
  } else {
    h.z0 = x.z + st * ct;
  }

  // calculate phi
  h.phi = std::atan2((p.y - a * x.x)/std::sqrt(discrim), (p.x + a * x.y)/std::sqrt(discrim));

  // calculate tanLambda
  h.tanLambda = p.z / pt;

  return h;
}

Pfcand fill_track_IP(const edm4hep::ReconstructedParticle& jet, const edm4hep::ReconstructedParticle& particle, Pfcand p, Helix h, const edm4hep::Vector3f& prim_vertex){
  /**
  Calculate the impact parameters of the track with respect to the primary vertex. The helix parametrization of the particle track is with respect to the primary vertex.
  * @param jet: the jet
  * @param particle: the charged particle / jet constituent
  * @param p: the particle object to fill
  * @param h: the helix object with the track parametrization
  * @return: the particle object with filled impact parameters
  */

  // IP
  p.pfcand_d0 = h.d0;
  p.pfcand_z0 = h.z0;

  // signed IP
  TVector3 jet_p(jet.getMomentum().x, jet.getMomentum().y, jet.getMomentum().z); 
  TVector3 part_p(particle.getMomentum().x, particle.getMomentum().y, particle.getMomentum().z);

  // calculate distance of closest approach in 3d - like in https://github.com/HEP-FCC/FCCAnalyses/blob/d39a711a703244ee2902f5d2191ad1e2367363ac/analyzers/dataframe/src/JetConstituentsUtils.cc#L616-L646 
  TVector3 n = part_p.Cross(jet_p).Unit(); // direction of closest approach
  TVector3 part_pnt(- h.d0 * std::sin(h.phi), h.d0 * std::cos(h.phi), h.z0); // point on particle track
  TVector3 jet_pnt(prim_vertex.x, prim_vertex.y, prim_vertex.z) ; //edm4hep::Vector3f(0,0,0); // point on jet - THIS DIFFERS FROM THE FCCANALYSIS IMPLEMENTATION
  const float d_3d = n.Dot(part_pnt - jet_pnt); // distance of closest approach
  p.pfcand_JetDistVal = d_3d;

  // calculate signed 2D impact parameter - like in https://github.com/HEP-FCC/FCCAnalyses/blob/d39a711a703244ee2902f5d2191ad1e2367363ac/analyzers/dataframe/src/JetConstituentsUtils.cc#L392-L419 
  const float sip2d = std::copysign(std::abs(h.d0), part_pnt.X() * jet_p.X() + part_pnt.Y() * jet_p.Y()); // dot product between part and jet in 2D: if angle between track and jet greater 90 deg -> negative sign; if smaller 90 deg -> positive sign
  p.pfcand_Sip2dVal = sip2d;
  p.pfcand_Sip2dSig = (p.pfcand_cov_d0d0 > 0) ? (sip2d / std::sqrt(p.pfcand_cov_d0d0)) : -999;

  // calculate signed 3D impact parameter - like in https://github.com/HEP-FCC/FCCAnalyses/blob/d39a711a703244ee2902f5d2191ad1e2367363ac/analyzers/dataframe/src/JetConstituentsUtils.cc#L503-L531 
  const float IP_3d = std::sqrt(h.d0*h.d0 + h.z0*h.z0);
  const float sip3d = std::copysign(std::abs(IP_3d),  part_pnt.Dot(jet_p)); // dot product between part in jet in 3d: if angle between track and jet greater 90 deg -> negative sign; if smaller 90 deg -> positive sign
  p.pfcand_Sip3dVal = sip3d;

  // significance in 3D
  const float in_sqrt = p.pfcand_cov_d0d0 + p.pfcand_cov_z0z0; 
  if (in_sqrt>0){ // can caluclate sqrt?
    const float err3d = std::sqrt(in_sqrt); // error of distance of closest approach
    p.pfcand_JetDistSig = d_3d/err3d; 
    p.pfcand_Sip3dSig = sip3d/err3d; 
  } else { // handle error -> dummy value
    p.pfcand_JetDistSig = -999;
    p.pfcand_Sip3dSig = -999;
  }

  return p;
}

Jet retrieve_input_observables(const edm4hep::ReconstructedParticle& jet, const edm4hep::VertexCollection& prim_vertex_coll, MsgStream& log) {
    /**
    * Function that retrieves the input observables for a jet and its constituents. The input observables are the 33 features per constituent that are used as input to the neural network for jet flavor tagging. The function loops over all jet constituents and fills the input observables for each constituent.
    * @param jet: the jet to retrieve the input observables for
    * @param prim_vertex_coll: the primary vertex collection
    * @param log: the message stream
    * @return: the filled jet object that contains the jet constituents with their input observables
    */

    // Create a jet object
    Jet j;
    const edm4hep::Vector3f prim_vertex = get_primary_vertex(prim_vertex_coll);

    // loop over all jet constituents and retrieve 33 input features to the network
    for (const auto& particle : jet.getParticles()) {

      // create a particle object
      Pfcand p;

      // kinematics
      p.pfcand_erel_log = get_relative_erel(jet, particle);
      p.pfcand_phirel = get_relative_angle(jet, particle, "phi");
      p.pfcand_thetarel = get_relative_angle(jet, particle, "theta");

      // PID
      p.pfcand_type = particle.getPDG(); // new; deprecated: get.Type() method
      p.pfcand_charge = particle.getCharge();
      p = pid_flags(p, particle);

      // track parameters
      int n_tracks = particle.getTracks().size();
      if (n_tracks == 1) { // charged particle
        p = fill_cov_matrix(p, particle); // covariance matrix
        Helix h = calculate_helix_params(particle, prim_vertex); // calculate track parameters described by a helix parametrization
        p = fill_track_IP(jet, particle, p, h, prim_vertex); // impact parameters
      } else if (n_tracks == 0) { // neutral particle
        p = fill_track_params_neutral(p);
      } else {
        throw std::invalid_argument("Particle has more than one track");
      }

      // add the pfcand to the jet
      j.constituents.push_back(p);
    }

    return j;
}

int tagger(Jet jet){
  /**
  * Function that takes a jet and returns a tag value. This function is a dummy function that returns a random tag value for demonstration purposes.
  * @param jet: the jet to tag
  * @return: the tag value
  */
  // Create a random number generator for demonstration purposes.
  static std::mt19937 rng(42);  // Seed for reproducibility
  static std::uniform_int_distribution<int> dist(0, 6);  // Assume 7 jet flavors: 0 to 6

  // Generate a random tag value for the jet.
  int tagValue = dist(rng);

  return tagValue;
}


// main function
struct JetTagger
    : k4FWCore::Transformer<edm4hep::ParticleIDCollection(const edm4hep::ReconstructedParticleCollection&, const edm4hep::VertexCollection& )> {
  JetTagger(const std::string& name, ISvcLocator* svcLoc)
    : Transformer(name, svcLoc, 
                  {
                    KeyValues("InputJets", {"RefinedVertexJets"}),
                    KeyValues("InputPrimaryVertices", {"PrimaryVertices"})
                  },
                  {KeyValues("OutputIDCollections", {"RefinedJetTags"})}
                  ) {}

  edm4hep::ParticleIDCollection operator()(const edm4hep::ReconstructedParticleCollection& inputJets, const edm4hep::VertexCollection& primVerticies) const override{
    info() << "Tagging " << inputJets.size() << " input jets" << endmsg;
    auto tagCollection = edm4hep::ParticleIDCollection();

    for (const auto& jet : inputJets) {
      Jet j = retrieve_input_observables(jet, primVerticies, info());
      int tagValue = tagger(j);  

      // Handle tag collection
      auto jetTag = tagCollection.create();
      jetTag.setParticle(jet);
      jetTag.setType(tagValue); // maybe alter this
    }

    return tagCollection;
  };
};

DECLARE_COMPONENT(JetTagger)
