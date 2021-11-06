#ifndef ANALYZER_H
#define ANALYZER_H

#include "EvioFileReader.h"
#include "EventParser.h"
#include "MPDVMERawEventDecoder.h"
#include "MPDSSPRawEventDecoder.h"
#include "SRSRawEventDecoder.h"

#include <string>
#include <vector>
#include <unordered_map>

class Analyzer
{
public:
    Analyzer();
    ~Analyzer();

    void Init();
    void SetFile(const std::string & path);

    //const std::unordered_map<APVAddress, std::vector<int>> & GetEvent() const;
    std::unordered_map<APVAddress, std::vector<int>> GetEvent();
    const std::unordered_map<APVAddress, std::vector<int>> & GetData() const;

    void GeneratePedestal(const char* path);
    void CloseFile();
    void OpenFile();

private:
    EvioFileReader *file_reader;
    EventParser *event_parser;
    MPDSSPRawEventDecoder *mpd_ssp_decoder;
    MPDVMERawEventDecoder *mpd_vme_decoder;
    SRSRawEventDecoder *srs_decoder;

    std::string input_file;
    const uint32_t *pBuf;
    uint32_t fBufLen;

    uint32_t event_number = 0;
    bool file_is_open = false;
};

#endif
