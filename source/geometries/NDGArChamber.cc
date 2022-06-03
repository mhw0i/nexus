// ----------------------------------------------------------------------------
// nexus | NDGArChamber.cc
//
// General-purpose NDGAr chamber.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#include <math.h>

#include "NDGArChamber.h"

#include "PmtR11410.h"
#include "NextNewKDB.h"
#include "MaterialsList.h"
#include "OpticalMaterialProperties.h"
#include "UniformElectricDriftField.h"
#include "IonizationSD.h"
#include "FactoryBase.h"
#include "GenericPhotosensor.h"
#include "NaIScintillator.h"
#include "Visibilities.h"

#include <G4Box.hh>
#include <G4NistManager.hh>
#include <G4GenericMessenger.hh>
#include <G4Tubs.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4SDManager.hh>
#include <G4OpticalSurface.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4RandomDirection.hh>

#include <CLHEP/Units/SystemOfUnits.h>
#include <CLHEP/Units/PhysicalConstants.h>

#include "TString.h"


namespace nexus {

  REGISTER_CLASS(NDGArChamber, GeometryBase)

  using namespace CLHEP;

  G4ThreadLocal G4UniformMagField* NDGArChamber::magnetic_field_ = nullptr;
  G4ThreadLocal G4FieldManager* NDGArChamber::field_mgr_ = nullptr;

  NDGArChamber::NDGArChamber():
    GeometryBase(), msg_(0)
  {
  }



  NDGArChamber::~NDGArChamber()
  {
    delete msg_;
  }



  void NDGArChamber::Construct()
  {
    // CHAMBER ///////////////////////////////////////////////////////

    const G4double chamber_diam   =  10. * m;
    const G4double chamber_length = 10. * m;

    const G4int n_cells = 4;

    const G4double SiPM_cell_x = 6 * mm;
    const G4double SiPM_cell_y = 6 * mm;
    const G4double SiPM_cell_z = 2. * mm;

    const G4double test_SiPM_size_x_ = n_cells * SiPM_cell_x ; //1.3 mm
    const G4double test_SiPM_size_y_ = n_cells * SiPM_cell_y; //1.3 mm
    const G4double test_SiPM_size_z_ = SiPM_cell_z; //2.0 mm


    
    G4Box* chamber_solid = new G4Box("CHAMBER", 0.5*10.*m, 0.5*10*m, 0.5*10*m);
      //new G4Tubs("CHAMBER", 0., chamber_diam/2.,
      //  chamber_length/2., 0., twopi);

    G4NistManager* nist = G4NistManager::Instance();
    G4Material* chamber_mat = nist->FindOrBuildMaterial("G4_AIR");

    G4LogicalVolume* chamber_logic =
      new G4LogicalVolume(chamber_solid, chamber_mat, "CHAMBER");

    this->SetLogicalVolume(chamber_logic);
    
    G4double mother_gar_diam = 8. * m;
    G4double mother_gar_z = 8. * m;

    G4Tubs* mother_gar_solid = new G4Tubs("MOTHER_GAR", 0., mother_gar_diam/2., mother_gar_z/2., 0., twopi);
    G4Material* gar_mat = materials::GAr(10.*bar); //standard pressure and temperature as default, change pressure to 10 bar and temp to -30°C later
    auto argon_mpt = opticalprops::GAr(0./MeV);
    gar_mat->SetMaterialPropertiesTable(argon_mpt);

    G4LogicalVolume* mother_gar_logic = new G4LogicalVolume(mother_gar_solid, gar_mat, "MOTHER_GAr");
    G4ThreeVector mother_gar_pos = G4ThreeVector(0,0,0);
    new G4PVPlacement(nullptr, mother_gar_pos, mother_gar_logic, "PV_MOTHER_GAr", chamber_logic, false, 0, true);


    // SET MAGNETIC FIELD
    magnetic_field_ = new G4UniformMagField(G4ThreeVector(0.0, 0.5*tesla, 0.0));
    field_mgr_ = new G4FieldManager();
    field_mgr_->SetDetectorField(magnetic_field_);
    field_mgr_->CreateChordFinder(magnetic_field_);
    G4bool forceToAllDaughters = true;
    mother_gar_logic->SetFieldManager(field_mgr_, forceToAllDaughters);
    
    G4double gar_diam = 5. * m;
    G4double gar_z = 5. * m;
    
    G4double tefl_wrap_diam = gar_diam + 5. * cm;
    G4double tefl_wrap_z = gar_z;

    G4double tefl_bot_diam = tefl_wrap_diam;
    G4double tefl_bot_z = 5. * cm;
    

    //build teflon wrapping

    G4Tubs* tefl_wrap_solid = new G4Tubs("TEFLON_WRAPPING", 0, tefl_wrap_diam/2., tefl_wrap_z/2., 0., twopi);
    G4Material* tefl_wrap_mat = nist->FindOrBuildMaterial("G4_TEFLON");
    G4LogicalVolume* tefl_wrap_logic = new G4LogicalVolume(tefl_wrap_solid, tefl_wrap_mat, "TEFLON_WRAPPING");

    G4ThreeVector tefl_wrap_pos = G4ThreeVector(0,0,0); //middle of volume
    new G4PVPlacement(nullptr, tefl_wrap_pos, tefl_wrap_logic, "PV_TEFLON_WRAPPING", mother_gar_logic, false, 0, true);

    // add optical surface to teflon wrapping
    G4OpticalSurface* tefl_wrap_optSurf = new G4OpticalSurface("TEFLON_WRAPPING_SURF", unified, ground, dielectric_metal);
    tefl_wrap_optSurf->SetMaterialPropertiesTable(opticalprops::PTFE());

    new G4LogicalSkinSurface("TEFLON_WRAPPING_SURF", tefl_wrap_logic, tefl_wrap_optSurf);

    //build teflon bottom cover
    /*
    G4Tubs* tefl_bot_solid = new G4Tubs("TEFLON_BOTTOM", 0, tefl_bot_diam/2., tefl_bot_z/2., 0., twopi);
    G4Material* tefl_bot_mat = nist->FindOrBuildMaterial("G4_TEFLON");
    G4LogicalVolume* tefl_bot_logic = new G4LogicalVolume(tefl_bot_solid, tefl_bot_mat, "TEFLON_BOTTOM");

    G4ThreeVector tefl_bot_pos = G4ThreeVector(0,0,tefl_wrap_z/2.+tefl_bot_z/2.);
    new G4PVPlacement(nullptr, tefl_bot_pos, tefl_bot_logic, "PV_TEFLON_BOTTOM", mother_gar_logic, false, 0, true);

    //add optical surface to teflon bottom cover
    G4OpticalSurface* tefl_bot_optSurf = new G4OpticalSurface("TEFLON_BOTTOM_SURF", unified, ground, dielectric_metal);
    tefl_bot_optSurf->SetMaterialPropertiesTable(opticalprops::PTFE());

    new G4LogicalSkinSurface("TEFLON_BOTTOM_SURF", tefl_bot_logic, tefl_bot_optSurf);
    */

    // Build active GAr volume

    G4Tubs* gar_solid = new G4Tubs("GAr", 0., gar_diam / 2., gar_z/2., 0., twopi);
    
    G4double theYield = 290; // value taken from Secondary scintillation yield in pure argon by C.M.B. Monteiro, J.A.M. Lopes et al. (Physics Letters B vol 668, issue 3, 9 Oct 2008)
    G4double theELifetime = 1.; //temporary value
    //gar_mat->SetMaterialPropertiesTable(opticalprops::GAr(theYield, theELifetime));
    
    G4LogicalVolume* gar_logic = new G4LogicalVolume(gar_solid, gar_mat, "GAr");

    G4ThreeVector gar_pos = G4ThreeVector(0, 0, 0); // middle of volume
    new G4PVPlacement(nullptr, gar_pos, gar_logic, "PV_GAr", tefl_wrap_logic, false, 0, true);


    // Define this volume as an ionization sensitive detector
    IonizationSD* sensdet = new IonizationSD("/NDGAR/ACTIVE");
    gar_logic->SetSensitiveDetector(sensdet);
    G4SDManager::GetSDMpointer()->AddNewDetector(sensdet);

    // Define an electric drift field for this volume
    UniformElectricDriftField* drift_field = 
      new UniformElectricDriftField(); //gar_z/2., -gar_z/2., kZAxis);
    drift_field->SetAnodePosition(gar_z/2.);
    drift_field->SetCathodePosition(-gar_z/2.);
    drift_field->SetDriftVelocity(1.*mm/microsecond);
    drift_field->SetTransverseDiffusion(.1*mm/sqrt(cm));
    drift_field->SetLongitudinalDiffusion(.1*mm/sqrt(cm));

    G4Region* drift_region = new G4Region("DRIFT_REGION");
    drift_region->SetUserInformation(drift_field);
    drift_region->AddRootLogicalVolume(gar_logic);

    // EL GAP ////////////////////////////////////////////////////////

    const G4double elgap_diam   = gar_diam;
    const G4double elgap_length = 1. * cm;

    G4Tubs* elgap_solid_vol =
      new G4Tubs("EL_GAP", 0., elgap_diam/2., elgap_length/2., 0, twopi);

    G4LogicalVolume* elgap_logic_vol =
      new G4LogicalVolume(elgap_solid_vol, gar_mat, "EL_GAP");

    G4double elgap_pos_z = gar_z/2. + elgap_length/2.;

    new G4PVPlacement(0, G4ThreeVector(0.,0.,elgap_pos_z), elgap_logic_vol, "EL_GAP",
		      mother_gar_logic, false, 0, true);

    //Define an EL field for this volume
    UniformElectricDriftField* el_field = 
      new UniformElectricDriftField(gar_z/2., gar_z/2.+elgap_length, kZAxis);
    el_field->SetDriftVelocity(5.*mm/microsecond);
    //el_field->SetDriftVelocity(1000.*cm/microsecond);
    //el_field->SetTransverseDiffusion(1.*mm/sqrt(cm));
    //el_field->SetLongitudinalDiffusion(.5*mm/sqrt(cm));
    el_field->SetLightYield(100./cm);

    G4Region* el_region = new G4Region("EL_REGION");
    el_region->SetUserInformation(el_field);
    el_region->AddRootLogicalVolume(elgap_logic_vol);

    // build SiPM plane
    
    GenericPhotosensor * test_SiPM_ = new GenericPhotosensor("test_SiPM_", test_SiPM_size_x_, test_SiPM_size_y_, test_SiPM_size_z_); // (x,y,z)
    test_SiPM_->SetWithWLSCoating(false);
    test_SiPM_->SetWindowRefractiveIndex(argon_mpt->GetProperty("RINDEX"));
    
    // Optical Properties of the sensor
    G4MaterialPropertiesTable* photosensor_mpt = new G4MaterialPropertiesTable();
    G4double energy[]       = {0.2 * eV, 3.5 * eV, 3.6 * eV, 11.5 * eV};
    G4double reflectivity[] = {0.0     , 0.0     , 0.0     ,  0.0     };
    G4double efficiency[]   = {1.0     , 1.0     , 1.0     ,  1.0     };
    photosensor_mpt->AddProperty("REFLECTIVITY", energy, reflectivity, 4);
    photosensor_mpt->AddProperty("EFFICIENCY",   energy, efficiency,   4);
    test_SiPM_->SetOpticalProperties(photosensor_mpt);

    // Set WLS coating
    
    

    // Set time binning
    test_SiPM_->SetTimeBinning(.001 * us);

    // Set mother depth & naming order
    test_SiPM_->SetSensorDepth(1);
    test_SiPM_->SetMotherDepth(0); // sets the "board" -> 1000, 2000, 3000, ...
    test_SiPM_->SetNamingOrder(0); // sets a "unit" of numbers of sensors per board
    //test_SiPM_->SetMotherDepth(2);
    //test_SiPM_->SetNamingOrder(1); // i think here something needs to be changed. Also, is SetDetectorVolumeDepth needed?

    // sensor depth: because sens area is inside the actual object "genericphotonsensor" so we tell geant to check one hierarchy above

    // Set visibility
    test_SiPM_->SetVisibility(true);

    // Construct
    test_SiPM_->Construct();
    G4LogicalVolume* test_SiPM_logic = test_SiPM_->GetLogicalVolume();
    G4double theOffset = 1. * mm;
    G4double sipm_spacing_x = test_SiPM_size_x_ + theOffset;
    G4double sipm_spacing_y = test_SiPM_size_y_ + theOffset;

    /// Placing the TP SiPMs ///
    G4int N_x = (G4int)(gar_diam / (sipm_spacing_x));
    G4int N_y = (G4int)(gar_diam / (sipm_spacing_y));
    G4int N = N_x * N_y;
    G4double sipm_pos_pv_x[N_x-1];
    G4double sipm_pos_pv_y[N_y-1];

    std::cout << "number of cells in x direction: " << N_x << std::endl;
    std::cout << "number of cells in y direction: " << N_y << std::endl;

    for (G4int i = 0; i < N_x-1; i++) {
        sipm_pos_pv_x[i] = (i-(N_x-1)/2.)*sipm_spacing_x+sipm_spacing_x/2.;
    }
    for (G4int i = 0; i < N_y-1; i++) {
        sipm_pos_pv_y[i] = (i-(N_y-1)/2.)*sipm_spacing_y+sipm_spacing_y/2.;
    }
    
    /*
    for (G4int i=0; i < N_x-1; i++) {
      for (G4int j = 0; j < N_y-1; j++) {
        if(i%100==0 || j%100==0) std::cout << sipm_pos_pv_x[i]/mm << ", " << sipm_pos_pv_y[j]/mm << " | ";
      }
    std::cout << std::endl;
    }*/



    G4int test_SiPM_id = 0;

    /*if (verbosity_)
      G4cout << "* SiPM Z positions: " << teflon_iniZ_
	     << " to " << teflon_iniZ_ + test_SiPM_size_z_ << G4endl;*/
    
    G4double sipm_full_size;
    for (G4int i=0; i < N_x-1; i++) {
      //std::cout << "This is the " << i << "-th iteration of the i loop." << std::endl;
      for (G4int j = 0; j < N_y-1; j++) {
        //std::cout << "This is the " << j << "-th iteration of the j loop." << std::endl;
        //sipm_full_size = std::sqrt(std::pow(std::abs(sipm_pos_pv_x[i]) + std::abs(test_SiPM_size_x_), 2) + std::pow(std::abs(sipm_pos_pv_y[i]) + std::abs(test_SiPM_size_y_), 2));
        sipm_full_size = std::sqrt(std::pow(std::abs(sipm_pos_pv_x[i]), 2) + std::pow(std::abs(sipm_pos_pv_y[j]), 2));
        if (sipm_full_size < gar_diam / 2.) {
          G4ThreeVector sipm_pos = G4ThreeVector(sipm_pos_pv_x[i], sipm_pos_pv_y[j], - gar_z / 2. - elgap_length/2. - test_SiPM_size_z_/2. - 1.*cm);
          new G4PVPlacement(nullptr, sipm_pos, test_SiPM_logic, test_SiPM_logic->GetName(),
              mother_gar_logic, false, test_SiPM_id, false);
          //std::cout << sipm_pos_pv_x[i]/mm << ", " << sipm_pos_pv_y[j]/mm << std::endl;
          //std::cout << test_SiPM_->GetWidth() << ", " << test_SiPM_->GetHeight() << std::endl << std::endl;
          //std::cout << "Test SiPM ID: " << test_SiPM_id << std::endl;
          ++test_SiPM_id;
        }
      }
    }
    
    delete test_SiPM_;
  }

  


  G4ThreeVector NDGArChamber::GenerateVertex(const G4String& region) const
  {
    G4double gar_z = 5.*m;
    G4double gar_diam = 5.*m;
    G4ThreeVector theRegion;
    G4double rand_x = 1000*m;
    G4double rand_y = 1000*m;
    G4double rand_z = (G4UniformRand()-0.5)*gar_z;
    G4double rand_r = (G4UniformRand())*gar_diam/2.;
    G4double rand_phi = (G4UniformRand())*twopi;
    G4double grid_x[1000];
    G4double grid_y[1000];
    //G4cout << "##########################################################" << G4endl;
    //G4cout << "#              I AM GETTING CALLED                       #" << G4endl;
    //G4cout << "##########################################################" << G4endl;
    rand_x = rand_r*std::cos(rand_phi);
    rand_y = rand_r*std::sin(rand_phi);
    /*while(std::sqrt(std::pow(rand_x,2)+std::pow(rand_y,2))>=gar_diam/2.) {
      rand_x = (G4UniformRand()-0.5)*gar_diam;
      rand_y = (G4UniformRand()-0.5)*gar_diam;
    }*/
    //if (region == "GRID") {
    //}
    TString Tregion = (TString)region;
    char c = ' ';
    TString zStr(Tregion(Tregion.Last(c)+1,Tregion.Length()));
    if(region=="RANDOM") {
      theRegion = G4ThreeVector(rand_x, rand_y, rand_z);
    }
    else if (region == "POINT") {
      theRegion = G4ThreeVector(-4500,0,0);
    }
    else if(Tregion.Contains("ZAXIS")) {
        //std::cout << std::endl << Tregion << std::endl << std::endl;
        //std::cout << zStr << std::endl;
      theRegion= G4ThreeVector(0.,0,zStr.Atof());
    }
    else if(Tregion.Contains("RAXIS")) {
        //std::cout << std::endl << Tregion << std::endl << std::endl;
        //std::cout << zStr << std::endl;
      theRegion= G4ThreeVector(zStr.Atof()*std::cos(rand_phi),zStr.Atof()*std::sin(rand_phi),0.);
    }
    else theRegion = G4ThreeVector(0.,0.,0.);
    return theRegion;
  }


} // end namespace nexus