#include "Viewer.h"
#include "ConfigSetup.h"

#include <QGraphicsRectItem>
#include <QSpinBox>
#include <QLabel>
#include <QTabWidget>
#include <QFileDialog>
#include <QGridLayout>
#include <QMessageBox>
#include <QScrollBar>
#include <QCoreApplication>

#include <iostream>
#include <fstream>

//#define USE_THREAD

#ifdef USE_THREAD
//#include <thread>
#endif

////////////////////////////////////////////////////////////////
// ctor

Viewer::Viewer(QWidget *parent) : QWidget(parent)
{
    InitGui();
    InitAnalyzer();

    resize(sizeHint());
}


////////////////////////////////////////////////////////////////
// init gui

void Viewer::InitGui()
{
    SetNumberOfTabs();

    InitLayout();
    AddMenuBar();

    // need to init right first, b/c left require valid
    // 'pRightCanvas' pointer to convey
    InitRightView();
    InitLeftView();

    setWindowTitle("Live Data Viewer");
}

////////////////////////////////////////////////////////////////
// init layout

void Viewer::InitLayout()
{
    // main layout
    pMainLayout = new QVBoxLayout(this);   // menubar + content

    // the whole drawing area
    pDrawingArea = new QWidget(this);

    // content layout
    pDrawingLayout = new QHBoxLayout(pDrawingArea);    // left content + right content

    // left content
    pLeft = new QWidget(pDrawingArea);
    pLeftLayout = new QVBoxLayout(pLeft);       // left content

    // right content
    pRight = new QWidget(pDrawingArea);
    pRightLayout = new QVBoxLayout(pRight);      // right content

    pDrawingLayout -> addWidget(pLeft);
    pDrawingLayout -> addWidget(pRight);

    pMainLayout -> addWidget(pDrawingArea);
}

////////////////////////////////////////////////////////////////
// add a menu bar

void Viewer::AddMenuBar()
{
    pMenuBar = new QMenuBar();

    // file menu
    pMenu = new QMenu("File");
    pMenuBar -> addMenu(pMenu);
    pMenu -> addAction("Save");
    pMenu -> addAction("Exit");

    // view menu
    pMenuBar -> addMenu(new QMenu("View"));
    // Edit menu
    pMenuBar -> addMenu(new QMenu("Edit"));
    // Format menu
    pMenuBar -> addMenu(new QMenu("Format"));
    // Help Menu
    pMenuBar -> addMenu(new QMenu("Help"));

    this->layout()->setMenuBar(pMenuBar);
}

////////////////////////////////////////////////////////////////
// init left drawing area

void Viewer::InitLeftView()
{
    InitLeftTab();
}

////////////////////////////////////////////////////////////////
// init right drawing area

void Viewer::InitRightView()
{
    // right content (show selected apv)
    pRightCanvas = new QWidget(pRight);

    // right control interface
    pRightCtrlInterface = new QWidget(pRight);

    // fix the width in the right side layout
    pRight -> setFixedWidth(300);

    InitCtrlInterface();

    // setup a printing log window
    pLogBox = new QTextEdit(pRight);
    pLogBox -> setTextColor(QColor("black"));
    pLogBox -> textCursor().insertText("System Log:\n");
    pLogBox -> setFixedHeight(200);
    pLogBox -> setEnabled(false);

    pRightLayout -> addWidget(pRightCtrlInterface);
    pRightLayout -> addWidget(pRightCanvas);
    pRightLayout -> addWidget(pLogBox);

    minimum_qt_unit_height(pRightCtrlInterface, pRightCanvas, pLogBox);
}


////////////////////////////////////////////////////////////////
// setup the control interface

void Viewer::InitCtrlInterface()
{
    QVBoxLayout *layout = new QVBoxLayout(pRightCtrlInterface);

    // ET viewer
    et_viewer = new ETViewer(pRightCtrlInterface);
    et_viewer -> SetViewer(this);
    timer = new QTimer(this);
    layout -> addWidget(et_viewer);

    // a file path input window
    QHBoxLayout *_layout1 = new QHBoxLayout();
    QLabel *l1 = new QLabel("File:", pRightCtrlInterface);
    file_indicator = new QLineEdit(pRightCtrlInterface);
    QPushButton *bOpenFile = new QPushButton("&Choose File", pRightCtrlInterface);
    file_indicator -> setText("data/gem_cleanroom_1440.evio.0");
    _layout1 -> addWidget(l1);
    _layout1 -> addWidget(file_indicator);
    _layout1 -> addWidget(bOpenFile);

    // a event number input window
    QHBoxLayout *_layout2 = new QHBoxLayout();
    QLabel *l2 = new QLabel("Event Number: ", pRightCtrlInterface);
    QSpinBox *event_number = new QSpinBox(pRightCtrlInterface);
    event_number -> setRange(0, 9999);
    _layout2 -> addWidget(l2);
    _layout2 -> addWidget(event_number);

    // function modules for generating pedestals 
    QGridLayout *_layout3 = new QGridLayout();
    // 1) max event for pedestal
    QLabel *l_num = new QLabel("Max events for pedestal: ", pRightCtrlInterface);
    QLineEdit *le_num = new QLineEdit(pRightCtrlInterface);
    le_num -> setText("3000");
    // 2) output path
    QLabel *l_path = new QLabel("Pedestal text File Output Path: ", pRightCtrlInterface);
    QLineEdit *le_path = new QLineEdit(pRightCtrlInterface);
    le_path -> setText("database/gem_ped.root");
    // 3) generate
    QLabel *l3 = new QLabel("Generate Pedestal:", pRightCtrlInterface);
    QPushButton *b = new QPushButton("&Generate", pRightCtrlInterface);
    _layout3 -> addWidget(l_num, 0, 0);
    _layout3 -> addWidget(le_num, 0, 1);
    _layout3 -> addWidget(l_path, 1, 0);
    _layout3 -> addWidget(le_path, 1, 1);
    _layout3 -> addWidget(l3, 2, 0);
    _layout3 -> addWidget(b, 2, 1);

    // function moudles for replaying evio file to root files
    QGridLayout *_layout4 = new QGridLayout();
    QLabel *l_replay = new QLabel("Replay File Output Path:", pRightCtrlInterface);
    QLineEdit *le_replay = new QLineEdit(pRightCtrlInterface);
    le_replay -> setText("./gem_replay.root");
    QLabel *l_split = new QLabel("Max File Split", pRightCtrlInterface);
    QLineEdit *le_split = new QLineEdit(pRightCtrlInterface);
    le_split -> setText("-1");
    QLabel *l4 = new QLabel("Replay to ROOT file: ", pRightCtrlInterface);
    QPushButton *b4 = new QPushButton("&Replay", pRightCtrlInterface);
    _layout4 -> addWidget(l_replay, 0, 0);
    _layout4 -> addWidget(le_replay, 0, 1);
    _layout4 -> addWidget(l_split, 1, 0);
    _layout4 -> addWidget(le_split, 1, 1);
    _layout4 -> addWidget(l4, 2, 0);
    _layout4 -> addWidget(b4, 2, 1);

    minimum_qt_unit_height(l1, file_indicator, bOpenFile, l2,  event_number,
		    l_num, le_num, l_path, le_path, l3, b,
		    l_replay, le_replay,
		    l_replay, le_replay, l4, b4);

    // add to overall layout
    layout -> addLayout(_layout1);
    layout -> addLayout(_layout2);
    layout -> addLayout(_layout3);
    layout -> addLayout(_layout4);

    // connect
    connect(file_indicator, SIGNAL(textChanged(const QString &)), this, SLOT(SetFile(const QString &)));
    connect(event_number, SIGNAL(valueChanged(int)), this, SLOT(DrawOfflineEvent(int)));
    connect(bOpenFile, SIGNAL(pressed()), this, SLOT(OpenFile()));
    connect(b, SIGNAL(pressed()), this, SLOT(GeneratePedestal_obsolete()));
    connect(le_path, SIGNAL(textChanged(const QString &)), this, SLOT(SetPedestalOutputPath(const QString &)));
    connect(le_num, SIGNAL(textChanged(const QString &)), this, SLOT(SetPedestalMaxEvents(const QString &)));
    connect(le_replay, SIGNAL(textChanged(const QString &)), this, SLOT(SetRootFileOutputPath(const QString &)));
    connect(le_split, SIGNAL(textChanged(const QString &)), this, SLOT(SetFileSplit(const QString &)));
    connect(b4, SIGNAL(pressed()), this, SLOT(Replay()));

    // timer
    connect(timer, SIGNAL(timeout()), this, SLOT(DrawOnlineEvent()));
    timer -> start(5000);
}
 
////////////////////////////////////////////////////////////////
// setup the tabs in left drawing area

void Viewer::InitLeftTab()
{
    ClearLeftTab();

    pLeftTab = new QTabWidget(pLeft);

    auto v_mpd_addr = apv_strip_mapping::Mapping::Instance()->GetMPDAddressVec();

    for(int i=0;i<nTab+1;i++)  // always built one extra tab
    {
        QWidget *tabWidget = new QWidget(pLeftTab);
        QVBoxLayout *tabWidgetLayout = new QVBoxLayout(tabWidget);
        HistoWidget *c = new HistoWidget(tabWidget);
        tabWidgetLayout -> addWidget(c);
        vTabCanvas.push_back(c);

        QString s;
        //s.sprintf("crate %d mpd %d", v_mpd_addr[i].crate_id, v_mpd_addr[i].mpd_id);
        s.sprintf("raw plots %d", i);
        pLeftTab -> addTab(tabWidget, s);
    }

    pLeftLayout -> addWidget(pLeftTab);
}
 
////////////////////////////////////////////////////////////////
// clear the tabs in left drawing area

void Viewer::ClearLeftTab()
{
    if(pLeftTab == nullptr)
        return;

    for(auto &i: vTabCanvas)
        delete i;

    vTabCanvas.clear();

    if(pLeftTab != nullptr) 
        delete pLeftTab;
}

////////////////////////////////////////////////////////////////
// set how many tabs for showing histograms
// rely on mapping

void Viewer::SetNumberOfTabs()
{
    nTab = apv_strip_mapping::Mapping::Instance()->GetTotalMPDs();

    // right now mapping is not used for ssp
    if(nTab <= 1)
        nTab = 1;
}


////////////////////////////////////////////////////////////////
// set file to analyzer

void Viewer::SetFile([[maybe_unused]] const QString & s)
{
    fFile = s.toStdString();
    if(!FileExist(fFile.c_str()))
    {
        std::cout<<"Error:: cannot find data file: \""<<fFile<<"\"."
                 <<std::endl;
        return;
    }

    if(fFile.find("evio") == std::string::npos &&
            fFile.find("dat") == std::string::npos )
    {
        std::cout<<"Error:: only evio or dat file are accepted."
                 <<std::endl;
        return;
    }

    std::cout<<"Openning file: "<<fFile<<std::endl;

    pAnalyzer -> CloseFile();
    pAnalyzer -> SetFile(s.toStdString().c_str());
    pAnalyzer -> Init();
    pAnalyzer -> OpenFile();
    pRightCtrlInterface -> findChild<QSpinBox*>() -> setValue(0);
    event_number_checked = 0;
}

////////////////////////////////////////////////////////////////
//  init gem analyzer

void Viewer::InitAnalyzer()
{
    pAnalyzer = new Analyzer();
    pAnalyzer -> SetFile(fFile.c_str());
    //pAnalyzer -> Init();

    pGEMReplay = new GEMReplay();
}

////////////////////////////////////////////////////////////////
// draw event

void Viewer::DrawOfflineEvent(int num)
{
    DrawOfflineGEMRawHistos(num);
}

////////////////////////////////////////////////////////////////
// draw gem raw event

void Viewer::DrawOfflineGEMRawHistos([[maybe_unused]]int num)
{
    std::map<APVAddress, std::vector<int>> mData;

    // forward
    if(num > event_number_checked)
    {
        // get apv raw histos
        auto _mData = pAnalyzer->GetEvent();
        if(_mData.size() <= 0) return;

        // sort raw histos
        for(auto &i: _mData)
            mData[i.first] = i.second;

        event_number_checked = num;
        event_cache.push_back(mData);
        if(event_cache.size() > max_cache_events)
        {
            event_cache.pop_front();
        }
    }
    // backward
    else if(num < event_number_checked)
    {
        size_t index = event_number_checked - num;
        if(index >= event_cache.size())
            return;

        size_t pos = event_cache.size()-1 - index;
        mData = event_cache.at(pos);
    }
    else
    {
        if(event_cache.size() <= 0) return;
        mData = event_cache.back();
    }

    drawRawHistos_impl(mData);
}

////////////////////////////////////////////////////////////////
// check if file exists

void Viewer::drawRawHistos_impl(const std::map<APVAddress, std::vector<int>> &mData)
{
    // see if need to prepare more tabs
    int nItemPerCanvas = config_setup::nRow * config_setup::nCol;
    int n_tabs = mData.size()/nItemPerCanvas;
    if(mData.size() % nItemPerCanvas != 0)
        n_tabs += 1;
    if(n_tabs != nTab) {
        nTab = n_tabs;
        InitLeftTab();
    }

    // print a log 
    std::string ss("total apv in current event : ");
    ss = ss + std::to_string(mData.size()) + "\n";
    pLogBox -> textCursor().insertText(ss.c_str());
    pLogBox -> verticalScrollBar()->setValue(pLogBox->verticalScrollBar()->maximum());
    
    std::vector<std::vector<int>> vH[nTab];
    std::vector<APVAddress> vAddr[nTab];
    int raw_index = 0;
    for(auto &i: mData) {
        MPDAddress mpd_addr(i.first.crate_id, i.first.mpd_id);
        int index = raw_index / nItemPerCanvas;
        if(index >= 0 && index < nTab) {
            vH[index].push_back(i.second);
            vAddr[index].push_back(i.first);
        }
        raw_index++;
    }

    // draw tab main canvas
    for(int i=0;i<nTab;i++) 
    {
        vTabCanvas[i] -> Clear();
        vTabCanvas[i] -> DrawCanvas(vH[i], vAddr[i], config_setup::nRow, config_setup::nCol);
        vTabCanvas[i] -> Refresh();
    }

    // draw right canvas
    if(mData.size() > 0) {
        std::vector<std::vector<int>> temp;
        std::vector<APVAddress> temp_addr;
        temp.push_back(mData.begin()->second);
        temp_addr.push_back(mData.begin()->first);
        //pRightCanvas->DrawCanvas(temp, temp_addr, 1, 1);
    }
}

////////////////////////////////////////////////////////////////
// check if file exists

void Viewer::DrawOnlineEvent()
{
    if(et_viewer -> IsOfflineMode())
        return;

    //std::cout<<"Viewer::get et event..."<<std::endl;
    std::unordered_map<APVAddress, std::vector<int>> data = et_viewer
        -> GetOneETEvent();

    // sort event
    std::map<APVAddress, std::vector<int>> mData;
    // sort raw histos
    for(auto &i: data)
        mData[i.first] = i.second;

    drawRawHistos_impl(mData);
}

////////////////////////////////////////////////////////////////
// check if file exists

bool Viewer::FileExist(const char* path)
{
    std::ifstream infile(path);
    return infile.good();
}

////////////////////////////////////////////////////////////////
// open file dialog

void Viewer::OpenFile()
{
    QString filename = QFileDialog::getOpenFileName(
            this,
            "Open Document",
            //QDir::currentPath(),
            "/home/daq/coda/data",
            "All files (*.*) ;; evio files (*.evio)");

    fFile = filename.toStdString();
    if(fFile.size() <= 0)
        return;

    file_indicator -> setText(filename);
}

////////////////////////////////////////////////////////////////
// set pedestal output file path

void Viewer::SetPedestalOutputPath(const QString &s)
{
    fPedestalSavePath = s.toStdString();
}

////////////////////////////////////////////////////////////////
// set root file output path

void Viewer::SetRootFileOutputPath(const QString &s)
{
    fRootFileSavePath = s.toStdString();
}

////////////////////////////////////////////////////////////////
// set input file split

void Viewer::SetFileSplit(const QString &s)
{
    std::string ss = s.toStdString();
    int i = std::stoi(ss);

    fFileSplit = i;
}

////////////////////////////////////////////////////////////////
// set pedestal max events

void Viewer::SetPedestalMaxEvents(const QString & s)
{
    std::string ss = s.toStdString();
    int i = std::stoi(ss);
    if(i <= 0) {
        std::cout<<"Warning: max events for generating pedestal is invalid"
            <<std::endl;
        std::cout<<"     using default 3000."<<std::endl;
        return;
    }
    fPedestalMaxEvents = i;
}


////////////////////////////////////////////////////////////////
// generate pedestal (obsolete)

void Viewer::GeneratePedestal_obsolete()
{
    QMessageBox::information(
            this,
            tr("Live Data Viewer"),
            tr("Generating Pedestals usually take ~30 seconds. \
                \nPress OK to start...") );

    pLogBox -> setTextColor(QColor("blue"));
    pLogBox -> textCursor().insertText("\ngenerating pedestal for file: \"");
    pLogBox -> textCursor().insertText(fFile.c_str());
    pLogBox -> textCursor().insertText("\"\nthis might take a while...\n");
    pLogBox -> verticalScrollBar()->setValue(pLogBox->verticalScrollBar()->maximum());
    QCoreApplication::processEvents();
#ifndef USE_THREAD
    pAnalyzer -> GeneratePedestal(fPedestalSavePath.c_str());
#else
    //std::thread th([&]() {
    //        pAnalyzer -> GeneratePedestal(fPedestalSavePath.c_str());
    //        }
    //        );
    //th.join();
#endif
    QMessageBox::information(
            this,
            tr("MPD GEM Viewer"),
            tr("Pedestal Done!") );

    pLogBox -> setTextColor(QColor("black"));
    pLogBox -> textCursor().insertText("Done.\n");
}

////////////////////////////////////////////////////////////////
// generate pedestal

void Viewer::GeneratePedestal()
{
    QMessageBox::information(
            this,
            tr("GEM Data Viewer"),
            tr("Generating Pedestals usually takes a while... \
                \nPress OK to start...") );

    pLogBox -> setTextColor(QColor("blue"));
    pLogBox -> textCursor().insertText("\ngenerating pedestal for file: \"");
    pLogBox -> textCursor().insertText(fFile.c_str());
    pLogBox -> textCursor().insertText("\"\nthis might take a while...\n");
    pLogBox -> verticalScrollBar()->setValue(pLogBox->verticalScrollBar()->maximum());
    QCoreApplication::processEvents();

    pGEMReplay -> SetInputFile(fFile);
    pGEMReplay -> SetOutputFile(fPedestalSavePath);
#ifndef USE_THREAD
    pGEMReplay -> GeneratePedestal();
#else
    //std::thread th([&]() {
    //        pGEMReplay -> GeneratePedestal();
    //        }
    //        );
    //th.join();
#endif
    QMessageBox::information(
            this,
            tr("GEM Data Viewer"),
            tr("Pedestal Done!") );

    pLogBox -> setTextColor(QColor("black"));
    pLogBox -> textCursor().insertText("Done.\n");
}

////////////////////////////////////////////////////////////////
// replay

void Viewer::Replay()
{
    QMessageBox::information(
            this,
            tr("GEM Data Viewer"),
            tr("Replay files usually takes a while... \
                \nPress OK to start...") );

    pLogBox -> setTextColor(QColor("blue"));
    pLogBox -> textCursor().insertText("\nReplaying for file: \"");
    pLogBox -> textCursor().insertText(fFile.c_str());
    pLogBox -> textCursor().insertText("\"\nthis might take a while...\n");
    pLogBox -> verticalScrollBar()->setValue(pLogBox->verticalScrollBar()->maximum());
    QCoreApplication::processEvents();

    pGEMReplay -> SetInputFile(fFile);
    pGEMReplay -> SetOutputFile(fRootFileSavePath);
    pGEMReplay -> SetSplit(fFileSplit);
#ifndef USE_THREAD
    pGEMReplay -> Replay();
#else
    //std::thread th([&]() {
    //        pGEMReplay -> Replay();
    //        }
    //        );
    //th.join();
#endif
    QMessageBox::information(
            this,
            tr("GEM Data Viewer"),
            tr("Replay Done!") );

    pLogBox -> setTextColor(QColor("black"));
    pLogBox -> textCursor().insertText("Done.\n");
}

////////////////////////////////////////////////////////////////
// replay

void Viewer::SetPollingETTimeInterval(int t)
{
    int time = t * 1000; // convert to mili seconds
    timer -> start(time);
}
