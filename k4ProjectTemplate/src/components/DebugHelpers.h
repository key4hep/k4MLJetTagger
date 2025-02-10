#ifndef DEBUGHELPERS_H
#define DEBUGHELPERS_H

#include "edm4hep/MCParticleCollection.h"

/**
* Find one of the MC quark in H->qq and return it.
*/
edm4hep::MCParticle get_MC_quark(const edm4hep::MCParticleCollection& mc_coll);


#endif // DEBUGHELPERS_H