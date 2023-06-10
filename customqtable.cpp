#include "customqtable.h"

#include <QMouseEvent>

CustomQTable::CustomQTable(QWidget *parent):QTableWidget(parent)
{

}

CustomQTable::~CustomQTable()
{

}

void CustomQTable::mousePressEvent(QMouseEvent *event)
{
    setCurrentIndex(QModelIndex());
    QTableView::mousePressEvent(event);
    setSelectionBehavior(QAbstractItemView::SelectRows);
}


