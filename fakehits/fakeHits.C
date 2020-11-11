
//
#include "TInterpreter.h"
#include "TRandom3.h"
#include "TFile.h"
#include "TTree.h"

#include <vector>
#include <iostream>

// /***************************************************************/
// class MyStruct : public TObject
// {
// public:
//  Int_t a;
//  Float_t b;
//  MyStruct() {a=0; b=0.0;}
//  ClassDef(MyStruct,1)
// };
// /***************************************************************/
void fakeHits(void)
{

  gInterpreter->GenerateDictionary("std::vector<o2::trd::HitType>", "vector");
  std::vector<o2::trd::HitType> hits;

  // ----------------------------------------------------------------------
  // set up data structures for reading

  // open main file
  TFile* f = new TFile("o2sim_HitsTRD.root","recreate");

  TTree* o2sim = new TTree("o2sim", "o2sim");
  o2sim->Branch("TRDHit", &hits);

  hits.resize(40);

  for (int i=0; i<hits.size(); i++) {
    hits[i] = o2::trd::HitType(1.,1.,1., 0.0,0.0,2.0, 0.,500,0,281,true);
  }

  double dx = 0.785*8;

  for(int i=0; i<20; i++) {
    hits[i].setLocalC( i*dx - 50 );
    hits[i].setLocalT( -0.45 );
  }

  for(int i=20; i<40; i++) {
    hits[i].setLocalC( (i-20)*dx - 50 );
    hits[i].setLocalT( -2.45 );
  }


  // hits[2] = o2::trd::HitType(1.,1.,1., -10., 0.0, -2.0,  0., 500, 0, 281, true);


  o2sim->Fill();

  o2sim->Write();
  f->Close();
}
