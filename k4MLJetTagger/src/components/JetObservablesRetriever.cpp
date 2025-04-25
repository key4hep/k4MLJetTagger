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
#include "TLorentzVector.h"
#include "TVector3.h"

#include "JetObservablesRetriever.h"

// public function

Jet JetObservablesRetriever::retrieve_input_observables(const edm4hep::ReconstructedParticle& jet,
                                                        const edm4hep::VertexCollection& prim_vertex_coll) {
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

    p.pfcand_e = particle.getEnergy();
    p.pfcand_p = std::sqrt(particle.getMomentum().x * particle.getMomentum().x +
                           particle.getMomentum().y * particle.getMomentum().y +
                           particle.getMomentum().z * particle.getMomentum().z);

    // PID
    p.pfcand_type = particle.getPDG(); // new; deprecated: get.Type() method
    p.pfcand_charge = particle.getCharge();
    pid_flags(p, particle);
    p.pfcand_dndx = 0; // dummy, filled with 0
    p.pfcand_tof = 0;  // dummy, filled with 0

    // track parameters
    int n_tracks = particle.getTracks().size();
    if (n_tracks == 1) {            // charged particle
      fill_cov_matrix(p, particle); // covariance matrix
      Helix h = calculate_helix_params(particle,
                                       prim_vertex); // calculate track parameters described by a helix parametrization
      fill_track_IP(jet, particle, p, h);            // impact parameters
    } else if (n_tracks == 0) {                      // neutral particle
      fill_track_params_neutral(p);
    } else {
      throw std::invalid_argument("Particle has more than one track");
    }

    // p.print_values();

    // add the pfcand to the jet
    j.constituents.push_back(p);
  }

  return j;
}

// private functions

float JetObservablesRetriever::get_relative_erel(const edm4hep::ReconstructedParticle& jet,
                                                 const edm4hep::ReconstructedParticle& particle) {
  const auto& jet_E = jet.getEnergy();
  const auto& pfcand_E = particle.getEnergy();
  float val = (jet_E > 0.) ? pfcand_E / jet_E : 1.;
  return float(std::log10(val));
}

float JetObservablesRetriever::get_relative_angle(const edm4hep::ReconstructedParticle& jet,
                                                  const edm4hep::ReconstructedParticle& particle,
                                                  std::string whichangle) {
  TLorentzVector jet_4mom;
  jet_4mom.SetXYZM(jet.getMomentum()[0], jet.getMomentum()[1], jet.getMomentum()[2], jet.getMass());
  TLorentzVector pfcand_4mom;
  pfcand_4mom.SetXYZM(particle.getMomentum()[0], particle.getMomentum()[1], particle.getMomentum()[2],
                      particle.getMass());

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

void JetObservablesRetriever::fill_track_params_neutral(Pfcand& p) {
  // cov matrix
  p.pfcand_cov_omegaomega = -9;
  p.pfcand_cov_tanLambdatanLambda = -9;
  p.pfcand_cov_phiphi = -9;
  p.pfcand_cov_d0d0 = -9;
  p.pfcand_cov_z0z0 = -9;
  p.pfcand_cov_d0z0 = -9;
  p.pfcand_cov_phid0 = -9;
  p.pfcand_cov_tanLambdaz0 = -9;
  p.pfcand_cov_d0omega = -9;
  p.pfcand_cov_d0tanLambda = -9;
  p.pfcand_cov_phiomega = -9;
  p.pfcand_cov_phiz0 = -9;
  p.pfcand_cov_phitanLambda = -9;
  p.pfcand_cov_omegaz0 = -9;
  p.pfcand_cov_omegatanLambda = -9;
  // IP
  p.pfcand_d0 = -9;
  p.pfcand_z0 = -9;
  p.pfcand_Sip2dVal = -9;
  p.pfcand_Sip2dSig = -200;
  p.pfcand_Sip3dVal = -9;
  p.pfcand_Sip3dSig = -200;
  p.pfcand_JetDistVal = -9;
  p.pfcand_JetDistSig = -200;
}

void JetObservablesRetriever::pid_flags(Pfcand& p, const edm4hep::ReconstructedParticle& particle) {
  int n_tracks = particle.getTracks().size();
  int p_type = particle.getPDG();

  int el = 0, mu = 0, gamma = 0, chad = 0, nhad = 0;
  if (p_type == 11 || p_type == -11) {
    el = 1;
  } else if (p_type == 13 || p_type == -13) {
    mu = 1;
  } else if (p_type == 22) {
    gamma = 1;
  } else {
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
}

void JetObservablesRetriever::fill_cov_matrix(Pfcand& p, const edm4hep::ReconstructedParticle& particle) {
  // approximation because this is wrt to (0,0,0) and not wrt to the primary vertex
  // get the track
  auto track = particle.getTracks()[0].getTrackStates()[0]; // get info at interaction point
  // diagonal elements
  p.pfcand_cov_d0d0 = track.covMatrix[0];
  p.pfcand_cov_phiphi = track.covMatrix[2];
  p.pfcand_cov_omegaomega = track.covMatrix[5]; // omega
  p.pfcand_cov_z0z0 = track.covMatrix[9];
  p.pfcand_cov_tanLambdatanLambda = track.covMatrix[14]; // tanLambda
  // off-diagonal elements
  p.pfcand_cov_d0z0 = track.covMatrix[6];
  p.pfcand_cov_phid0 = track.covMatrix[1];
  p.pfcand_cov_tanLambdaz0 = track.covMatrix[13];
  p.pfcand_cov_d0omega = track.covMatrix[3];
  p.pfcand_cov_d0tanLambda = track.covMatrix[10];
  p.pfcand_cov_phiomega = track.covMatrix[4];
  p.pfcand_cov_phiz0 = track.covMatrix[7];
  p.pfcand_cov_phitanLambda = track.covMatrix[11];
  p.pfcand_cov_omegaz0 = track.covMatrix[8];
  p.pfcand_cov_omegatanLambda = track.covMatrix[12];
}

const edm4hep::Vector3f JetObservablesRetriever::get_primary_vertex(const edm4hep::VertexCollection& prim_vertex) {
  // get primary vertex
  edm4hep::Vector3f dummy; // A dummy variable to use for initialization
  edm4hep::Vector3f& pv_pos = dummy;
  int i = 0;
  for (const auto& pv : prim_vertex) {
    if (i > 0) {
      throw std::invalid_argument("More than one primary vertex found");
    } else {
      pv_pos = pv.getPosition();
      i++;
    }
  }
  if (i == 0) {
    // throw std::invalid_argument("No primary vertex found");
    std::cout << "JetObservablesRetriever: WARNING - No primary vertex found. Set it to (0,0,0)" << std::endl;
    pv_pos = edm4hep::Vector3f(0, 0, 0);
  }

  return pv_pos;
}

Helix JetObservablesRetriever::calculate_helix_params(const edm4hep::ReconstructedParticle& particle,
                                                      const edm4hep::Vector3f& pv_pos) {
  // get track
  auto track = particle.getTracks()[0].getTrackStates()[0]; // get info at interaction point
  // get other needed parameters
  const int q = particle.getCharge();
  const edm4hep::Vector3f& p = particle.getMomentum();
  // constants
  const float cSpeed = 2.99792458e8 * 1.0e-9; // speed of light; 10^-9 comes from GeV of momentum

  // calculate helpers
  const edm4hep::Vector3f point_on_track = edm4hep::Vector3f(
      -track.D0 * std::sin(track.phi), track.D0 * std::cos(track.phi), track.Z0); // point on particle track
  const edm4hep::Vector3f x = edm4hep::Vector3f(
      point_on_track.x - pv_pos.x, point_on_track.y - pv_pos.y,
      point_on_track.z - pv_pos.z); // point_on_track - pv_pos; // vector from primary vertex to point on track
  const float pt = std::sqrt(p.x * p.x + p.y * p.y); // transverse momentum of particle
  const float a = -q * Bz * cSpeed;                  // Lorentz force on particle in magnetic field
  const float r2 = x.x * x.x + x.y * x.y;
  const float cross = x.x * p.y - x.y * p.x;
  const float discrim = pt * pt - 2 * a * cross + a * a * r2;

  // helix parameters wrt to primary vertex
  Helix h;
  h.omega = track.omega; // curvature [1/mm] does not change with respect to primary vertex

  // calculate d0
  if (discrim > 0) {
    if (pt < 10.0) {
      h.d0 = (std::sqrt(discrim) - pt) / a;
    } else {
      h.d0 = (-2 * cross + a * r2) / (std::sqrt(discrim) + pt);
    }
  } else {
    h.d0 = -9;
  }

  // calculate c - conviently to calculate with but to get omega in [1/mm]: omega = c*10**(-3) * (-1), see
  // https://github.com/HEP-FCC/FCCAnalyses/blob/pre-edm4hep1/analyzers/dataframe/src/ReconstructedParticle2Track.cc#L194-L217
  const float curv = a / (2 * pt);

  // calculate z0
  float b = curv * std::sqrt(std::max(r2 - h.d0 * h.d0, float(0)) / (1 + 2 * curv * h.d0));
  if (std::abs(b) > 1) {
    b = std::signbit(b);
  }
  const float st = std::asin(b) / curv;
  const float ct = p.z / pt;
  const float dot = x.x * p.x + x.y * p.y;
  if (dot > 0) {
    h.z0 = x.z - st * ct;
  } else {
    h.z0 = x.z + st * ct;
  }

  // calculate phi
  h.phi = std::atan2((p.y - a * x.x) / std::sqrt(discrim), (p.x + a * x.y) / std::sqrt(discrim));

  // calculate tanLambda
  h.tanLambda = p.z / pt;

  return h;
}

void JetObservablesRetriever::fill_track_IP(const edm4hep::ReconstructedParticle& jet,
                                            const edm4hep::ReconstructedParticle& particle, Pfcand& p, Helix& h) {
  // IP
  p.pfcand_d0 = h.d0;
  p.pfcand_z0 = h.z0;

  // signed IP
  TVector3 jet_p(
      jet.getMomentum().x, jet.getMomentum().y,
      jet.getMomentum().z); // for neutrals: wrt (0,0,0); for charged: track momentum at closest approach to (0,0,0)
  TVector3 part_p(particle.getMomentum().x, particle.getMomentum().y, particle.getMomentum().z); // same

  // calculate distance of closest approach in 3d - like in
  // https://github.com/HEP-FCC/FCCAnalyses/blob/d39a711a703244ee2902f5d2191ad1e2367363ac/analyzers/dataframe/src/JetConstituentsUtils.cc#L616-L646
  TVector3 n = part_p.Cross(jet_p).Unit(); // direction of closest approach; wrt to (0,0,0)
  TVector3 part_pnt(-h.d0 * std::sin(h.phi), h.d0 * std::cos(h.phi),
                    h.z0);                      // point on particle track; wrt to PV NOT (0,0,0) - not correct
  TVector3 jet_pnt(0, 0, 0);                    // point on jet
  const float d_3d = n.Dot(part_pnt - jet_pnt); // distance of closest approach
  p.pfcand_JetDistVal = d_3d;

  // calculate signed 2D impact parameter - like in //
  // https://github.com/HEP-FCC/FCCAnalyses/blob/d39a711a703244ee2902f5d2191ad1e2367363ac/analyzers/dataframe/src/JetConstituentsUtils.cc#L450-L475
  // approximation bc part_pnt is wrt to PV and jet_p is wrt to (0,0,0)
  const float sip2d = std::copysign(
      std::abs(h.d0),
      part_pnt.X() * jet_p.X() +
          part_pnt.Y() * jet_p.Y()); // dot product between part and jet in 2D: if angle between track and jet greater
                                     // 90 deg -> negative sign; if smaller 90 deg -> positive sign
  p.pfcand_Sip2dVal = sip2d;
  p.pfcand_Sip2dSig = (p.pfcand_cov_d0d0 > 0) ? (sip2d / std::sqrt(p.pfcand_cov_d0d0)) : -999;

  // calculate signed 3D impact parameter - like in
  // https://github.com/HEP-FCC/FCCAnalyses/blob/d39a711a703244ee2902f5d2191ad1e2367363ac/analyzers/dataframe/src/JetConstituentsUtils.cc#L503-L531
  const float IP_3d = std::sqrt(h.d0 * h.d0 + h.z0 * h.z0);
  const float sip3d =
      std::copysign(std::abs(IP_3d),
                    part_pnt.Dot(jet_p)); // dot product between part in jet in 3d: if angle between track and jet
                                          // greater 90 deg -> negative sign; if smaller 90 deg -> positive sign
  p.pfcand_Sip3dVal = sip3d;

  // significance in 3D
  const float in_sqrt = p.pfcand_cov_d0d0 + p.pfcand_cov_z0z0;
  if (in_sqrt > 0) {                        // can caluclate sqrt?
    const float err3d = std::sqrt(in_sqrt); // error of distance of closest approach
    p.pfcand_JetDistSig = d_3d / err3d;
    p.pfcand_Sip3dSig = sip3d / err3d;
  } else { // handle error -> dummy value
    p.pfcand_JetDistSig = -999;
    p.pfcand_Sip3dSig = -999;
  }
}
