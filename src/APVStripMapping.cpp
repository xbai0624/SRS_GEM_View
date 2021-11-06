#include "APVStripMapping.h"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace apv_strip_mapping {

Mapping* Mapping::instance = nullptr;

////////////////////////////////////////////////////////////////////////////////
// a helper
// remove leading and trailing white space

static std::string trim(const std::string &str)
{
    if(str.size() <= 0) 
        return "";

    const auto begin = str.find_first_not_of(' ');
    const auto end = str.find_last_not_of(' ');

    const auto range = end - begin + 1;

    return str.substr(begin, range);
}

////////////////////////////////////////////////////////////////////////////////
// a helper

std::ostream& operator<<(std::ostream& out, const APVInfo& info)
{
    out<<std::setfill(' ')<<std::setw(8)<<"APV"
        <<std::setfill(' ')<<std::setw(8)<<info.crate_id
        <<std::setfill(' ')<<std::setw(8)<<info.layer_id
        <<std::setfill(' ')<<std::setw(8)<<info.mpd_id
        <<std::setfill(' ')<<std::setw(8)<<info.detector_id
        <<std::setfill(' ')<<std::setw(8)<<info.dimension
        <<std::setfill(' ')<<std::setw(8)<<info.adc_ch
        <<std::setfill(' ')<<std::setw(8)<<info.i2c_ch
        <<std::setfill(' ')<<std::setw(8)<<info.apv_pos
        <<std::setfill(' ')<<std::setw(8)<<info.invert
        <<std::setfill(' ')<<std::setw(8)<<info.discriptor
        <<std::setfill(' ')<<std::setw(8)<<info.backplane_id
        <<std::setfill(' ')<<std::setw(8)<<info.gem_pos
        <<std::endl;

    return out;
}


////////////////////////////////////////////////////////////////////////////////
// mapping ctor

Mapping::Mapping()
{
    LoadMap("database/gem_map.txt");
}

////////////////////////////////////////////////////////////////////////////////
// load map

void Mapping::LoadMap(const char* path)
{
    if(map_loadded) return;

    std::fstream f(path, std::fstream::in);
    if(!f.is_open()) {
        std::cout<<"Error: mapping cannot open file: "<<path<<std::endl;
        exit(0);
    }

    std::string line;
    while(std::getline(f, line))
    {
        if(line.size() <= 0) continue;

        std::string tmp = trim(line);

        // skip comment
        if(tmp[0] == '#') continue;

        if(tmp.find("APV") == std::string::npos)
            continue;

        APVInfo apv_info(tmp);

        APVAddress addr(apv_info.crate_id, apv_info.mpd_id, apv_info.adc_ch);
        if(apvs.find(addr) != apvs.end())
        {
            std::cout<<"Mapping:: Error reading mapping file."
                <<" Found duplicated apv entries in map file."
                <<std::endl;
            std::cout<<apv_info;
            exit(0);
        }
        apvs[addr] = apv_info;
    }

    f.close();

    map_loadded = true;

    // extract all MPD IDs
    ExtractMPDAddress();
}

////////////////////////////////////////////////////////////////////////////////
// print map

void Mapping::Print()
{
    for(auto &i: apvs)
        std::cout<<i.second;
}

////////////////////////////////////////////////////////////////////////////////
// in order to generate the same root tree
// reimplemented from siyu's code

int Mapping::GetPlaneID(const GEMChannelAddress &addr)
{
    if(!map_loadded) {
        std::cout<<"Error in mapping:: mapping file not yet loadded."<<std::endl;
        exit(0);
    }

    // seems layer id
    // gem location in layer
    APVAddress ad(addr.crate, addr.mpd, addr.adc);
    if(apvs.find(ad) == apvs.end()) {
        std::cout<<"Error: apv not found."<<std::endl;
        exit(0);
    }

    int plane_id = apvs[ad].layer_id;
    return plane_id;
}

////////////////////////////////////////////////////////////////////////////////
// reimplemented from siyu's code

int Mapping::GetProdID(const GEMChannelAddress &addr)
{
    if(!map_loadded) {
        std::cout<<"Error in mapping:: mapping file not yet loadded."<<std::endl;
        exit(0);
    }

    // gem production id given by UVa during assembly
    APVAddress ad(addr.crate, addr.mpd, addr.adc);
    if(apvs.find(ad) == apvs.end()) {
        std::cout<<"Error: apv not found."<<std::endl;
        exit(0);
    }

    int prod_id = apvs[ad].detector_id;
    return prod_id;
}

////////////////////////////////////////////////////////////////////////////////
// reimplemented from siyu's code

int Mapping::GetModuleID(const GEMChannelAddress &addr)
{
    if(!map_loadded) {
        std::cout<<"Error in mapping:: mapping file not yet loadded."<<std::endl;
        exit(0);
    }

    // gem location in layer
    APVAddress ad(addr.crate, addr.mpd, addr.adc);
    if(apvs.find(ad) == apvs.end()) {
        std::cout<<"Error: apv not found."<<std::endl;
        exit(0);
    }

    int module_id = apvs[ad].gem_pos;

    return module_id;
}

////////////////////////////////////////////////////////////////////////////////
// reimplemented from siyu's code

int Mapping::GetAxis(const GEMChannelAddress &addr)
{
    if(!map_loadded) {
        std::cout<<"Error in mapping:: mapping file not yet loadded."<<std::endl;
        exit(0);
    }

    // x or y plane
    APVAddress ad(addr.crate, addr.mpd, addr.adc);
    if(apvs.find(ad) == apvs.end()) {
        std::cout<<"Error: apv not found."<<std::endl;
        exit(0);
    }

    int a = apvs[ad].dimension;
    return a;
}

////////////////////////////////////////////////////////////////////////////////
// reimplemented from siyu's code

int Mapping::GetStrip(const GEMChannelAddress &addr)
{
    if(!map_loadded) {
        std::cout<<"Error in mapping:: mapping file not yet loadded."<<std::endl;
        exit(0);
    }

    // apv address
    APVAddress ad(addr.crate, addr.mpd, addr.adc);
    if(apvs.find(ad) == apvs.end()) {
        std::cout<<"Error: apv not found."<<std::endl;
        exit(0);
    }

    // apv internal channel number
    int strip = addr.strip; 
    // convert to detector sequence
    strip = mapped_strip[strip];

    if(apvs[ad].invert)
        strip = 127 - strip;

    int plane_index = apvs[ad].apv_pos;

    strip = plane_index * 128 + strip;

    return strip;
}


////////////////////////////////////////////////////////////////////////////////
// extract all MPD IDs

void Mapping::ExtractMPDAddress()
{
    if(!map_loadded) {
        std::cout<<"Error in mapping:: mapping file not yet loadded."<<std::endl;
        exit(0);
    }

    for(auto &i: apvs)
    {
        MPDAddress mpd_addr(i.first.crate_id, i.first.mpd_id);
        if(std::find(vMPDAddr.begin(), vMPDAddr.end(), mpd_addr) == vMPDAddr.end())
            vMPDAddr.push_back(mpd_addr);
    }

    std::sort(vMPDAddr.begin(), vMPDAddr.end());
}

////////////////////////////////////////////////////////////////////////////////
// get all MPD IDs

const std::vector<MPDAddress> & Mapping::GetMPDAddressVec() const
{
    if(!map_loadded) {
        std::cout<<"Error in mapping:: mapping file not yet loadded."<<std::endl;
        exit(0);
    }

    return vMPDAddr;
}


////////////////////////////////////////////////////////////////////////////////
// get total number of MPDs

int Mapping::GetTotalMPDs()
{
    if(!map_loadded) {
        std::cout<<"Error in mapping:: mapping file not yet loadded."<<std::endl;
        exit(0);
    }

    int res = static_cast<int>(vMPDAddr.size());
    return res;
}

};
