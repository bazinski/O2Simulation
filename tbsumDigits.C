#if !defined(__CLING__) || defined(__ROOTCLING__)
#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TProfile.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TLatex.h>
#include "TStyle.h"
#include <iostream>
#include "FairLogger.h"
#include "TRDBase/Digit.h"
#include "TRDBase/TRDSimParam.h"
#include "TRDBase/TRDCommonParam.h"
#include "DataFormatsTRD/Constants.h"
#endif
using namespace o2::trd;
constexpr int kMINENTRIES = 100;

void tbsumDigits(std::string digifile = "/scratch/alice/trddigits.root",
                 std::string hitfile = "/scratch/alice/o2sim_HitsTRD.root",
                 std::string inputGeom = "",
                 std::string paramfile = "/scratch/alice/o2sim_par.root")
{
  TFile* fin = TFile::Open(digifile.data());
  TTree* digitTree = (TTree*)fin->Get("o2sim");
  std::vector<Digit>* digitCont = nullptr;
  //std::vector<Digit>* digits = nullptr;
  digitTree->SetBranchAddress("TRDDigit", &digitCont);
  //digitTree->SetBranchAddress("TRDDigit", &digits);
  int nev = digitTree->GetEntries();

  TH1F* htbsum = new TH1F("htbsum", "Tbsum", 100, 0, 3000);
  TH1F* htbhi = new TH1F("htbhi", "Tbsum", 100, 0, 3000);
  TH1F* htblo = new TH1F("htblo", "Tbsum", 100, 0, 3000);
  TH1F* htbmax = new TH1F("htbm ax", "Tbsum", 100, 0, 3000);

  LOG(INFO) << nev << " entries found";
  for (int iev = 0; iev < nev; ++iev) {
    digitTree->GetEvent(iev);
      int tbsum[540][16][144];
      for (const auto& digit : *digitCont) {
          // loop over det, pad, row?
          auto adcs = digit.getADC();
          int det = digit.getDetector();
          int row = digit.getRow();
          int pad = digit.getPad();

          for (int tb = 0; tb < o2::trd::constants::TIMEBINS; ++tb) {
            ADC_t adc = adcs[tb];
            if (adc == (ADC_t)SimParam::instance()->getADCoutRange()) {
              // LOG(INFO) << "Out of range ADC " << adc;
              continue;
            }
            //int rowb[det][pad][tb];
            tbsum[det][row][pad] += adc;
          }
          htbsum->Fill(tbsum[det][row][pad]);
        }// end digitcont

        for (int d=0;d<540;d++) {
          for (int r=0;r<16;r++) {
            for (int c=1; c<143;c++) {

              if (tbsum[d][r][c]>tbsum[d][r][c-1] && tbsum[d][r][c]>tbsum[d][r][c+1]) {
                if (tbsum[d][r][c-1] > tbsum[d][r][c+1]) {
                  // T[c] > T[c-1] > T[c+1]
                  htbmax->Fill(tbsum[d][r][c]);
                  htbhi->Fill(tbsum[d][r][c-1]);
                  htblo->Fill(tbsum[d][r][c+1]);

                } else {
                  // T[c] > T[c+1] > T[c-1]
                  htbmax->Fill(tbsum[d][r][c]);
                  htbhi->Fill(tbsum[d][r][c+1]);
                  htblo->Fill(tbsum[d][r][c-1]);
                }//end else
              }// end if (tbsum[d][r][c]>tbsum[d][r][c-1] && tbsum[d][r][c]>tbsum[d][r][c+1])
            }  // end for c
          }//end for r
        }// end for d
    } //end event

    TCanvas* c3 = new TCanvas("c3", "TB Sum", 600, 600);
    htbmax->SetLineColor(kRed);
    htblo->SetLineColor(kBlue);
    htbhi->SetLineColor(kGreen);
    htbsum->SetLineColor(kBlack);

    htbsum->Draw();
    htbmax->Draw("SAME");
    htbhi->Draw("SAME");
    htblo->Draw("SAME");

    TLegend* border = new TLegend(0.7, 0.7, 0.9, 0.9);
    border->SetBorderSize(0); // no border
    border->SetFillStyle(0);
    border->SetFillColor(0); // Legend background should be white
    border->SetTextFont(42);
    border->SetTextSize(0.03); // Increase entry font size!
    border->AddEntry(htbsum, "htbsum", "l");
    border->AddEntry(htbmax, "htbmax", "l");
    border->AddEntry(htbhi, "htbhi", "l");
    border->AddEntry(htblo, "htblo", "l");
    border->Draw();

    c3->SaveAs("tbsum.pdf");
}// end of macro
