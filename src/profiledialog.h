#ifndef PROFILEDIALOG_H
#define PROFILEDIALOG_H

#include <QDialog>
#include "ui_profiledialog.h"
#include "profile.h"


class ProfileDialog : public QDialog, public Ui::ProfileDialog
{
    Q_OBJECT
    
public:
    explicit ProfileDialog(const QString &name = QString(), QWidget *parent = 0);
    ~ProfileDialog();
    
private:
    Profile m_profile;

    void setButtonColor(const QColor &c, QPushButton *b);
    void closeEvent(QCloseEvent *event);

private slots:
    void loadProfile(const QString &name);
    void currentItemChanged(QListWidgetItem * current, QListWidgetItem * previous);
    void addButton_clicked();
    void deleteButton_clicked();

    void selectLogo(void);
    void imageTextChange(void);
    void horizontalSpinBox_valueChanged(int v);
    void verticalSpinBox_valueChanged(int v);
    void textColorButton_clicked(void);
    void outlineColorButton_clicked();
    void transparency_valueChanged(int value);
    void plainTextEdit_textChanged();
    void font_changed();

    void accept();

};

#endif // PROFILEDIALOG_H
