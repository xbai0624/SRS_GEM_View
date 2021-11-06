#ifndef CONFIG_SETUP_H
#define CONFIG_SETUP_H

namespace config_setup {

const int nRow = 4;  //  rows per canvas
const int nCol = 4; //  columns per canvas

const int time_sample = 6;
const int apv_header_level = 1500;

//#define USE_SSP     // ssp setup use this
//#define USE_VME     // vme setup use this
#define USE_SRS     // SRS setup use this

};

#endif
