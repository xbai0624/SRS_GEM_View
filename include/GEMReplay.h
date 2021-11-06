/*
 * dummy class (implemented in MPD_GEM_Viewer)
 */

#ifndef GEM_REPLAY_H
#define GEM_REPLAY_H

#include <string>

class GEMReplay
{
public:
    GEMReplay();
    ~GEMReplay();

    void GeneratePedestal();
    void Replay();

    void SetInputFile(const std::string &);
    void SetOutputFile(const std::string &);
    void SetSplit(const int &s);

private:
};

#endif
