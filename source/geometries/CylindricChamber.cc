// ----------------------------------------------------------------------------
// nexus | CylindricChamber.cc
//
// General-purpose cylindric chamber.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#include "CylindricChamber.h"

#include "PmtR11410.h"
#include "NextNewKDB.h"
#include "MaterialsList.h"
#include "OpticalMaterialProperties.h"
#include "UniformElectricDriftField.h"
#include "IonizationSD.h"
#include "FactoryBase.h"
#include "GenericPhotosensor.h"
#include "NaIScintillator.h"

#include <G4GenericMessenger.hh>
#include <G4Tubs.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4SDManager.hh>

#include <CLHEP/Units/SystemOfUnits.h>
#include <CLHEP/Units/PhysicalConstants.h>


namespace nexus {

  REGISTER_CLASS(CylindricChamber, GeometryBase)

  using namespace CLHEP;

  CylindricChamber::CylindricChamber():
    GeometryBase(), msg_(0)
  {
  }



  CylindricChamber::~CylindricChamber()
  {
    delete msg_;
  }



  void CylindricChamber::Construct()
  {
    // CHAMBER ///////////////////////////////////////////////////////

    const G4double chamber_diam   =  10. * cm;
    const G4double chamber_length = 50. * cm;
    const G4double chamber_thickn =   1. * cm;

    const G4double test_SiPM_size_x_ = 1. * cm; //1.3 mm
    const G4double test_SiPM_size_y_ = 1. * cm; //1.3 mm
    const G4double test_SiPM_size_z_ = 5. * mm; //2.0 mm


    
    G4Tubs* chamber_solid =
      new G4Tubs("CHAMBER", 0., (chamber_diam/2. + chamber_thickn),
        (chamber_length/2. + chamber_thickn), 0., twopi);

    G4LogicalVolume* chamber_logic =
      new G4LogicalVolume(chamber_solid, materials::Steel(), "CHAMBER");

    this->SetLogicalVolume(chamber_logic);
    
    /*
    // GAS ///////////////////////////////////////////////////////////

    G4Tubs* gas_solid =
      new G4Tubs("GAS", 0., chamber_diam/2., chamber_length/2., 0., twopi);

    G4Material* gxe = materials::GXe(10.*bar);
    gxe->SetMaterialPropertiesTable(opticalprops::GXe(10.*bar, 303));

    G4LogicalVolume* gas_logic = new G4LogicalVolume(gas_solid, gxe, "GAS");

    new G4PVPlacement(0, G4ThreeVector(0.,0.,0.), gas_logic, "GAS",
		      chamber_logic, false, 0, true);


    // ACTIVE ////////////////////////////////////////////////////////

    const G4double active_diam   = chamber_diam;
    const G4double active_length = chamber_length/2.;

    G4Tubs* active_solid =
      new G4Tubs("ACTIVE", 0., active_diam/2., active_length/2., 0, twopi);

    G4LogicalVolume* active_logic =
      new G4LogicalVolume(active_solid, gxe, "ACTIVE");

    new G4PVPlacement(0, G4ThreeVector(0.,0.,0.), active_logic, "ACTIVE",
		      gas_logic, false, 0, true);

    // Define this volume as an ionization sensitive detector
    IonizationSD* sensdet = new IonizationSD("/CYLINDRIC_CHAMBER/ACTIVE");
    active_logic->SetSensitiveDetector(sensdet);
    G4SDManager::GetSDMpointer()->AddNewDetector(sensdet);

    // Define an electric drift field for this volume
    UniformElectricDriftField* drift_field = new UniformElectricDriftField();
    drift_field->SetCathodePosition(-active_length/2.);
    drift_field->SetAnodePosition(active_length/2.);
    drift_field->SetDriftVelocity(1.*mm/microsecond);
    drift_field->SetTransverseDiffusion(1.*mm/sqrt(cm));
    drift_field->SetLongitudinalDiffusion(.5*mm/sqrt(cm));

    G4Region* drift_region = new G4Region("DRIFT_REGION");
    drift_region->SetUserInformation(drift_field);
    drift_region->AddRootLogicalVolume(active_logic);


    // EL GAP ////////////////////////////////////////////////////////

    const G4double elgap_diam   = active_diam;
    const G4double elgap_length = 1. * cm;

    G4Tubs* elgap_solid =
      new G4Tubs("EL_GAP", 0., elgap_diam/2., elgap_length/2., 0, twopi);

    G4LogicalVolume* elgap_logic =
      new G4LogicalVolume(elgap_solid, gxe, "EL_GAP");

    G4double pos_z = active_length/2. + elgap_length/2.;

    new G4PVPlacement(0, G4ThreeVector(0.,0.,pos_z), elgap_logic, "EL_GAP",
		      gas_logic, false, 0, true);

    // Define an EL field for this volume
    UniformElectricDriftField* el_field = new UniformElectricDriftField();
    el_field->SetCathodePosition(active_length/2.);
    el_field->SetAnodePosition(active_length/2. + elgap_length);
    el_field->SetDriftVelocity(5.*mm/microsecond);
    el_field->SetTransverseDiffusion(1.*mm/sqrt(cm));
    el_field->SetLongitudinalDiffusion(.5*mm/sqrt(cm));
    el_field->SetLightYield(1000./cm);

    G4Region* el_region = new G4Region("EL_REGION");
    el_region->SetUserInformation(el_field);
    el_region->AddRootLogicalVolume(elgap_logic);


    // PHOTOMULTIPLIER ///////////////////////////////////////////////

    PmtR11410 pmt_geom;
    pmt_geom.SetSensorDepth(0);
    pmt_geom.Construct();
    G4LogicalVolume* pmt_logic = pmt_geom.GetLogicalVolume();

    pos_z = -30. * cm;

    new G4PVPlacement(0, G4ThreeVector(0., 0., pos_z), pmt_logic,
      "PMT", gas_logic, false, 0, true);
    */
    
    GenericPhotosensor * test_SiPM_ = new GenericPhotosensor("test_SiPM_", test_SiPM_size_x_, test_SiPM_size_y_, test_SiPM_size_z_); // (x,y,z)
    
    // Optical Properties of the sensor
    G4MaterialPropertiesTable* photosensor_mpt = new G4MaterialPropertiesTable();
    G4double energy[]       = {0.2 * eV, 3.5 * eV, 3.6 * eV, 11.5 * eV};
    G4double reflectivity[] = {0.0     , 0.0     , 0.0     ,  0.0     };
    G4double efficiency[]   = {1.0     , 1.0     , 0.0     ,  0.0     };
    photosensor_mpt->AddProperty("REFLECTIVITY", energy, reflectivity, 4);
    photosensor_mpt->AddProperty("EFFICIENCY",   energy, efficiency,   4);
    test_SiPM_->SetOpticalProperties(photosensor_mpt);

    // Set WLS coating
    test_SiPM_->SetWithWLSCoating(true);

    // Set time binning
    test_SiPM_->SetTimeBinning(1. * us);

    // Set mother depth & naming order
    test_SiPM_->SetSensorDepth(1);
    test_SiPM_->SetMotherDepth(2);
    test_SiPM_->SetNamingOrder(1);

    // Set visibility
    test_SiPM_->SetVisibility(true);

    // Construct
    test_SiPM_->Construct();
    G4LogicalVolume* test_SiPM_logic = test_SiPM_->GetLogicalVolume();

    /// Placing the TP SiPMs ///
    G4double test_SiPM_pos_z = 0. + test_SiPM_size_z_/2.;
    /*if (verbosity_)
      G4cout << "* SiPM Z positions: " << teflon_iniZ_
	     << " to " << teflon_iniZ_ + test_SiPM_size_z_ << G4endl;*/

    for (G4int i=0; i<1 /*num_SiPMs_*/; i++){
      G4int test_SiPM_id = 0 + i;

      G4ThreeVector sipm_pos = G4ThreeVector(0,0,-10*cm);  //test_SiPM_positions_[i];
      sipm_pos.setZ(test_SiPM_pos_z);
      //this->SetLogicalVolume(test_SiPM_logic);
      new G4PVPlacement(nullptr, sipm_pos, test_SiPM_logic, test_SiPM_logic->GetName(),
	  	      chamber_logic, false, test_SiPM_id, true);
      /*if (sipm_verbosity_) 
        G4cout << "* TP_SiPM " << test_SiPM_id << " position: " 
	       << sipm_pos << G4endl;*/
    }	  

    
    // Build NaI scintillator
    NaIScintillator* test_scint = new NaIScintillator();

    // Construct
    test_scint->Construct();
    G4LogicalVolume* test_scint_logic = test_scint->GetLogicalVolume();

    // place the scintillator
    G4double test_scint_length = test_scint->GetLength();
    G4double test_scint_pos_z = test_SiPM_pos_z + test_scint_length / 2.;
    G4ThreeVector scint_pos = G4ThreeVector(0, 0, -20 * cm);
    scint_pos.setZ(test_scint_pos_z);
    new G4PVPlacement(nullptr, scint_pos, test_scint_logic, "cell0", chamber_logic, false, 0, true);
    

    // DICE BOARD ////////////////////////////////////////////////////
    /*
    NextNewKDB kdb_geom(5,5);
    kdb_geom.Construct();

    G4LogicalVolume* kdb_logic = kdb_geom.GetLogicalVolume();

    pos_z = active_length/2. + elgap_length + 5.0*mm;

    new G4PVPlacement(0, G4ThreeVector(0., 0., pos_z), kdb_logic,
      "KDB", gas_logic, false, 0, true);*/

  }



  G4ThreeVector CylindricChamber::GenerateVertex(const G4String& /*region*/) const
  {
    return G4ThreeVector(0.,0.,0.);
  }


} // end namespace nexus
