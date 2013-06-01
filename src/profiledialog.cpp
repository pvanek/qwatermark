#include <QtDebug>
#include <QFileDialog>
#include <QColorDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QSettings>

#include "profiledialog.h"
#include "ui_profiledialog.h"
#include "profile.h"


ProfileDialog::ProfileDialog(const QString &name, QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);

    QSettings s;
    s.beginGroup("ProfileDialog");
    restoreGeometry(s.value("geometry").toByteArray());
    s.endGroup();

    connect(imageRadioButton, SIGNAL(toggled(bool)),
            this, SLOT(imageTextChange(void)));
    connect(textRadioButton, SIGNAL(toggled(bool)),
            this, SLOT(imageTextChange(void)));

    connect(watermarkPushButton, SIGNAL(clicked()),
            this, SLOT(selectLogo(void)));
    connect(textColorButton,SIGNAL(clicked()),
            this, SLOT(textColorButton_clicked(void)));
    connect(outlineColorButton,SIGNAL(clicked()),
            this, SLOT(outlineColorButton_clicked(void)));

    connect(transparencyHorizontalSlider, SIGNAL(valueChanged(int)),
            transparencySpinBox, SLOT(setValue(int)));
    connect(transparencySpinBox, SIGNAL(valueChanged(int)),
            transparencyHorizontalSlider, SLOT(setValue(int)));
    connect(transparencySpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(transparency_valueChanged(int)));

    connect(plainTextEdit, SIGNAL(textChanged()),
            this, SLOT(plainTextEdit_textChanged()));

    connect(fontComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(font_changed()));
    connect(sizeSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(font_changed()));
    connect(boldToolButton, SIGNAL(clicked()),
            this, SLOT(font_changed()));
    connect(italicToolButton, SIGNAL(clicked()),
            this, SLOT(font_changed()));
    connect(underlineToolButton, SIGNAL(clicked()),
            this, SLOT(font_changed()));

    connect(listWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(currentItemChanged(QListWidgetItem*,QListWidgetItem*)));
    connect(addButton, SIGNAL(clicked()), this, SLOT(addButton_clicked()));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteButton_clicked()));

    listWidget->addItems(Profile::getProfiles());
    listWidget->sortItems();
    QList<QListWidgetItem *> l = listWidget->findItems(name.isNull() ? "Default" : name, Qt::MatchExactly);
    if (l.count())
        listWidget->setCurrentItem(l.at(0));
    else
        listWidget->setCurrentRow(0);

    loadProfile(name.isNull() ? "Default" : name);
}

void ProfileDialog::accept()
{
    Profile old = Profile::getProfile(profileLabel->text());
    if (m_profile != old)
    {
        m_profile.save();
    }

    QDialog::accept();
}

void ProfileDialog::closeEvent(QCloseEvent *event)
{
    QSettings s;
    s.beginGroup("ProfileDialog");
    s.setValue("geometry", saveGeometry());
    s.endGroup();

    QWidget::closeEvent(event);
}

ProfileDialog::~ProfileDialog()
{
}

void ProfileDialog::loadProfile(const QString &name)
{
    qDebug() << "loadProfile" << name;
    m_profile = Profile::getProfile(name);

    profileLabel->setText(name);

    textRadioButton->setChecked(m_profile.type() == Profile::Text);
    imageTextChange();

    transparencySpinBox->setValue(m_profile.transparency() * 100);

    watermarkLineEdit->setText(m_profile.logoPath());

    plainTextEdit->setPlainText(m_profile.text());
    fontComboBox->setCurrentFont(m_profile.font());
    sizeSpinBox->setValue(m_profile.font().pointSize());
    qDebug() << "ISBOLD" << m_profile.font().bold();
    boldToolButton->setChecked(m_profile.font().bold());
    italicToolButton->setChecked(m_profile.font().italic());
    underlineToolButton->setChecked(m_profile.font().underline());

    setButtonColor(m_profile.mainColor(), textColorButton);
    setButtonColor(m_profile.outlineColor(), outlineColorButton);
}

//Select the image to use to watermark
void ProfileDialog::selectLogo(void)
{
    QString logoFile = QFileDialog::getOpenFileName(this, tr("Select File"),"",
            tr("Images (*. bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm *.tiff *.xbm *.xpm)"));

    if (!logoFile.isEmpty())
    {
        watermarkLineEdit->setText(logoFile);
        m_profile.setLogoPath(logoFile);
    }
}

//Switch between image and text watermarking
void ProfileDialog::imageTextChange(void)
{
    if (textRadioButton->isChecked())
    {
        typeStackedWidget->setCurrentIndex(0);
        m_profile.setType(Profile::Text);
    }
    else
    {
        typeStackedWidget->setCurrentIndex(1);
        m_profile.setType(Profile::Image);
    }
}

void ProfileDialog::setButtonColor(const QColor &c, QPushButton *b)
{
    if (!c.isValid())
        return;
    QPixmap pm(16, 16);
    pm.fill(c);
    b->setIcon(QIcon(pm));
}

void ProfileDialog::textColorButton_clicked(void)
{
    QColor c = QColorDialog::getColor(m_profile.mainColor(), this);
    setButtonColor(c, textColorButton);
    m_profile.setMainColor(c);
}

void ProfileDialog::outlineColorButton_clicked(void)
{
    QColor c = QColorDialog::getColor(m_profile.outlineColor(), this);
    setButtonColor(c, outlineColorButton);
    m_profile.setOutlineColor(c);
}

void ProfileDialog::transparency_valueChanged(int value)
{
    m_profile.setTransparency(value / 100.0);
}

void ProfileDialog::plainTextEdit_textChanged()
{
    m_profile.setText(plainTextEdit->toPlainText());
}

void ProfileDialog::font_changed()
{
    QFont f = fontComboBox->currentFont();

    f.setBold(boldToolButton->isChecked());
    f.setUnderline(underlineToolButton->isChecked());
    f.setItalic(italicToolButton->isChecked());

    f.setPointSize(sizeSpinBox->value());

    m_profile.setFont(f);
}

void ProfileDialog::currentItemChanged(QListWidgetItem * current, QListWidgetItem * previous)
{
    if (previous)
    {
        Profile old = Profile::getProfile(previous->text());
        if (old != m_profile)
        {
            // ask to save first
            if (QMessageBox::question(this,
                                      tr("Save?"),
                                      tr("Profile '%1' has been modified. Save it?").arg(previous->text()),
                                      QMessageBox::Yes, QMessageBox::No
                                     )
                    == QMessageBox::Yes)
            {
                m_profile.save();
            }
        }
    }

    loadProfile(current->text());
}

void ProfileDialog::addButton_clicked()
{
    bool ok;
    QString name = QInputDialog::getText(this,
                                         tr("New Profile Name"),
                                         tr("New Profile Name:"),
                                         QLineEdit::Normal,
                                         "",
                                         &ok);
    if (ok && !name.isEmpty())
    {
        int ix = 1;
        QString templ("%1_%2");
        QString newName = name;
        while (listWidget->findItems(newName, Qt::MatchExactly).count())
        {
            newName = templ.arg(name).arg(ix);
            ++ix;
        }

        listWidget->addItem(newName);
        listWidget->setCurrentRow(listWidget->count()-1);
        listWidget->sortItems();
    }
}

void ProfileDialog::deleteButton_clicked()
{
    m_profile.remove();
}
