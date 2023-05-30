#ifndef ET_VIEWER_H
#define ET_VIEWER_H

#include <QWidget>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QLineEdit>
#include <QPushButton>
#include <QPixmap>
#include <QTimer>
#include <unordered_map>

#include "MPDDataStruct.h"
#include "EventParser.h"
#include "SRSRawEventDecoder.h"

class ETChannel;
class QLabel;
class Viewer;

class ETViewer : public QWidget
{
    Q_OBJECT

public:
    ETViewer(QWidget *parent = nullptr);
    ~ETViewer();

    bool IsOfflineMode() {return is_offline_mode;}
    bool IsOnlineMode() {return !is_offline_mode;}

    void SetOfflineMode(bool b) {is_offline_mode = b;}
    void SetOnlineMode(bool b) {is_offline_mode = !b;}

    std::string GetHostIPAddress() {return host_ip_address;}
    std::string GetHostPort() {return host_port;}
    std::string GetMemoryMappedFile(){return memory_mapped_file;}
    int GetPollTimeInterval() {return poll_time_interval;}

    void ConnectActions();

    ETChannel *GetETChannel(){return et_channel;}
    std::unordered_map<APVAddress, std::vector<int>> GetOneETEvent();

    void SetViewer(Viewer *v){viewer = v;}

public slots:
    void OfflineButtonSelected();
    void OnlineButtonSelected();
    void SetHostIPAddress(const QString &ip) {host_ip_address = ip.toStdString();}
    void SetHostPort(const QString &port) {host_port = port.toStdString();}
    void SetMemoryFile(const QString &s){memory_mapped_file = s.toStdString();}
    void SetPollTimeInterval(const QString &t);
    void TimerEvent();
    void TurnOnET();
    void TurnOffET();

private:
    QGroupBox *ExclusiveGroup();

    bool is_offline_mode = true;

    // default values
    std::string host_ip_address = "127.0.0.1";
    std::string host_port = "23911";
    std::string memory_mapped_file = "/tmp/et_fermitest_ER1";
    int poll_time_interval = 1; // seconds

    // layout
    QVBoxLayout *layout;

    // mode 
    QGroupBox *groupBox;
    QRadioButton *b_online_mode;
    QRadioButton *b_offline_mode;

    // turn on/off et
    QPushButton *b_turn_on_et;
    QPushButton *b_turn_off_et;
    // et status indicator
    QPixmap *p_indicator;
    QLabel *l_indicator;
    QTimer *timer;
    bool light_on = false;
    bool et_is_alive = false;

    // ip & port & time interval
    QLineEdit *le_ip;
    QLineEdit *le_port;
    QLineEdit *le_time_interval;
    QLineEdit *le_memory_file;

    // et channel
    ETChannel *et_channel = nullptr;
    // parse event
    SRSRawEventDecoder *raw_decoder = nullptr;
    EventParser *event_parser = nullptr;
    // viewer pointer, for setting viwer interval
    Viewer *viewer = nullptr;

public:
    template<typename T> void minimum_qt_unit_height(T b)
    {
        b -> setMinimumHeight(10);
    }
    template<typename T, typename... Args> void minimum_qt_unit_height(T b, Args... args) {
        minimum_qt_unit_height(b);
        minimum_qt_unit_height(args...);
    }

};

#endif
