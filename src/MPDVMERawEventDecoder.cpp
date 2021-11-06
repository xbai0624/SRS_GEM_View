#include "MPDVMERawEventDecoder.h"
#include "RolStruct.h"

#include <iostream>

////////////////////////////////////////////////////////////////
// ctor

MPDVMERawEventDecoder::MPDVMERawEventDecoder()
{
    // place holder
}

////////////////////////////////////////////////////////////////
// dtor

MPDVMERawEventDecoder::~MPDVMERawEventDecoder()
{
    // place holder
}

////////////////////////////////////////////////////////////////
// decode mpd vme raw data

void MPDVMERawEventDecoder::Decode(const uint32_t *pBuf, uint32_t fBufLen,
        std::vector<int> &vTagTrack)
{
    DecodeAPV(pBuf, fBufLen, vTagTrack);
}

////////////////////////////////////////////////////////////////
// decode mpd vme raw data. It seems pretty lengthy, but this 
// DecodeAPV() routine relys on hardware design, It is highly
// possible that it will be changed later

void MPDVMERawEventDecoder::DecodeAPV(const uint32_t *pBuf, uint32_t fBufLen,
        std::vector<int> &vTagTrack)
{
    APVAddress apv_addr; // apv address
    int mpd_id = 0, adc_ch = 0;
    // crate id was was passed by upper level ROC id: vTagTrack[1] (vTagTrack[0] is current level tag)
    int  crate_id = vTagTrack[1]; 

    for(uint32_t i=0;i<fBufLen; i++)
    {
        const uint32_t & data_word = pBuf[i];
        MPD_VME_Raw_Data_Word word(data_word);

        switch(word.type)
        {
            case MPD_VME_Raw_Data_Type::Block_Header:
                mpd_id = word.mpd_id;
                break;
            case MPD_VME_Raw_Data_Type::Block_Trailer:
                break;
            case MPD_VME_Raw_Data_Type::Event_Header:
                break;
            case MPD_VME_Raw_Data_Type::Trigger_Time:
                break;
            case MPD_VME_Raw_Data_Type::APV_Ch_Data:
                switch(word.apv_ch_data_info)
                {
                    case APV_Ch_Data_Info::APV_Header:
                        {
                            adc_ch = word.adc_ch;
                            APVAddress _ad(crate_id, mpd_id, adc_ch);
                            apv_addr = _ad;
                        }
                        break;
                    case APV_Ch_Data_Info::ADC_Value:
                        mAPVData[apv_addr].push_back(word.adc);
                        break;
                    case APV_Ch_Data_Info::APV_Trailer:
                        mAPVData[apv_addr].push_back(word.apv_trailer);
                        break;
                    case APV_Ch_Data_Info::Trailer:
                        break;
                    default:
                        break;
                }
                break;
            case MPD_VME_Raw_Data_Type::Event_Trailer:
                break;
            case MPD_VME_Raw_Data_Type::Crate_Id: 
                crate_id = word.crate_id;
                break;
            case MPD_VME_Raw_Data_Type::Filler_Word:
                break;
            default:
                break;
        }
    }
}

////////////////////////////////////////////////////////////////
// get decoded apv data

const std::unordered_map<APVAddress, std::vector<int>> & MPDVMERawEventDecoder::GetAPV() 
    const
{
    return mAPVData;
}

////////////////////////////////////////////////////////////////
// clear for next event

void MPDVMERawEventDecoder::Clear() 
{
    mAPVData.clear();
}


