#ifndef QWATERMARK_H
#define QWATERMARK_H

#include <QMainWindow>
#include <QFileSystemModel>

#include "ui_qwatermark.h"


class Profile;

class QWatermark : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT

public:
    QWatermark(QWidget *parent = 0);

private:
    bool checkDir(const QString& name);
    QString getTargetPath(const QString &fname);

    void paintOne(int w, int h, QPainter *painter, Profile *profile);

    void closeEvent(QCloseEvent *event);

private slots:
    void checkConditions();

    void selectSourceFolder(void);
    void selectDestinationFolder(void);
    void editProfileButton_clicked();

    void doWatermark(void);
    void preview();

    void about(void);

};

class FSModel : public QFileSystemModel
{
public:
    FSModel(QObject *parent = 0);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
};


#endif // QWATERMARK_H
