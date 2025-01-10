#ifndef JETOBSERVABLESRETRIEVER_H
#define JETOBSERVABLESRETRIEVER_H


#include <edm4hep/ParticleIDCollection.h>
#include <edm4hep/ReconstructedParticleCollection.h>
#include <edm4hep/VertexCollection.h>

#include "Structs.h"


class JetObservablesRetriever {

public:
    double Bz; // magnetic field B in z direction in Tesla

    /**
    * Function that retrieves the input observables for a jet and its constituents. The input observables are the 35 features per constituent that are used as input to the neural network for jet flavor tagging. The function loops over all jet constituents and fills the input observables for each constituent.
    * @param jet: the jet to retrieve the input observables for
    * @param prim_vertex_coll: the primary vertex collection
    * @return: the filled jet object that contains the jet constituents with their input observables
    */
    Jet retrieve_input_observables(const edm4hep::ReconstructedParticle& jet, const edm4hep::VertexCollection& prim_vertex_coll);

private:


    /**
    * Calculate the relative energy of a particle with respect to a jet.
    * @param jet: the jet
    * @param particle: a particle of the jet
    * @return: the relative energy
    */
    float get_relative_erel(const edm4hep::ReconstructedParticle& jet, const edm4hep::ReconstructedParticle& particle);

    /**
    * Calculate the relative angle (phi or theta) of a particle with respect to a jet.
    * @param jet: the jet
    * @param particle: a particle of the jet
    * @param whichangle: "phi" or "theta"
    * @return: the relative angle
    */
    float get_relative_angle(const edm4hep::ReconstructedParticle& jet, const edm4hep::ReconstructedParticle& particle, std::string whichangle);

    /**
    * Fill the track parameters for a neutral particle with dummy values.
    * The value dummy value -9 comes from fast sim https://github.com/HEP-FCC/FCCAnalyses/blob/d39a711a703244ee2902f5d2191ad1e2367363ac/analyzers/dataframe/src/JetConstituentsUtils.cc#L495, however the signicance that is chosen here is -200 to lie outside the distribution. 
    * @param p: the particle object to fill
    */
    void fill_track_params_neutral(Pfcand& p);

    /**
    * Fill the PID flags for a particle.
    * @param p: the particle object to fill
    * @param particle: the particle / jet constituent from which to extract the PID flags
    */
    void pid_flags(Pfcand& p, const edm4hep::ReconstructedParticle& particle);


    /**
    * Fill the covariance matrix for a charged particle.
    * The covariance matrix is a 5 dim matrix, therefore we have 15 distinct values. 
    * On the diagonal it's: d0 = xy, phi, omega = pt, z0, tanLambda = eta.
    * @param p: the particle object to fill
    * @param particle: the particle / jet constituent from which to extract the covariance matrix
    */
    void fill_cov_matrix(Pfcand& p, const edm4hep::ReconstructedParticle& particle);

    /**
    * Get the primary vertex of the event.
    * @param prim_vertex: the primary vertex collection of the event
    * @return: the primary vertex
    */
    const edm4hep::Vector3f get_primary_vertex(const edm4hep::VertexCollection& prim_vertex);

    /**
    * We must extract the helix parametrisation of the track with respect to the PRIMARY VERTEX. 
    * This is done like in: https://github.com/HEP-FCC/FCCAnalyses/blob/63d346103159c4fc88cdee7884e09b3966cfeca4/analyzers/dataframe/src/ReconstructedParticle2Track.cc#L64 
    * @param particle: the charged particle / jet constituent which track should be parametrized
    * @param pv_pos: the primary vertex position of the event
    * @return: helix object filled with track parametrization with respect to the primary vertex
    */
    Helix calculate_helix_params(const edm4hep::ReconstructedParticle& particle, const edm4hep::Vector3f& pv_pos);

    /**
    Calculate the impact parameters of the track with respect to the primary vertex. The helix parametrization of the particle track is with respect to the primary vertex.
    * @param jet: the jet
    * @param particle: the charged particle / jet constituent
    * @param p: the particle object to fill
    * @param h: the helix object with the track parametrization
    */
    void fill_track_IP(const edm4hep::ReconstructedParticle& jet, const edm4hep::ReconstructedParticle& particle, Pfcand& p, Helix& h);

};

#endif // JETOBSERVABLESRETRIEVER_H