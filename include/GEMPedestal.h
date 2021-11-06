#ifndef GEM_PEDESTAL_H
#define GEM_PEDESTAL_H

#include "MPDDataStruct.h"
#include "GEMStruct.h"
#include "EvioFileReader.h"
#include "EventParser.h"

#include <unordered_map>
#include <vector>
#include <string>

#include <TH1F.h>

class GEMPedestal
{
public:
    GEMPedestal();
    ~GEMPedestal();

    void CalculatePedestal();
    void CalculateEventRawPedestal(const std::unordered_map<APVAddress, std::vector<int>> &);
    void GenerateAPVPedestal_using_histo();
    void GenerateAPVPedestal_using_vec();
    void SetDataFile(const char* path);
    void SetNumberOfEvents(int num);
    void Clear();

    std::vector<StripRawADC> DecodeAPV(std::vector<int> const &);
    std::vector<float> GetTimeSampleCommonMode(const std::vector<StripRawADC> &);

    // helpers
    APVAddress ParseAPVAddressFromString(const std::string &);
    bool APVStripIsNew(const APVStripAddress &);
    void RawAPVUnit_histo(const std::unordered_map<APVAddress, std::vector<int>>::value_type &);
    void RawAPVUnit_vec(const std::unordered_map<APVAddress, std::vector<int>>::value_type &);
    void RawPedestalThread(const std::unordered_map<APVAddress, std::vector<int>> &, int, int);
    void GetEvent(EvioFileReader *, EventParser *, uint32_t &nEvents);
    float GetMean(const std::vector<float> &);
    float GetRMS(const std::vector<float> &);
    std::vector<int> cleanup_srs_apv_header_words(const std::vector<int>&);
 
    // getters
    int GetNumberOfEvents() const;
    const std::unordered_map<APVAddress, std::vector<float>> & GetAPVNoise() const;
    const std::unordered_map<APVAddress, std::vector<float>> & GetAPVOffset() const;
    const std::unordered_map<APVAddress, TH1F*> & GetAPVNoiseHisto() const ;
    const std::unordered_map<APVAddress, TH1F*> & GetAPVOffsetHisto() const;

    // read/write to disk
    void SavePedestalHisto(const char* path);
    void SavePedestalText();
    void LoadPedestalHisto(const char* path);
    void LoadPedestalText();

private:
    std::unordered_map<APVAddress, std::vector<float>> mAPVNoise;
    std::unordered_map<APVAddress, std::vector<float>> mAPVOffset;
    std::unordered_map<APVAddress, TH1F*> mAPVNoiseHisto;
    std::unordered_map<APVAddress, TH1F*> mAPVOffsetHisto;

    // for each strip, they all have a TH1F
    // this might consumes huge memory, seems no way to avoid it
    std::unordered_map<APVStripAddress, TH1F*> mAPVStripNoise;
    std::unordered_map<APVStripAddress, TH1F*> mAPVStripOffset;
    // std::vector is about 3 times faster than TH1F, 
    // for computing intensive jobs, use vector instead of TH1F
    std::unordered_map<APVStripAddress, std::vector<float>> mAPVStripNoiseVec;
    std::unordered_map<APVStripAddress, std::vector<float>> mAPVStripOffsetVec;

    // total number of events used for calculating pedestal
    uint32_t fNumberEvents = 5000;
    std::string data_file_path = "";

    // file reader
    EvioFileReader *file_reader = nullptr;
};

#endif
