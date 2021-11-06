/*
 * test ssp decoder
 */

#include "EvioFileReader.h"
#include "EventParser.h"
#include "MPDVMERawEventDecoder.h"
#include "MPDSSPRawEventDecoder.h"
#include "GEMPedestal.h"

#include <iostream>
#include <string>

#include <TH1I.h>
#include <TFile.h>

////////////////////////////////////////////////////////////////
// An example for how to generate pedestal files

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    if(argc != 2) {
        std::cout<<"Usage: ./main <raw_evio_file>"<<std::endl;
        exit(0);
    }

    std::string evio_file = argv[1];
    std::cout<<"Generating pedestal from file: "<<evio_file<<std::endl;

    GEMPedestal *pedestal = new GEMPedestal();
    pedestal -> SetDataFile(evio_file.c_str());
    pedestal -> SetNumberOfEvents(-1);

    std::cout<<"analyzing pedestal...."<<std::endl;
    pedestal -> CalculatePedestal();

    std::cout<<"Saving pedestal file to: ./gem_ped.root"<<std::endl;
    pedestal -> SavePedestalHisto("gem_ped.root");

    return 0;
}
