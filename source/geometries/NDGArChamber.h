// ----------------------------------------------------------------------------
// nexus | NDGArChamber.h
//
// General-purpose cylindric chamber.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#ifndef NDGAR_CHAMBER_H
#define NDGAR_CHAMBER_H

#include "GeometryBase.h"

class G4GenericMessenger;


namespace nexus {

  class NDGArChamber: public GeometryBase
  {
  public:
    /// Constructor
    NDGArChamber();
    /// Destructor
    ~NDGArChamber();

    /// Return vertex within region <region> of the chamber
    virtual G4ThreeVector GenerateVertex(const G4String& region) const;

    virtual void Construct();

  private:
    /// Messenger for the definition of control commands
    G4GenericMessenger* msg_;
  };

} // end namespace nexus

#endif
