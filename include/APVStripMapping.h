#ifndef APV_STRIP_MAPPING_H
#define APV_STRIP_MAPPING_H

/*
 *  This file needs to be reorganized in a better way
 *  Improvement will be continuously made
 */

////////////////////////////////////////////////////////////////////////////////
// apv internal channel mapped to detector position
// this map includes two stages:
// stage1: apv internal channel number  -> apv hybrid card strip number
// stage2: apv hybrid card strip number -> sorted strip number on GEM detector
//
// using this mapping, one get directly map apv internal channel to 
// sorted strip number on GEM chamber
//
// this is original and believed to be the right one, Jan 09 2017
//
// check the APV hybrid board for details

#include <unordered_map>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include "GEMStruct.h"

namespace apv_strip_mapping {

////////////////////////////////////////////////////////////////////////////////
// a strip mapping from danning

const int mapped_strip[128] = {
     1,  33, 65,  97,  9,  41, 73, 105, 17,  49, 
    81, 113, 25,  57, 89, 121,  3,  35, 67,  99, 
    11,  43, 75, 107, 19,  51, 83, 115, 27,  59, 
    91, 123,  5,  37, 69, 101, 13,  45, 77, 109, 
    21,  53, 85, 117, 29,  61, 93, 125,  7,  39, 
    71, 103, 15,  47, 79, 111, 23,  55, 87, 119, 
    31,  63, 95, 127,  0,  32, 64,  96,  8,  40, 
    72, 104, 16,  48, 80, 112, 24,  56, 88, 120, 
     2,  34, 66,  98, 10,  42, 74, 106, 18,  50, 
    82, 114, 26,  58, 90, 122,  4,  36, 68, 100, 
    12,  44, 76, 108, 20,  52, 84, 116, 28,  60, 
    92, 124,  6,  38, 70, 102, 14,  46, 78, 110, 
    22,  54, 86, 118, 30,  62, 94, 126
};

////////////////////////////////////////////////////////////////////////////////
// a struct stores all apv information 
// duplicated structure, for compatibility to all previous data generated using
// siyu's code
//
// to be removed

struct APVInfo
{
    int crate_id, layer_id, mpd_id;

    // detector id is an unique number assigned to each GEM during assembly in UVa
    // it is labeled on each detector
    int detector_id;

    // x/y plane (0=x; 1=y; to be changed to string)
    int dimension;
    int adc_ch, i2c_ch, apv_pos, invert;
    std::string discriptor;
    int backplane_id, gem_pos;

    APVInfo(){};
    APVInfo(const std::string &str)
    {
        if(str.find("APV") == std::string::npos)
            return;

        std::istringstream entry(str);
        std::string token;
        std::vector<std::string> tmp;
        while(std::getline(entry, token, ','))
            tmp.push_back(token);

        crate_id     = std::stoi(tmp[1]);    layer_id    = std::stoi(tmp[2]); 
        mpd_id       = std::stoi(tmp[3]);    detector_id = std::stoi(tmp[4]);
        dimension    = std::stoi(tmp[5]);    adc_ch      = std::stoi(tmp[6]); 
        i2c_ch       = std::stoi(tmp[7]);    apv_pos     = std::stoi(tmp[8]);   
        invert       = std::stoi(tmp[9]);    discriptor  = tmp[10];          
        backplane_id = std::stoi(tmp[11]);   gem_pos     = std::stoi(tmp[12]);
    }
};

std::ostream &operator<<(std::ostream& out, const APVInfo &);

////////////////////////////////////////////////////////////////////////////////
// a gem mapping class
// this class is redundant, to be removed in the future

class Mapping 
{
public:
    static Mapping* Instance() {
        if(!instance)
            instance = new Mapping();
        return instance;
    }

    Mapping(Mapping const &) = delete;
    void operator=(Mapping const &) = delete;

    ~Mapping(){}

    void LoadMap(const char* path);
    void Print();

    // members
    void ExtractMPDAddress();

    // getters
    int GetPlaneID(const GEMChannelAddress &addr);
    int GetProdID(const GEMChannelAddress &addr);
    int GetModuleID(const GEMChannelAddress &addr);
    int GetAxis(const GEMChannelAddress &addr);
    int GetStrip(const GEMChannelAddress &addr);

    int GetTotalMPDs();
    const std::vector<MPDAddress> & GetMPDAddressVec() const;

private:
    static Mapping* instance;
    Mapping();

    std::unordered_map<APVAddress, APVInfo> apvs;
    std::vector<MPDAddress> vMPDAddr;

    bool map_loadded = false;
};

};
#endif
