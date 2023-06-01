#include "ETViewer.h"
#include "ETChannel.h"
#include "RolStruct.h"
#include "Viewer.h"

#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>

ETViewer::ETViewer(QWidget *parent) : QWidget(parent)
{
    layout = new QVBoxLayout(this);
    layout -> setContentsMargins(0, 0, 0, 0);

    ExclusiveGroup();
    layout -> addWidget(groupBox);

    QGridLayout *grid_layout = new QGridLayout();
    QLabel *l_ip = new QLabel("Host IP:", this);
    le_ip = new QLineEdit(this);
    le_ip -> setText("127.0.0.1");
    le_ip -> setEnabled(false);
    QLabel *l_port = new QLabel("Host port:", this);
    le_port = new QLineEdit(this);
    le_port -> setText("23911");
    le_port -> setEnabled(false);
    QLabel *l_time = new QLabel("Refresh Time(s):", this);
    le_time_interval = new QLineEdit(this);
    le_time_interval -> setText("5.0");
    le_time_interval -> setEnabled(false);
    QLabel *l_mem = new QLabel("ET memory File:", this);
    le_memory_file = new QLineEdit(this);
    le_memory_file -> setText("/tmp/et_fermitest_ER1");
    le_memory_file -> setEnabled(false);

    grid_layout -> addWidget(l_ip, 0, 0);
    grid_layout -> addWidget(le_ip, 0, 1);
    grid_layout -> addWidget(l_port, 1, 0);
    grid_layout -> addWidget(le_port, 1, 1);
    grid_layout -> addWidget(l_time, 2, 0);
    grid_layout -> addWidget(le_time_interval, 2, 1);
    grid_layout -> addWidget(l_mem, 3, 0);
    grid_layout -> addWidget(le_memory_file, 3, 1);

    // turn on/off et
    QHBoxLayout *h_layout = new QHBoxLayout();
    b_turn_on_et = new QPushButton("Turn On ET", this);
    b_turn_on_et -> setEnabled(false);
    b_turn_off_et = new QPushButton("Turn Off ET", this);
    b_turn_off_et -> setToolTip("No need to turn off. You can exit the program to stop ET.");
    b_turn_off_et -> setEnabled(false); // disable turn off, coda needs et running
    // a et status indicator
    p_indicator = new QPixmap("resources/statusindicator-green.png");
    l_indicator = new QLabel("", this);
    l_indicator -> setPixmap(*p_indicator);
    l_indicator -> setScaledContents(true);
    l_indicator -> setFixedWidth(25);
    l_indicator -> setFixedHeight(25);
    l_indicator -> setEnabled(true);
    QLabel *status_et = new QLabel("ET live:", this);

    h_layout -> addWidget(status_et);
    h_layout -> addWidget(l_indicator);
    h_layout -> addWidget(b_turn_on_et);
    h_layout -> addWidget(b_turn_off_et);

    minimum_qt_unit_height(l_ip, le_ip, l_port, le_port, l_time, le_time_interval, l_mem,
		    le_memory_file, status_et, l_indicator, b_turn_on_et, b_turn_off_et);

    // a timer
    timer = new QTimer(this);

    layout -> addLayout(grid_layout);
    layout -> addLayout(h_layout);

    // connect actions
    ConnectActions();
}

ETViewer::~ETViewer()
{
}

QGroupBox* ETViewer::ExclusiveGroup()
{
    groupBox = new QGroupBox(tr("Viewer Mode"));
    b_offline_mode = new QRadioButton(tr("Offline Mode"));
    b_online_mode = new QRadioButton(tr("Online Mode"));

    b_offline_mode -> setChecked(true);

    QHBoxLayout *hbox = new QHBoxLayout(groupBox);
    hbox -> addWidget(b_offline_mode);
    hbox -> addWidget(b_online_mode);
    hbox -> addStretch(1);

    minimum_qt_unit_height(b_offline_mode, b_online_mode);

    return groupBox;
}

void ETViewer::OfflineButtonSelected()
{
    le_ip -> setEnabled(false);
    le_port -> setEnabled(false);
    le_time_interval -> setEnabled(false);
    le_memory_file -> setEnabled(false);
    b_turn_on_et -> setEnabled(false);

    is_offline_mode = true;
}


void ETViewer::OnlineButtonSelected()
{
    le_ip -> setEnabled(true);
    le_port -> setEnabled(true);
    le_time_interval -> setEnabled(true);
    le_memory_file -> setEnabled(true);
    b_turn_on_et -> setEnabled(true);

    is_offline_mode = false;
}

void ETViewer::ConnectActions()
{
    connect(b_offline_mode, SIGNAL(clicked()), this, SLOT(OfflineButtonSelected()));
    connect(b_online_mode, SIGNAL(clicked()), this, SLOT(OnlineButtonSelected()));
    connect(le_ip, SIGNAL(textChanged(const QString&)), this, SLOT(SetHostIPAddress(const QString &)));
    connect(le_time_interval, SIGNAL(textChanged(const QString &)), this, SLOT(SetPollTimeInterval(const QString &)));

    // timer
    connect(timer, SIGNAL(timeout()), this, SLOT(TimerEvent()));
    timer -> start(800);

    connect(b_turn_on_et, SIGNAL(pressed()), this, SLOT(TurnOnET()));
    connect(b_turn_off_et, SIGNAL(pressed()), this, SLOT(TurnOffET()));
}

void ETViewer::TimerEvent()
{
    if(is_offline_mode || !et_is_alive) {
        l_indicator -> setEnabled(false);
        return;
    }

    l_indicator -> setEnabled(light_on);
    light_on = !light_on;
}

void ETViewer::TurnOnET()
{
    if(et_is_alive) {
	    std::cout<<"ET is already alive. Nothing is needed."<<std::endl;
        return;
    }

    et_is_alive = true;

    et_channel = new ETChannel();
    et_channel -> SetETViewer(this);

    et_channel -> Init();

    // create event parser 
    if(event_parser == nullptr)
        event_parser = new EventParser();
    if(raw_decoder == nullptr)
        raw_decoder = new SRSRawEventDecoder();
    for(int ifec=0; ifec<MAX_NFEC; ++ifec) {
        //std::cout<<"  "<<static_cast<int>(Bank_TagID::SRS) + ifec;
        event_parser -> RegisterRawDecoder(static_cast<int>(Bank_TagID::SRS) + ifec, raw_decoder);
    }
}

void ETViewer::TurnOffET()
{
    if(!et_is_alive)
        return;

    et_is_alive = false;

    et_channel -> KillET();
}

std::unordered_map<APVAddress, std::vector<int>> ETViewer::GetOneETEvent()
{
    std::unordered_map<APVAddress, std::vector<int>> res;

    uint32_t *pBuf = 0;
    uint32_t fBufLen = 0;
    std::vector<uint32_t> vBuf;

    if(et_channel == nullptr)
    {
        std::cout<<"ET is not turned on yet..."<<std::endl;
        return res;
    }

    et_channel -> GetOneLiveEvent(&pBuf, fBufLen, vBuf);

    if(pBuf == nullptr || fBufLen == 0)
        return res;

    if(event_parser == nullptr) {
	    std::cout<<"event parser is not initilized..."<<std::endl;
	    return res;
    }

    event_parser -> ParseEvent(vBuf);
    //event_parser -> ParseEvent(pBuf, fBufLen);
    res = raw_decoder -> GetAPV();

    return res;
}

void ETViewer::SetPollTimeInterval(const QString &t)
{
    poll_time_interval = t.toDouble();

    // minimum polling time: 1 millisecond
    if(poll_time_interval < 0.001)
        return;

    if(viewer == nullptr)
    {
        std::cout<<"Host Viewer for ETViewer is not set."<<std::endl;
        return;
    }
    viewer -> SetPollingETTimeInterval(poll_time_interval);
}
