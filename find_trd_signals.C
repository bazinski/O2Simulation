#if !defined(__CLING__) || defined(__ROOTCLING__)
#include <TFile.h>
#include <TTree.h>
#include <TProfile.h>
#include <TCanvas.h>
#include "DataFormatsTRD/Digit.h"
#include "DataFormatsTRD/TriggerRecord.h"

#include <iostream>
#include <numeric>
#include <vector>
#include <array>
#endif

// Helpers for signal handling
static constexpr int KEY_MIN = 0;
static constexpr int KEY_MAX = 2211727;
int calculateKey(const int det, const int row, const int col)
{
    int key = ((det << 12) | (row << 8) | col);
    assert(!(key < KEY_MIN || key > KEY_MAX));
    return key;
}
int getDetectorFromKey(const int key) { return (key >> 12) & 0xFFF; }
int getRowFromKey(const int key) { return (key >> 8) & 0xF; }
int getColFromKey(const int key) { return key & 0xFF; }

std::unordered_map<int, o2::trd::Digit> getDigitsMap(const std::vector<o2::trd::Digit>& digits)
{
    std::unordered_map<int, o2::trd::Digit> digitsMap;
    for (const auto& digit : digits) {
        int det = digit.getDetector();
        int row = digit.getRow();
        int pad = digit.getPad();
        int key = calculateKey(det, row, pad);
        digitsMap[key] = digit;
    }
    return digitsMap;
}

void fillProfiles(int det, std::map<int, TProfile*>& profADCChamber, int tb, float cluster)
{
    if (profADCChamber.count(det)) {
        profADCChamber[det]->Fill(tb, cluster);
    } else {
        int d = det;
        profADCChamber[det] = new TProfile(Form("profADCChamber%d", d), Form("ADC distribution for chamber %d for each time bin;Time bin;ADC", d), 31, -0.5, 30.5);
        profADCChamber[det]->Fill(tb, cluster);
    }
}

void find_trd_signals()
{
    auto fin = TFile::Open("trddigits.root");
    TTree* digitTree = (TTree*)fin->Get("o2sim");
    std::vector<o2::trd::Digit>* digits = nullptr;
    std::vector<o2::trd::TriggerRecord>* trigrecords = nullptr;
    digitTree->SetBranchAddress("TRDDigit", &digits);
    digitTree->SetBranchAddress("TriggerRecord", &trigrecords);
    const int nev = digitTree->GetEntries();
    printf("I have %d entries\n", nev);

    TFile* fout = TFile::Open("results.root", "RECREATE");

    TProfile* profADC = new TProfile("profADC", "ADC distribution for all chambers for each time bin;Time bin;ADC", 31, -0.5, 30.5);
    std::map<int, TProfile*> profADCChamber;

    TTree* tree = new TTree("tree", "tree of digit info");
    int adcsum, adcsumnp, adcsumnn;
    int adcsumClu;
    tree->Branch("adcsum", &adcsum);
    tree->Branch("adcsumnp", &adcsumnp);
    tree->Branch("adcsumnn", &adcsumnn);
    tree->Branch("adcsumClu", &adcsumClu);

    for (int entry = 0; entry < nev; ++entry) {
        digitTree->GetEntry(entry);
        printf("The digit container has %lu entries\n", digits->size());
        printf("The trigger record container has %lu entries\n", trigrecords->size());

        int totalReadObjs = 0;
        const int nTrigRec = trigrecords->size();
        // for (const auto& t : *trigrecords) {
        for (int it = 0; it < nTrigRec; ++it) {
            const auto& t = (*trigrecords).at(it);

            // get a vector with the digits from this readout event
            auto first = digits->begin() + t.getFirstDigit();
            auto last = first + t.getNumberOfDigits();
            const std::vector<o2::trd::Digit> eventDigits(first, last);

            assert(eventDigits.size() == t.getNumberOfDigits());
            if (eventDigits.size() != t.getNumberOfDigits()) {
                printf("I have a subset of %lu digits when fetching %d objects\n",
                       eventDigits.size(), t.getNumberOfDigits());
            }
            totalReadObjs += t.getNumberOfDigits();
            // printf("The trigger record %d has %d entries\n", it, t.getNumberOfObjects());
            // printf("The event has %d digits\n", eventDigits.size());

            if (eventDigits.size() == 0) {
                continue;
            }

            // get a map of digits for easier det/col/row manipulation
            auto digitsMap = getDigitsMap(eventDigits);

            for (int det = 0; det < 540; ++det) {
                for (int row = 0; row < 16; ++row) {
                    for (int col = 0; col < 144; ++col) {
                        if (col == 0 || col == 143) { // won't work with boundaries yet
                            continue;
                        }

                        // skip if element doesn't exist
                        if (digitsMap.count(calculateKey(det, row, col)) == 0) {
                            continue;
                        }

                        auto digit = digitsMap[calculateKey(det, row, col)];
                        auto adcs = digit.getADC();
                        auto adcsnp = digitsMap[calculateKey(det, row, col + 1)].getADC();
                        auto adcsnn = digitsMap[calculateKey(det, row, col - 1)].getADC();

                        adcsum = digit.getADCsum();
                        adcsumnp = digitsMap[calculateKey(det, row, col + 1)].getADCsum();
                        adcsumnn = digitsMap[calculateKey(det, row, col - 1)].getADCsum();
                        adcsumClu = adcsum + adcsumnp + adcsumnn;

                        if (adcsum < adcsumnp || adcsum < adcsumnn) {
                            continue;
                        }

                        if (adcsum < 310 || adcsumClu < 910) {
                            continue;
                        }

                        if (adcsumnp < 310 && adcsumnn < 310) {
                            continue;
                        }

                        tree->Fill();

                        for (int tb = 0; tb < 30; ++tb) {
                            auto cluster = adcs[tb] + adcsnn[tb] + adcsnp[tb];
                            fillProfiles(det, profADCChamber, tb, cluster);
                            profADC->Fill(tb, cluster);
                        } // loop over timebins

                    } // cols
                }     // rows
            }         // dets
        }             // loop over trigger records

        printf("I read %d objects from the trigger records in %d events\n", totalReadObjs, trigrecords->size());

        for (const auto& [det, prof] : profADCChamber) {
            prof->Write();
        }

        tree->Print();
        fout->Write();
        fout->Close();

    } // loop over entries in tree
} // end of program
