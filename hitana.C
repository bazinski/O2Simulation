

#if !defined(__CLING__) || defined(__ROOTCLING__)
#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TCanvas.h>

#include "FairLogger.h"
#include "TRDSimulation/Detector.h"
#include "TRDBase/TRDGeometry.h"
#include "TRDBase/Calibrations.h"
#include "DetectorsCommonDataFormats/NameConf.h"
#include "DetectorsCommonDataFormats/DetID.h"
#endif

using namespace o2::detectors;


// #include "TRDSimulation/Detector.h" // For TRD Hit
// #include <TFile.h>
// #include <TTree.h>
//
// #include <iostream>
// using namespace std;

// gSystem->Load("libBase")
// gSystem->Load("libO2TRDBase")
// gSystem->Load("libO2TRDSimulation")

void hitana(TString filename="o2sim_HitsTRD.root")
{

  TFile* fin = TFile::Open(hitfile.data());


  TTree* tree = (TTree*)fin->Get("o2sim");
  std::vector<o2::trd::HitType>* hits = nullptr;
  tree->SetBranchAddress("TRDHit", &hits);
  int nev = hitTree->GetEntries();

  // TH1F* hlocC = new TH1F("hlocC", ";locC (cm);Counts", 100, -60, 60);
  // TH1F* hlocR = new TH1F("hlocR", ";locR (cm);Counts", 100, -80, 80);
  // TH1F* hlocT = new TH1F("hlocT", ";locT (cm);Counts", 100, -3.5, 0.5);
  // TH1F* hnEl = new TH1F("hnEl", ";Number of Electrons;Counts", 100, 0, 5000);
  // TH1F* hnElPhoton = new TH1F("hnElPhoton", ";Number of Electrons;Counts", 100, 0, 1000);
  //
  // TH2F* h2locClocT = new TH2F("h2locClocT", ";locC (cm);locT(cm)", 100, -60, 60, 100, -3.5, 0.5);
  // TH2F* h2locClocTnEl = new TH2F("h2locClocTnEl", "nEl;locC (cm);locT(cm)", 100, -60, 60, 100, -3.5, 0.5);
  //

  LOG(INFO) << nev << " entries found";

  for (int iev = 0; iev < nev; ++iev) {
    hitTree->GetEvent(iev);
    for (const auto& hit : *hits) {


      cout << hit.GetX() << endl;

    }

  }

  //     TFile rf(filename, "OPEN");
  //     auto tr = (TTree*)rf.Get("o2sim");
  //
  // auto br = tr->GetBranch("TRDHit");
  // if (!br) {
  //   cout << "No TTree" << endl;
  //   return;
  // }
  //
  // auto entries = br->GetEntries();
  // std::vector<o2::trd::HitType>* hitvector = nullptr;
  // br->SetAddress(&hitvector);

//   for (int i = 0; i < entries; ++i) {
//     br->GetEntry(i);
//     for (auto& hit : *hitvector) {
//       cout << hit.GetX() << endl;
//     }
//   }
// }
