#include "MPDSSPRawEventDecoder.h"
#include "sspApvdec.h"
#include <iostream>
#include <cassert>


////////////////////////////////////////////////////////////////
// static words for getting information during ssp decoding, 
// a temporary solution, needs to be removed

static uint32_t type_last = 15;	/* initialize to type FILLER WORD */
static uint32_t time_last = 0;
static int new_type = 0;
static int apv_data_word = 0;
static bool current_strip_finished = false;

////////////////////////////////////////////////////////////////
// ctor

MPDSSPRawEventDecoder::MPDSSPRawEventDecoder()
{
    // ssp readout is very different with vme readout
    // the APVAddress mapping between ssp and vme
    // in below is mostly likely incorrect
    // I am using this mapping for temprary in order 
    // to keep code consistent with vme situation
    // need to confirm whether this mapping is OK or not
    //
    //                  case:        vme    =   ssp
    apvAddress = APVAddress(-1, // crate id = fiber id
                            -1, // mpd id   = slot id
                            -1);// adc ch   = apv id
    vStripADC.clear();
}

////////////////////////////////////////////////////////////////
// dtor

MPDSSPRawEventDecoder::~MPDSSPRawEventDecoder()
{
    // place holder
}

////////////////////////////////////////////////////////////////
// decode interface

void MPDSSPRawEventDecoder::Decode([[maybe_unused]] const uint32_t *pBuf, 
        [[maybe_unused]]uint32_t fBufLen, std::vector<int> &vTagTrack)
{
    // DO NOT use Clear() here, use it in EventParser class, b/c you 
    // only want to Clear() for each new event
    //Clear();

    DecodeAPV(pBuf, fBufLen, vTagTrack);
}

////////////////////////////////////////////////////////////////
// decode mpd ssp raw data
// procedure relay on ssp firmware design

void MPDSSPRawEventDecoder::DecodeAPV(const uint32_t *pBuf, uint32_t fBufLen,
        [[maybe_unused]]std::vector<int> &vTagTrack)
{
    // clear previous event
    mAPVData.clear();

    for(uint32_t i = 0; i<fBufLen; i++)
    {
        sspApvDataDecode(pBuf[i]);

        // discard strip numbers > 128 (apv only has 128 channels), might lose info for debugging
	if( !current_strip_finished || current_strip_number >= 128) 
	    continue;

	// reorganize data into time sample format
	if(mAPVData.find(apvAddress) == mAPVData.end())
	    mAPVData[apvAddress].resize(SSP_TIME_SAMPLE * TS_PERIOD_LEN, 0);

        // get crate id
        apvAddress.crate_id = vTagTrack[1];

	for(int ts = 0; ts < SSP_TIME_SAMPLE; ts++)
	{
	    // debug; temporary solution, to be removed
	    // duplicate APV ID detected
	    if(mAPVData[apvAddress][ts*TS_PERIOD_LEN + current_strip_number] != 0)
	    {
		std::cout<<__func__<<" Warning: duplicated APV detected: "<<apvAddress<<std::endl;
		while( mAPVData.find(apvAddress) != mAPVData.end() &&
			mAPVData[apvAddress][ts*TS_PERIOD_LEN + current_strip_number] != 0)
		{
		    apvAddress.adc_ch += 16;
		}
		if(mAPVData.find(apvAddress) == mAPVData.end() )
		    mAPVData[apvAddress].resize(SSP_TIME_SAMPLE * TS_PERIOD_LEN, 0);
	    }

	    mAPVData[apvAddress][ts*TS_PERIOD_LEN + current_strip_number] = 
		vStripADC[ts];
	}
    }
}

////////////////////////////////////////////////////////////////
// get decoded apv data

const std::unordered_map<APVAddress, std::vector<int>> & MPDSSPRawEventDecoder::GetAPV()
    const
{
    return mAPVData;
}

////////////////////////////////////////////////////////////////
// clear for next event

void MPDSSPRawEventDecoder::Clear()
{
    mAPVData.clear();
}

// a helper to get negative values
static int convert(const uint32_t & word)
{
    int res = (word & 0x1000) ?
              (word & 0x1fff) | 0xFFFFE000 :
              (word & 0x1FFF);

    return res;
}

////////////////////////////////////////////////////////////////
// This decoder version is from Bryan Moffit

void MPDSSPRawEventDecoder::sspApvDataDecode(const uint32_t &data)
{
    current_strip_finished = false;
    //static uint32_t type_last = 15;	/* initialize to type FILLER WORD */
    //static uint32_t time_last = 0;
    //static int new_type = 0;
    int type_current = 0;
    //static int apv_data_word = 0;
    generic_data_word_t gword;

    gword.raw = data;

    if(gword.bf.data_type_defining) /* data type defining word */
    {
        new_type = 1;
        type_current = gword.bf.data_type_tag;
    }
    else
    {
        new_type = 0;
        type_current = type_last;
    }

    switch( type_current )
    {
        case 0:		/* BLOCK HEADER */
            {
                block_header_t d; d.raw = data;

                //printf("%8X - BLOCK HEADER - slot = %d  modID = %d   n_evts = %d   n_blk = %d\n",
                //        d.raw,
                //        d.bf.slot_number,
                //        d.bf.module_ID,
                //        d.bf.number_of_events_in_block,
                //        d.bf.event_block_number);

                //apvAddress.crate_id = d.bf.slot_number;

                break;
            }

        case 1:		/* BLOCK TRAILER */
            {
                block_trailer_t d; d.raw = data;

                //printf("%8X - BLOCK TRAILER - slot = %d   n_words = %d\n",
                //        d.raw,
                //        d.bf.slot_number,
                //        d.bf.words_in_block);
                break;
            }
        case 2:		/* EVENT HEADER */
            {
                sspApv_event_header_t d; d.raw = data;

                //printf("%8X - EVENT HEADER 1 - trig num = %d\n",
                //        d.raw,
                //        d.bf.trigger_number);
                break;
            }
        case 3:		/* TRIGGER TIME */
            {
                if( new_type )
                {
                    sspApv_trigger_time_1_t d; d.raw = data;

                    //printf("%8X - TRIGGER TIME 1 - time = %08x\n",
                    //        d.raw,
                    //        d.bf.trigger_time_l);

                    time_last = 1;
                }
                else
                {
                    sspApv_trigger_time_2_t d; d.raw = data;
                    //if( time_last == 1 )
                    //{
                    //    printf("%8X - TRIGGER TIME 2 - time = %08x\n",
                    //            d.raw,
                    //            d.bf.trigger_time_h);
                    //}
                    //else
                    //    printf("%8X - TRIGGER TIME - (ERROR)\n", data);

                    time_last = 0;
                }
                break;
            }
        case 5:		/* MPD Frame */
            {
                if( new_type )
                {
                    sspApv_mpd_frame_1_t d; d.raw = data;

                    //printf("%8X - FIBER = %2d  MPD_ID = %2d\n",
                    //        d.raw,
                    //        d.bf.fiber,
                    //        d.bf.mpd_id);

                    apvAddress.mpd_id = d.bf.fiber;

                    apv_data_word = 1;
                }
                else
                {
                    switch(apv_data_word)
                    {
                        case 1:
                            {
                                sspApv_apv_data_1_t d; d.raw = data;
                                //printf("%8X - APV DATA 1 - CHANNEL_NUM(4:0) = %2d S1 = %4x  S0 = %4x\n",
                                //        d.raw,
                                //        d.bf.apv_channel_num_40,
                                //        d.bf.apv_sample1,
                                //        d.bf.apv_sample0);
                                current_strip_number = d.bf.apv_channel_num_40;
                                vStripADC.clear();
                                vStripADC.push_back(static_cast<int>(convert(d.bf.apv_sample0)));
                                vStripADC.push_back(static_cast<int>(convert(d.bf.apv_sample1)));
                                //print();

                                apv_data_word++;
                                break;
                            }
                        case 2:
                            {
                                sspApv_apv_data_2_t d; d.raw = data;
                                //printf("%8X - APV DATA 2 - CHANNEL_NUM(6:5) = %d S3 = %4x  S2 = %4x\n",
                                //        d.raw,
                                //        d.bf.apv_channel_num_65,
                                //        d.bf.apv_sample3,
                                //        d.bf.apv_sample2);
                                current_strip_number |= (d.bf.apv_channel_num_65 << 5);
                                vStripADC.push_back(static_cast<int>(convert(d.bf.apv_sample2)));
                                vStripADC.push_back(static_cast<int>(convert(d.bf.apv_sample3)));
                                //print();

                                apv_data_word++;
                                break;
                            }
                        case 3:
                            {
                                sspApv_apv_data_3_t d; d.raw = data;
                                //printf("%8X - APV DATA 3 - APV_ID = %2d S5 = %4x  S4 = %4x\n",
                                //        d.raw,
                                //        d.bf.apv_id,
                                //        d.bf.apv_sample5,
                                //        d.bf.apv_sample4);
                                apvAddress.adc_ch = d.bf.apv_id;
                                vStripADC.push_back(static_cast<int>(convert(d.bf.apv_sample4)));
                                vStripADC.push_back(static_cast<int>(convert(d.bf.apv_sample5)));
 
                                apv_data_word=1;
                                current_strip_finished = true;
                                break;
                            }
                        default:
                            break;
                    }

                }
                break;
            }
        case 14:		/* DATA NOT VALID (no data available) */
            {
                data_not_valid_t d; d.raw = data;

                //printf("%8X - DATA NOT VALID = %d\n",
                //        d.raw,
                //        d.bf.data_type_tag);
                break;
            }
        case 15:		/* FILLER WORD */
            {
                filler_word_t d; d.raw = data;

                //printf("%8X - FILLER WORD = %d\n",
                //        d.raw,
                //        d.bf.data_type_tag);
                break;
            }
        case 4:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        default:
            {
                //printf("%8X - UNDEFINED TYPE = %d\n",
                //        gword.raw,
                //        gword.bf.data_type_tag);
                break;
            }
    }

    type_last = type_current;	/* save type of current data word */
}

// debug
void MPDSSPRawEventDecoder::print()
{
    std::cout<<"strp: "<<current_strip_number<<std::endl;
    for(auto &i: vStripADC)
        std::cout<<i<<", ";
    std::cout<<std::endl;
}
