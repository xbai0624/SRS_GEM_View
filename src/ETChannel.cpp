#include "ETChannel.h"
#include <iostream>
#include <string>

#include <arpa/inet.h>
#include <unistd.h>

ETChannel::ETChannel()
{
}

ETChannel::~ETChannel()
{
    // kill et
    KillET();
}

void ETChannel::Init()
{
    if(et_viewer == nullptr) {
        std::cout<<"ERROR::ETViewer is not configured."<<std::endl;
        return;
    }

    std::string ip = et_viewer -> GetHostIPAddress();
    std::string port = et_viewer -> GetHostPort();
    std::string memory_file = et_viewer -> GetMemoryMappedFile();
    int time = et_viewer -> GetPollTimeInterval();

    // 1 open et system
    et_open_config_init(&config);
    et_open_config_sethost(config, ip.c_str());
    et_open_config_setcast(config, ET_DIRECT);
    et_open_config_setserverport(config, std::stoi(port));

    int open_status = -1;
    while(open_status != 0) {
        std::cout<<"INFO:: Openning ET, please make sure ET is alive..."<<std::endl;
        open_status = et_open(&id, memory_file.c_str(), config);
        sleep(time);
    }
    std::cout<<"SUCCESS:: et open with status: "<<open_status<<std::endl;
    et_open_config_destroy(config);

    // 2 create station
    et_station_config_init(&sconfig);
    et_station_config_setselect(sconfig, ET_STATION_SELECT_ALL);
    et_station_config_setblock(sconfig, ET_STATION_NONBLOCKING);
    et_station_config_setcue(sconfig, 10);
    et_station_config_setuser(sconfig, ET_STATION_USER_MULTI);
    //et_station_config_setrestore(sconfig, ET_STATION_RESTORE_GC);
    et_station_config_setrestore(sconfig, ET_STATION_RESTORE_OUT);
    et_station_config_setprescale(sconfig, 1);

    int station_status = -1;
    while(station_status != 0) {
        std::cout<<"INFO:: Creating ET station, please make sure you have acess to write disk..."<<std::endl;
        station_status = et_station_create(id, &stat_id, "xinzhan's test et station", sconfig);
        sleep(time);
    }
    std::cout<<"SUCCESS:: et station created: "<<station_status<<std::endl;
    et_station_config_destroy(sconfig);

    // 3 attach to station
    int attach_status = -1;
    while(attach_status != 0) {
        std::cout<<"INFO:: Attaching ET station, please make sure ET station is created..."<<std::endl;
        attach_status = et_station_attach(id, stat_id, &att_id);
        sleep(time);
    }
    std::cout<<"SUCCESS:: et station attached..."<<std::endl;

    // 4 fine tuning et
    et_system_setdebug(id, ET_DEBUG_INFO);
}

void ETChannel::GetOneLiveEvent(uint32_t **pBuf, uint32_t &fBufLen)
{
    if(!et_alive(id))
        return;

    int status = et_event_get(id, att_id, &pe, ET_ASYNC, &time); // non-blocking

    if(status != ET_OK)
        return;

    //std::cout<<"debug: event length: "<<pe->length<<std::endl;
    int event_length = (int)pe->length;

    char buf = 'c';
    tmp = (void*)(&buf);
    event_buf = &(tmp);

    if(tmp == NULL)
        std::cout<<"destination buffer is NULL."<<std::endl;

    if(pe->pdata == NULL)
        std::cout<<"event buffer is NULL."<<std::endl;

    status = et_event_getdata(pe, event_buf);

    // switch endianess, need to switch from internet format to local format
    for(int i=0; i<event_length/4; i++) {
        ((uint32_t*)(*event_buf))[i] = ntohl(((uint32_t*)(*event_buf))[i]);
    }

    // need to shift by 8 uint32_t words, to skip coda event header, decoder won't handle this header
    //parser -> ParseEvent((uint32_t*)(*tmp) + 8, event_length/4);
    *pBuf = (uint32_t*)(*event_buf) + 8;
    fBufLen = event_length / 4;

    // put event back
    et_event_put(id, att_id, pe);
}

void ETChannel::KillET()
{
    et_kill(id);
}
