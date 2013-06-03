#include "QDebug"
#include <QProgressDialog>
#include <QDirIterator>
#include <QImageReader>
#include <QCompleter>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QSettings>

#include "qwatermark.h"
#include "profile.h"
#include "profiledialog.h"


QWatermark::QWatermark(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi(this);

    QCompleter *completer = new QCompleter(this);
    FSModel *fsModel = new FSModel(completer);
    fsModel->setRootPath("");
    completer->setModel(fsModel);

    profileComboBox->addItems(Profile::getProfiles());

    QSettings s;
    s.beginGroup("MainWindow");
    restoreGeometry(s.value("geometry").toByteArray());
    restoreState(s.value("windowState").toByteArray());
    sourceLineEdit->setText(s.value("sourcePath").toString());
    destinationLineEdit->setText(s.value("destinationPath").toString());
    previewZoomSpinBox->setValue(s.value("zoom", 30).toInt());
    treeCheckBox->setChecked(s.value("treeIteration", false).toBool());

    int ix = profileComboBox->findText(s.value("profile", tr("Default")).toString());
    if (ix > -1)
        profileComboBox->setCurrentIndex(ix);

    QAbstractButton *b = buttonGroup->button(s.value("buttonGroup", 0).toInt());
    if (!b)
        b = ULRadioButton;
    b->setChecked(true);

    s.endGroup();

    sourceLineEdit->setCompleter(completer);
    destinationLineEdit->setCompleter(completer);

    connect(sourcePushButton,SIGNAL(clicked()),this,SLOT(selectSourceFolder(void)));
    connect(destinationPushButton,SIGNAL(clicked()),this,SLOT(selectDestinationFolder(void)));
    connect(startButton,SIGNAL(clicked()),this,SLOT(doWatermark(void)));
    connect(actionAbout, SIGNAL(triggered()), this, SLOT(about(void)));
    connect(actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    connect(sourceLineEdit, SIGNAL(textEdited(QString)), this, SLOT(checkConditions()));
    connect(destinationLineEdit, SIGNAL(textEdited(QString)), this, SLOT(checkConditions()));

    connect(buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(preview()));
    connect(profileComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(preview()));
    connect(previewZoomSpinBox, SIGNAL(valueChanged(int)), this, SLOT(preview()));

    connect(editProfileButton, SIGNAL(clicked()), this, SLOT(editProfileButton_clicked()));

    checkConditions();

    preview();
}

void QWatermark::closeEvent(QCloseEvent *event)
{
    QSettings s;
    s.beginGroup("MainWindow");
    s.setValue("geometry", saveGeometry());
    s.setValue("windowState", saveState());
    s.setValue("sourcePath", sourceLineEdit->text());
    s.setValue("destinationPath", destinationLineEdit->text());
    s.setValue("buttonGroup", buttonGroup->checkedId());
    s.setValue("zoom", previewZoomSpinBox->value());
    s.setValue("profile", profileComboBox->currentText());
    s.setValue("treeIteration", treeCheckBox->isChecked());
    s.endGroup();
    QWidget::closeEvent(event);
}

//Select source folder for images
void QWatermark::selectSourceFolder(void)
{
	//Create a dialog to select a folder
	QString sourceDirectoryName = QFileDialog::getExistingDirectory(this,tr("Open Source Directory"),
			"",QFileDialog::ShowDirsOnly);
    if (sourceDirectoryName.isNull())
        return;

	//Assign of the selected folder to the line edit
    sourceLineEdit->setText(sourceDirectoryName);
    checkConditions();
}

//Select destination folder for images (similar to source)
void QWatermark::selectDestinationFolder(void)
{
	QString destinationDirectoryName = QFileDialog::getExistingDirectory(this,tr("Open Destination Directory"),
			"",QFileDialog::ShowDirsOnly);

    if (destinationDirectoryName.isNull())
        return;

    destinationLineEdit->setText(destinationDirectoryName);
    checkConditions();
}

//Execute the watermark
void QWatermark::doWatermark(void)
{
	QImage resultImage;
	QPainter painter;

    Profile profile = Profile::getProfile(profileComboBox->currentText());
    if (!profile.isValid())
    {
        qDebug() << "TODO/FIXME: invalid profile msg?";
        return;
    }

    QStringList filesToProcess;
    QDir::Filters filters = QDir::NoDotAndDotDot | QDir::Readable | QDir::Files | QDir::AllDirs;
    QDirIterator::IteratorFlags flags = treeCheckBox->isChecked()
                                            ? QDirIterator::Subdirectories | QDirIterator::FollowSymlinks
                                            : QDirIterator::NoIteratorFlags;
    QDirIterator it(sourceLineEdit->text(), filters, flags);
    while (it.hasNext())
    {
        it.next();
        if (!QImageReader::imageFormat(it.filePath()).isNull())
            filesToProcess << it.filePath();
        else
            qDebug() << "Ignored" << it.filePath();
    }

    QProgressDialog progress("Applying watermarks...", "Abort", 0, filesToProcess.size(), this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();

    int errCnt = 0;

	//Iterate over images
    for(int i = 0; i < filesToProcess.size(); ++i){

        qDebug() << "FILE" << filesToProcess.at(i);

        if (progress.wasCanceled())
        {
            qDebug() << "TODO/FIXME: cleanup already created files";
            break;
        }
        progress.setValue(i+1);
        progress.setLabelText(filesToProcess.at(i));

		//Load image
        if (!resultImage.load(filesToProcess.at(i)))
        {
            qDebug() << "Cannot load" << filesToProcess.at(i) << "skipping";
            continue;
        }
        if (!painter.begin(&resultImage))
        {
            qDebug() << "TODO/FIXME: painter.begin check" << filesToProcess.at(i);
            continue;
        }

        //painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        paintOne(resultImage.width(), resultImage.height(), &painter, &profile);
        painter.end();

        //save the image
        QString tgtPath = getTargetPath(filesToProcess.at(i));
        qDebug() << "SAVE" << tgtPath;
        QDir dir = QFileInfo(tgtPath).absoluteDir();
        if (!dir.exists(dir.path()))
            dir.mkpath(dir.path());

        if(!resultImage.save(tgtPath, 0, 100))
        {
            errCnt++;
            if (QMessageBox::question(this, tr("Error"),
                                  tr("An error occurred while saving the image '%1'. Continue?").arg(tgtPath),
                                  QMessageBox::Yes, QMessageBox::No)
                    == QMessageBox::No)
            {
                qDebug() << "User canceled processing after an error";
                break;
            }
        }

	}

    if (errCnt == 0)
        QMessageBox::information(this, tr("Success"), tr("Processing Completed."));
    qDebug() << "TODO/FIXME: Clear input/target lineedits?";
}

void QWatermark::paintOne(int w, int h, QPainter *painter, Profile *profile)
{
    int imageX = 0;
    int imageY = 0;

    painter->setOpacity(profile->transparency());

    QSize size = profile->size(w, h);

    //controls which logo position is selected
    if (ULRadioButton->isChecked())
    {
        //Do nothing, is default already set
    }
    else if (UCRadioButton->isChecked())
    {
        imageX = w/2 - size.width()/2;
        imageY = 0 + profile->marginVertical();
    }
    else if (URRadioButton->isChecked())
    {
        imageX = w - size.width() - profile->marginHorizontal();
        imageY = 0 + profile->marginVertical();
    }
    else if (CLRadioButton->isChecked())
    {
        imageX = 0 + profile->marginHorizontal();
        imageY = h/2 - size.height()/2;
    }
    else if (CCRadioButton->isChecked())
    {
        imageX = w/2 - size.width()/2;
        imageY = h/2 - size.height()/2;
    }
    else if (CRRadioButton->isChecked())
    {
        imageX = w - size.width() - profile->marginHorizontal();
        imageY = h/2 - size.height()/2;
    }
    else if (LLRadioButton->isChecked())
    {
        imageX = 0 + profile->marginHorizontal();
        imageY = h - size.height() - profile->marginVertical();
    }
    else if (LCRadioButton->isChecked())
    {
        imageX = w/2 - size.width()/2;
        imageY = h - size.height() - profile->marginVertical();
    }
    else if (LRRadioButton->isChecked())
    {
        imageX = w - size.width() - profile->marginHorizontal();
        imageY = h - size.height() - profile->marginVertical();
    }

    switch (profile->type())
    {
    case Profile::Image:
        painter->drawImage(imageX, imageY, profile->logo(), 0, 0, -1, -1);
        break;
    case Profile::Text:
    {
        painter->setBrush(profile->mainColor());
        QPen pen(profile->outlineColor());
        pen.setWidth(2);
        painter->setPen(pen);

        QPainterPath path;
        path.addText(imageX, imageY+size.height(), profile->font(), profile->text());
        painter->drawPath(path);

        break;
    }
    }

}

void QWatermark::preview()
{
    Profile profile = Profile::getProfile(profileComboBox->currentText());
    if (!profile.isValid())
    {
        qDebug() << "TODO/FIXME: invalid profile msg?";
        return;
    }

    QImage img(":/preview.jpg");
    QPainter p(&img);
    paintOne(img.width(), img.height(), &p, &profile);
    p.end();

    int zoom = previewZoomSpinBox->value();
    previewLabel->setPixmap(QPixmap::fromImage(img).scaledToHeight(img.height()/100.0*zoom));
}

QString QWatermark::getTargetPath(const QString &fname)
{
    QRegExp re("^" + sourceLineEdit->text());
    QString ret = fname;
    ret.replace(re, destinationLineEdit->text());
    return ret;
}

void QWatermark::about(void)
{
    QMessageBox::about(this, tr("About QWatermark"),
                       tr("<b>%1 %2</b>, a simple program to "
                          "impress text or logo over images.").arg(QApplication::applicationName()).arg(QApplication::applicationVersion()));
}


bool QWatermark::checkDir(const QString& name)
{
    QFileInfo fi(name);
    if (!fi.isDir())
        return false;

    return true;
}

void QWatermark::checkConditions()
{
    bool enable = true;

    enable &= checkDir(sourceLineEdit->text());
    enable &= checkDir(destinationLineEdit->text());

    startButton->setEnabled(enable);
}

void QWatermark::editProfileButton_clicked()
{
    ProfileDialog pd(profileComboBox->currentText());
    pd.exec();
    preview();
}


FSModel::FSModel(QObject *parent)
    : QFileSystemModel(parent)
{
}

QVariant FSModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole && index.column() == 0)
    {
        QString path  = QDir::toNativeSeparators(filePath(index));
        if (path.endsWith(QDir::separator()))
            path.chop(1);
        return path;
    }

    return QFileSystemModel::data(index, role);
}
