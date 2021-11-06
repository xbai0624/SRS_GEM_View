#ifndef HISTO_WIDGET_H
#define HISTO_WIDGET_H

#include <vector>
#include <QGraphicsScene>
#include "HistoView.h"
#include "HistoItem.h"
#include "ConfigSetup.h"

#include "MPDDataStruct.h"

class HistoWidget : public QWidget
{
    Q_OBJECT
public:
    HistoWidget(QWidget *parent = nullptr);
    ~HistoWidget();

    void resize(int, int);
    void Refresh();

    void Divide(int r, int c);
    void ReDistributePaintingArea();
    void ReInitHistoItems();

    void DrawCanvas(const std::vector<std::vector<int>> &data, 
            const std::vector<APVAddress> &addr, int, int);

    void Clear();

protected:
    void resizeEvent(QResizeEvent *e);
    void mousePressEvent(QMouseEvent *event);

private:
    QGraphicsScene *scene;
    HistoView *view;
    HistoItem **pItem = nullptr;

    // divide the whole area into fCol by fRow sections
    int fRow = config_setup::nRow;
    int fCol = config_setup::nCol;
};

#endif
