#include "HistoWidget.h"
#include <iostream>
#include <QHBoxLayout>
#include <cassert>

////////////////////////////////////////////////////////////////////////////////
// ctor

HistoWidget::HistoWidget(QWidget *parent): QWidget(parent)
{
    scene = new QGraphicsScene(this);
    view = new HistoView(scene);

    ReInitHistoItems();
    ReDistributePaintingArea();

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(view);
}

////////////////////////////////////////////////////////////////////////////////
// dtor

HistoWidget::~HistoWidget()
{
}

////////////////////////////////////////////////////////////////////////////////
// resize

void HistoWidget::resize([[maybe_unused]]int x, [[maybe_unused]]int y)
{
}

////////////////////////////////////////////////////////////////////////////////
// resize event

void HistoWidget::resizeEvent([[maybe_unused]]QResizeEvent *e)
{
    ReDistributePaintingArea();
}

////////////////////////////////////////////////////////////////////////////////
// mouse press event

void HistoWidget::mousePressEvent([[maybe_unused]]QMouseEvent *e)
{
    // place holder
}

////////////////////////////////////////////////////////////////////////////////
// init histoitems

void HistoWidget::ReInitHistoItems()
{
    pItem = new HistoItem*[fCol*fRow];

    for(int i=0;i<fCol*fRow;i++) {
        pItem[i] = new HistoItem();
        scene -> addItem(pItem[i]);
    }
}

////////////////////////////////////////////////////////////////////////////////
// similar to root divide canvas

void HistoWidget::Divide(int r, int c)
{
    // clear all previous graphicsitem
    scene -> clear();

    // clear all previous histoitems
    for(int i=0;i<fCol*fRow;i++)
        delete pItem[i];

    delete[] pItem;

    fCol = c; fRow = r;

    // re-init histoitems
    ReInitHistoItems();

    // re-distribute painting area
    ReDistributePaintingArea();
}

////////////////////////////////////////////////////////////////////////////////
// distribute painting area

void HistoWidget::ReDistributePaintingArea()
{
    int w = width();
    int h = height();

    // this margin is for hosting scroll bar etc
    float margin_x = 20;
    float margin_y = 50;
    w = w - margin_x;
    h = h - margin_y;

    // fix scene rect
    scene -> setSceneRect(0, 0, w-margin_x, h);

    float x_interval = w / fCol;
    float y_interval = h / fRow;

    for(int i=0;i<fRow;i++)
        for(int j=0;j<fCol;j++)
        {
            QRectF f(x_interval*j, y_interval*i, x_interval, y_interval);
            pItem[i*fCol + j] -> SetBoundingRect(f);
        }
}

////////////////////////////////////////////////////////////////////////////////
// distribute painting area

void HistoWidget::Refresh()
{
    for(int i=0;i<fRow*fCol;i++)
        pItem[i] -> update();

    update();
}

////////////////////////////////////////////////////////////////////////////////
// distribute painting area

void HistoWidget::DrawCanvas(const std::vector<std::vector<int>> &data, 
            const std::vector<APVAddress> &addr, int row, int col)
{
    if(row != fRow || col != fCol)
    {
        Divide(row, col);
        ReDistributePaintingArea();
        ReInitHistoItems();
    }

    assert(static_cast<int>(data.size()) <= fRow * fCol);

    // fill histoitems
    for(unsigned int i=0; i < data.size(); i++) {
        pItem[i] -> ReceiveContents(data[i]);
        std::string title = "crate_" + std::to_string(addr[i].crate_id) +
                            "_fec_" + std::to_string(addr[i].mpd_id) + 
                            "_apv_id_" + std::to_string(addr[i].adc_ch);
        pItem[i] -> SetTitle(title);
    }
}


////////////////////////////////////////////////////////////////////////////////
// clear

void HistoWidget::Clear()
{
    for(int i=0;i<fRow*fCol;i++)
        pItem[i] -> Clear();
}

