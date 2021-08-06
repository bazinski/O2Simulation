#if !defined(__CLING__) || defined(__ROOTCLING__)
#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TProfile.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TLatex.h>
#include "TNtuple.h"
#include "TStyle.h"
#include <iostream>
#include <tuple>
#include "FairLogger.h"
#include "TRDBase/Digit.h"
#include "TRDBase/TRDSimParam.h"
#include "TRDBase/TRDCommonParam.h"
#include "DataFormatsTRD/Constants.h"
#endif
using namespace o2::trd;
using namespace std;

constexpr int kMINENTRIES = 100;

void tbsumDigits(std::string digifile = "./foo/trddigits.root",
                 std::string hitfile = "./foo/o2sim_HitsTRD.root",
                 std::string inputGeom = "",
                 std::string paramfile = "./foo/o2sim_par.root")
{
  TFile* fin = TFile::Open(digifile.data());
  TTree* digitTree = (TTree*)fin->Get("o2sim");

  auto f = TFile::Open("ntuples.root","RECREATE"); //root file where tntuples are being stored

  std::vector<Digit>* digitCont = nullptr;
  digitTree->SetBranchAddress("TRDDigit", &digitCont);
  int nev = digitTree->GetEntries();

  TH1F* htbsum = new TH1F("htbsum", "Tbsum", 100, 0, 3000);
  TH1F* htbhi = new TH1F("htbhi", "Tbsum", 100, 0, 3000);
  TH1F* htblo = new TH1F("htblo", "Tbsum", 100, 0, 3000);
  TH1F* htbmax = new TH1F("htbmax", "Tbsum", 100, 0, 3000);
  TH1F* ph = new TH1F("pulseheight", "Pulseheight", 100, 0, 3000);

  LOG(INFO) << nev << " entries found";

  TNtuple *t = new TNtuple("nt","nt","d:r:c:m:h:l");
  int tbmax = 0;
  int tbhi = 0;
  int tblo = 0;

  int pulseheight = 0;

  int det = 0;
  int row = 0;
  int pad = 0;

  //int digits[540][16][144][30] = {{{{0}}}};

  for (int iev = 0; iev < nev; iev++) {
    digitTree->GetEvent(iev);
    int tbsum[540][16][144] = {{{0}}};
      for (const auto& digit : *digitCont) {
          // loop over det, pad, row?
          auto adcs = digit.getADC();
          det = digit.getDetector();
          row = digit.getPadRow();
          pad = digit.getPadCol();
          //cout<<det<<" "<<row<<" "<<pad<<" "<<endl;
          for (int tb = 0; tb < o2::trd::constants::TIMEBINS; tb++) {
            ADC_t adc = adcs[tb];
            //data[det][row][pad][tb] = adc;
            if (adc == (ADC_t)SimParam::instance()->getADCoutRange()) {
               //LOG(INFO) << "Out of range ADC " << adc;
              continue;
            }
            //cout<<adc<<endl;
            tbsum[det][row][pad] += adc; 
          }
          htbsum->Fill(tbsum[det][row][pad]);
          //cout<<det<<" "<<row<<" "<<pad<<" "<<tbsum[det][row][pad]<<endl;
        }// end digitcont
      for (int d=0;d<540;d++) {
        for (int r=0;r<16;r++) {
          for (int c=0;c<144;c++) {
            //cout<<"hello"<<endl;
            if (tbsum[d][r][c]>tbsum[d][r][c-1] && tbsum[d][r][c]>tbsum[d][r][c+1]) {
              if (tbsum[d][r][c-1] > tbsum[d][r][c+1]) {
                tbmax = tbsum[d][r][c];
                tbhi = tbsum[d][r][c-1];
                tblo = tbsum[d][r][c+1];
                if (tblo != 0){
                  htbmax->Fill(tbsum[d][r][c]);
                  htbhi->Fill(tbsum[d][r][c-1]);
                  htblo->Fill(tbsum[d][r][c+1]);
                  t->Fill(d,r,c,tbmax,tbhi,tblo);
                }
                /*
                for (int tb = 0; tb<30 ; tb++){
                  pulseheight = digits[d][r][c][tb] + digits[d][r][c + 1][tb] +digits[d][r][c - 1][tb];
                  if (pulseheight!=0){
                    ph->Fill(pulseheight);
                  }
                } */
              } 
              else {
                tbmax = tbsum[d][r][c];
                tbhi = tbsum[d][r][c+1];
                tblo = tbsum[d][r][c-1];
                if (tblo != 0){
                  htbmax->Fill(tbsum[d][r][c]);
                  htbhi->Fill(tbsum[d][r][c+1]);
                  htblo->Fill(tbsum[d][r][c-1]);
                  
                  t->Fill(d,r,c,tbmax,tbhi,tblo);
                }
                /*
                for (int tb = 0; tb<30 ; tb++){
                  pulseheight = digits[d][r][c][tb] + digits[d][r][c + 1][tb] + digits[d][r][c - 1][tb];
                  if (pulseheight!=0){
                    ph->Fill(pulseheight); 
                  }
                } */
              }//end else
            }// end if (tbsum[d][r][c]>tbsum[d][r][c-1] && tbsum[d][r][c]>tbsum[d][r][c+1])
          }  // end for c
        }//end for r
      }// end for d 
    } //end event 

    t->Write();
    TCanvas* c3 = new TCanvas("c3", "TB Sum", 600, 600);
    //c3->Divide(1,2);

    htbmax->SetLineColor(kRed);
    htblo->SetLineColor(kBlue);
    htbhi->SetLineColor(kGreen); 
    htbsum->SetLineColor(kBlack);

    //c3->cd(1);
    htbsum->Draw();
    htbmax->Draw("SAME");
    htbhi->Draw("SAME");
    htblo->Draw("SAME"); 
    //c3->cd(2);
    //ph->Draw();

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
