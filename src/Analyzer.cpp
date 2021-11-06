#include "Analyzer.h"
#include "ConfigSetup.h"
#include "GEMPedestal.h"

#include <iostream>
#include <fstream>

using namespace std;

Analyzer::Analyzer()
{
    Init();
}

Analyzer::~Analyzer()
{
    // place holder
}

void Analyzer::Init()
{
    file_reader = new EvioFileReader();
    event_parser = new EventParser();

#ifdef USE_SSP
        mpd_ssp_decoder = new MPDSSPRawEventDecoder();
        event_parser -> RegisterRawDecoder(static_cast<int>(Bank_TagID::MPD_SSP), mpd_ssp_decoder);
#endif

#ifdef USE_VME
        mpd_vme_decoder = new MPDVMERawEventDecoder();
        event_parser -> RegisterRawDecoder(static_cast<int>(Bank_TagID::MPD_VME), mpd_vme_decoder);
#endif

#ifdef USE_SRS
        srs_decoder = new SRSRawEventDecoder();
        std::cout<<"Info: Reserving the following bank IDs for FECs:"<<std::endl;
        for(int ifec=0; ifec<MAX_NFEC; ++ifec) {
            std::cout<<"  "<<static_cast<int>(Bank_TagID::SRS) + ifec;
            event_parser -> RegisterRawDecoder(static_cast<int>(Bank_TagID::SRS) + ifec, srs_decoder);
        }
        std::cout<<endl<<"Info: Done with registering SRS decoders."<<std::endl;
#endif

    file_is_open = false;
}

void Analyzer::SetFile(const std::string &path)
{
    input_file = path;
    file_reader -> CloseFile();
    file_reader -> SetFile(input_file.c_str());

    event_number = 0;
}


std::unordered_map<APVAddress, std::vector<int>> Analyzer::GetEvent()
{
    std::unordered_map<APVAddress, std::vector<int>> res;

    if(!file_is_open)
        OpenFile();
 
    // open file failed, return
    if(!file_is_open) {
        std::cout<<"Error: failed openning file: "<<input_file<<std::endl;
        return res;
    }
    
    if(file_reader -> ReadNoCopy(&pBuf, &fBufLen) != S_SUCCESS) {
        std::cout<<"Info: finished reading file: "<<input_file<<std::endl;
        return res;
    }

    event_parser -> ParseEvent(pBuf, fBufLen);

#ifdef USE_SSP
        const std::unordered_map<APVAddress, std::vector<int>> & res_dec 
            = mpd_ssp_decoder -> GetAPV();
#endif
#ifdef USE_VME
        const std::unordered_map<APVAddress, std::vector<int>> & res_dec 
            = mpd_vme_decoder -> GetAPV();
#endif
#ifdef USE_SRS
        const std::unordered_map<APVAddress, std::vector<int>> &res_dec
            = srs_decoder -> GetAPV();
#endif

    return res_dec;
}

void Analyzer::GeneratePedestal(const char* path)
{
    // place holder
    GEMPedestal *pedestal = new GEMPedestal();

    std::cout<<"INFO::Generating pedestals from file: "<<input_file<<std::endl;
    pedestal -> SetDataFile(input_file.c_str());
    pedestal -> SetNumberOfEvents(-1);
    pedestal -> CalculatePedestal();

    std::cout<<"INFO::Saving pedestalf to file: "<<path<<std::endl;
    pedestal -> SavePedestalHisto(path);
}

void Analyzer::CloseFile()
{
    file_reader -> CloseFile();

    file_is_open = false;
}

void Analyzer::OpenFile()
{
    std::ifstream infile(input_file.c_str());
    if(!infile.good()) {
        std::cout<<"File: "<<input_file<<" not exist."<<std::endl;
        return;
    }

    std::cout<<"Info: Openning file: "<<input_file<<std::endl;
    SetFile(input_file);

    file_reader -> OpenFile();

    file_is_open = true;
}
