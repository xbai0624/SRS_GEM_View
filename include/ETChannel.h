#ifndef ET_CHANNEL_H
#define ET_CHANNEL_H

#include "ETViewer.h"
#include "et.h"
#include <vector>

class ETChannel
{
public:
    ETChannel();
    ~ETChannel();

    void SetETViewer(ETViewer* v){et_viewer = v;}
    ETViewer* GetETViewer() {return et_viewer;}
    void Init();
    void KillET();

    void GetOneLiveEvent(uint32_t **pBuf, uint32_t &fBufLen, std::vector<uint32_t> &vBuf);

private:
    ETViewer *et_viewer = nullptr;

    // et system
    et_sys_id id;
    et_openconfig config;

    // et station
    et_stat_id stat_id;
    et_statconfig sconfig;

    // et attach
    et_att_id att_id;

    // et event
    et_event *pe;
    struct timespec time;
    void *tmp;
    void **event_buf;
};

#endif
