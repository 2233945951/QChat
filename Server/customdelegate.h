#ifndef CUSTOMDELEGATE_H
#define CUSTOMDELEGATE_H
#include <QStyledItemDelegate>
class CustomDelegate:public QStyledItemDelegate
{
    Q_OBJECT
public:
    CustomDelegate();
protected:
    // editing
    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
};

#endif // CUSTOMDELEGATE_H
