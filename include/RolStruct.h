#ifndef ROL_STRUCT_H
#define ROL_STRUCT_H

////////////////////////////////////////////////////////////////
// this file defines the setups in readout list


////////////////////////////////////////////////////////////////
// eventy type definition

enum class EventType 
{
    PreStart,
    Go,
    Physics,
};

////////////////////////////////////////////////////////////////
// tag (in bank) id

#define MAX_NFEC 8 // max number of FEC cards
enum class Bank_TagID 
{
    FADC    = 3,
    SRS     = 5, // SRS starting bank ID, b/c in SRS setup, each FEC occupies one bank
    TDC     = 6,
    MPD_VME = 10,
    MPD_SSP = 10
};

////////////////////////////////////////////////////////////////
// MPD VME data type identifer
//
// For VME raw data, each 32bit word consists two parts: 
//   higher bit (bit 31 ~ bit X) defines the type of data in the 
//   current word
//   lower bit (bix X ~ 0) is the data payload
//   position X depends on mpd setting

enum class MPD_VME_Raw_Data_Type
{
    Block_Header    = 0x0,
    Block_Trailer   = 0x1,
    Event_Header    = 0x2,
    Trigger_Time    = 0x3,
    APV_Ch_Data     = 0x4,
    Event_Trailer   = 0x5,
    Crate_Id        = 0x6,
    Filler_Word     = 0x7,
    Undefined
};

////////////////////////////////////////////////////////////////
// FADC250 VME data type identifier
enum class FADC250_VME_Raw_Data_Type
{
    Block_Header     = 0,
    Block_Trailer    = 1,
    Event_Header     = 2,
    Trigger_Time     = 3,
    Window_Raw_Data  = 4,
    Pulse_Raw_Data   = 6,
    Pulse_Integral   = 7,
    Pulse_Time       = 8,
    Scaler_Data      = 12,
    Data_Not_Valid   = 14,
    Filler_Word      = 15,
    Undefined
};

#endif
