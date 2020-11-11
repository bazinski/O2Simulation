
// #include <>

#include "SpacePointConverter.C"

std::ostream& operator<<(std::ostream& os, const o2::trd::Digit& d)
{
  int tbsum=0;
  auto adcs = d.getADC();

  for (int i=0;i<30;i++) {
    tbsum += adcs[i];
  }

  os << d.getDetector() << ":" << d.getRow() << ":" << d.getPad()
     << "  tbsum=" << tbsum;

  return os;
}

std::ostream& operator<<(std::ostream& os, const o2::trd::HitType& h)
{
  os << h.GetDetectorID() << ":"
     << h.getLocalC() << "/" << h.getLocalT() << "/" << h.getLocalR()
     << "  q=" << h.GetCharge();

  return os;
}


void draw()
{

  // ----------------------------------------------------------------------
  // set up data structures for reading

  TFile* fhits = new TFile("o2sim_HitsTRD.root");

  // get `o2sim` tree from file
  TTree* trhits = NULL;
  fhits->GetObject("o2sim", trhits);

  // instantiate the reader for the tree
  TTreeReader reader_hit(trhits);

  // set up the branches we want to read
  TTreeReaderArray<o2::trd::HitType> trdhits(reader_hit, "TRDHit");

  // open main file
  TFile* fdig = new TFile("trddigits.root");

  // get `o2sim` tree from file
  TTree* trdig = NULL;
  fdig->GetObject("o2sim", trdig);


  // instantiate the reader for the tree
  TTreeReader reader_dig(trdig);

  // set up the branches we want to read
  TTreeReaderArray<o2::trd::Digit> trddigits(reader_dig, "TRDDigit");

  SpacePointConverter conv;


  // ----------------------------------------------------------------------

  TH2F* padrow = new TH2F("padrow", "padrow",144,-0.5,143.5,30,-0.5,29.5);

  TCanvas* cnv = new TCanvas("cnv_padrow", "cnv_padrow", 800,600);

  // ----------------------------------------------------------------------
  // event loop

  // loop over the entries in trddigits.root, which are time frames
  while ( reader_dig.Next() ) {


    // // loop over the trigger records - skip the first one (known bug)
    // for(int iev=1; iev<trdtrgrec.GetSize(); iev++) {
    //   auto evrec = trdtrgrec[iev];
    //
    //   cout << evrec << endl;

    // load the hits for the next event
    if ( ! reader_hit.Next() ) {
      cout << "no hits found for event" << endl;
      break;
    }

    // cout << "## Event " << tfno << ":" << evno << ":  "
    //      << trdhits.GetSize() << " hits   "
    //      << evrec.getNumberOfObjects() << " digits" << endl;

    for(int idig=0; idig<trddigits.GetSize(); idig++) {

      auto dig = trddigits[idig];
      cout << dig << endl;

      auto adc = dig.getADC();

      for (int i=0;i<30;i++) {
        padrow->Fill(dig.getPad(), i, adc[i]);
      }
    }

    padrow->Draw("colz");

    TMarker m;
    m.SetMarkerStyle(20);

    for(int ihit=0; ihit<trdhits.GetSize(); ihit++) {
      auto hit = trdhits[ihit];

      auto rct = conv.Hit2RowColTime(hit);

      cout << hit << "   " << rct[0] << ":" << rct[1] << ":" << rct[2] << endl;

      m.DrawMarker(rct[1], rct[2]);
    }

  }

 // hdet->Draw("lego");
 // padrow->Draw("colz");

}
