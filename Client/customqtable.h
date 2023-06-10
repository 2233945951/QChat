#ifndef CUSTOMQTABLE_H
#define CUSTOMQTABLE_H

#include <QTableWidget>
#include <QtDebug>
class CustomQTable :public QTableWidget
{
    Q_OBJECT
public:
    CustomQTable(QWidget *parent);
    ~CustomQTable();

protected:
    void mousePressEvent(QMouseEvent *event) override;

};

#endif // CUSTOMQTABLE_H
