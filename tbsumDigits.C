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

void tbsumDigits(std::string digifile = "./RawData/trddigits-1ktf-2021-08-11-10h17.root"
                 //std::string hitfile = "./foo/o2sim_HitsTRD.root",
                 //std::string inputGeom = ""
                 //std::string paramfile = "./foo/o2sim_par.root"
                 )
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
  TH1F* ph = new TH1F("pulseheight", "Pulseheight", 30, -0.5, 29.5);

  LOG(INFO) << nev << " entries found";

  TNtuple *t = new TNtuple("nt","nt","d:r:c:m:h:l");
  int tbmax = 0;
  int tbhi = 0;
  int tblo = 0;

  int pulseheight = 0;

  int det = 0;
  int row = 0;
  int pad = 0;
  int channel = 0;
   map<tuple<int,int,int>,ArrayADC> dataMap;

  for (int iev = 0; iev < nev; iev++) {
    digitTree->GetEvent(iev);
    int tbsum[540][16][144] = {{{0}}};
      for (const auto& digit : *digitCont) {
          // loop over det, pad, row?
          auto adcs = digit.getADC();
          det = digit.getDetector()/2;
          row = digit.getPadRow();
          pad = digit.getPadCol();
          if (pad <0 && digit.getChannel() !=22){cout<<pad<<endl;}
          channel = digit.getChannel();
          dataMap.insert(make_pair(make_tuple(det,row,pad), adcs));
          if (channel == 0 || channel == 19 || channel ==20){continue;}
          else{
            for (int tb = 0; tb < o2::trd::constants::TIMEBINS; tb++) {
              ADC_t adc = adcs[tb];
              if (adc == (ADC_t)SimParam::instance()->getADCoutRange()) {
                //LOG(INFO) << "Out of range ADC " << adc;
                continue;
              }
              tbsum[det][row][pad] += adc;
            }
            htbsum->Fill(tbsum[det][row][pad]);
          }
        }// end digitcont
      for (int d=0;d<540;d++) {
        for (int r=0;r<16;r++) {
          for (int c=1;c<143;c++) {
            if (tbsum[d][r][c]>tbsum[d][r][c-1] && tbsum[d][r][c]>tbsum[d][r][c+1]) {
              if (tbsum[d][r][c-1] > tbsum[d][r][c+1]) {
                tbmax = tbsum[d][r][c];
                tbhi = tbsum[d][r][c-1];
                tblo = tbsum[d][r][c+1];
                auto adcMax = dataMap.find(make_tuple(d,r,c)) ;
                auto adcHi = dataMap.find(make_tuple(d,r,c-1));
                auto adcLo = dataMap.find(make_tuple(d,r,c+1));
                if (tblo != 0){
                  htbmax->Fill(tbsum[d][r][c]);
                  htbhi->Fill(tbsum[d][r][c-1]);
                  htblo->Fill(tbsum[d][r][c+1]);
                  t->Fill(d,r,c,tbmax,tbhi,tblo);
                  int phVal = 0;
                  for (int tb = 0 ; tb<30;tb++){
                    phVal = ((adcMax->second)[tb] + (adcHi->second)[tb] + (adcLo->second)[tb])/3;
                    ph->Fill(tb,phVal);
                  }
                }
              } 
              else {
                tbmax = tbsum[d][r][c];
                tbhi = tbsum[d][r][c+1];
                tblo = tbsum[d][r][c-1];
                auto adcMax = dataMap.find(make_tuple(d,r,c)) ;
                auto adcHi = dataMap.find(make_tuple(d,r,c+1));
                auto adcLo = dataMap.find(make_tuple(d,r,c-1));
                if (tblo != 0){
                  htbmax->Fill(tbsum[d][r][c]);
                  htbhi->Fill(tbsum[d][r][c+1]);
                  htblo->Fill(tbsum[d][r][c-1]);
                  
                  t->Fill(d,r,c,tbmax,tbhi,tblo);
                  int phVal = 0;
                  for (int tb = 0 ; tb<30;tb++){
                    phVal = ((adcMax->second)[tb] + (adcHi->second)[tb] + (adcLo->second)[tb])/3;
                    ph->Fill(tb,phVal);
                  }
                }
              }//end else
            }// end if (tbsum[d][r][c]>tbsum[d][r][c-1] && tbsum[d][r][c]>tbsum[d][r][c+1])
          }  // end for c
        }//end for r
      }// end for d 
    } //end event 

  t->Write();
  TCanvas* c3 = new TCanvas("c3", "TB Sum", 600, 600);
  gPad->SetLogy();
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

  TCanvas* c4 = new TCanvas("c4", "Pulse Height", 600, 600);
  ph->Draw();
  ph->SetLineColor(kBlack);
  TLegend* border2 = new TLegend(0.7, 0.7, 0.9, 0.9);
  border2->SetBorderSize(0); // no border
  border2->SetFillStyle(0);
  border2->SetFillColor(0); // Legend background should be white
  border2->SetTextFont(42);
  border2->SetTextSize(0.03); // Increase entry font size!
  border2->AddEntry(ph, "pulse height", "l");
  border2->Draw();
  c4->SaveAs("pulseheight.pdf");
}// end of macro
