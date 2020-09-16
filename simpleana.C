
#include "TRDSimulation/Detector.h" // For TRD Hit

void simpleana(TString filename="o2sim.root")
{
  TFile rf(filename, "OPEN");
  auto tr = (TTree*)rf.Get("o2sim");

  auto br = tr->GetBranch("TRDHit");
  if (!br) {
    cout << "No TTree" << endl;
    return;
  }

  auto entries = br->GetEntries();
  std::vector<o2::trd::HitType>* hitvector = nullptr;
  br->SetAddress(&hitvector);

  for (int i = 0; i < entries; ++i) {
    br->GetEntry(i);
    for (auto& hit : *hitvector) {
      cout << hit.GetX() << endl;
    }
  }
}
