#include <QtDebug>
#include <QSettings>
#include <QFileInfo>
#include <QFontMetrics>
#include <QApplication>

#include "profile.h"


QStringList Profile::getProfiles()
{
    QSettings s;
    QStringList l;

    foreach(QString i, s.childGroups())
    {
        s.beginGroup(i);
        if (s.value("is_profile", false).toBool())
            l << i;
        s.endGroup();
    }

    if (!l.contains(QObject::tr("Default")))
        l << QObject::tr("Default");

    return l;
}

Profile Profile::getProfile(const QString &name)
{
    return Profile(name);
}

Profile::Profile(const QString &name)
    : m_name(name)
{
    load();
}

bool Profile::isValid()
{
    return (m_type == Profile::Text && !m_watermarkText.isEmpty())
            || (m_type == Profile::Image && QFileInfo(m_watermarkImage).exists());
}

void Profile::load()
{
    QSettings s;
    s.beginGroup(m_name);

    m_type = s.value("type", "text").toString() == "image" ? Profile::Image : Profile::Text;

    m_watermarkText = s.value("text", QApplication::applicationName() + " " + QApplication::applicationVersion()).toString();
    m_watermarkImage = s.value("image").toString();

    m_marginHorizontal = s.value("marginHorizontal", 10).toInt();
    m_marginVertical = s.value("marginVertical", 10).toInt();

    m_transparency = s.value("transparency", 0.5).toReal();

    QString fontStr = s.value("font", "n/a").toString();
    if (fontStr == "n/a")
    {
        m_font = QFont();
        m_font.setPointSize(48);
    }
    else
        m_font.fromString(fontStr);
    qDebug() << "font!" << m_font << m_font.bold();

    m_mainColor = QColor(s.value("mainColor", "#ffffff").toString());
    m_outlineColor = QColor(s.value("outlineColor", "#000000").toString());

    s.endGroup();
}

void Profile::save()
{
    QSettings s;
    s.beginGroup(m_name);

    s.setValue("type", m_type == Profile::Image ? "image" : "text");
    s.setValue("is_profile", true);

    s.setValue("text", m_watermarkText);
    s.setValue("image", m_watermarkImage);

    s.setValue("marginHorizontal", m_marginHorizontal);
    s.setValue("marginVertical", m_marginVertical);

    s.setValue("transparency", m_transparency);

    s.setValue("font", m_font.toString());

    s.setValue("mainColor", m_mainColor.name());
    s.setValue("outlineColor", m_outlineColor.name());

    s.endGroup();
}

void Profile::remove()
{
    QSettings s;
    s.remove(m_name);
}

QImage Profile::logo() const
{
    qDebug() << "TODO/FIXME: check presence of path";
    return QImage(m_watermarkImage);
}

QString Profile::text() const
{
    return m_watermarkText;
}

qreal Profile::transparency() const
{
    return m_transparency;
}

QFont Profile::font() const
{
    return m_font;
}

QColor Profile::mainColor() const
{
    return m_mainColor;
}

QColor Profile::outlineColor() const
{
    return m_outlineColor;
}

QSize Profile::size(int w, int h)
{
    switch (m_type)
    {
    case Profile::Image:
        return logo().size();
    case Profile::Text:
        {
        QFontMetrics fm(m_font);
        return fm.boundingRect(0, 0, w, h, Qt::AlignLeft|Qt::AlignTop, m_watermarkText).size();
        }
    }

    return QSize();
}

bool Profile::operator!=(const Profile &other) const
{
    qDebug() << "\n\nOPER" << (this->type() != other.type()) << (this->text() != other.text()) <<
            (this->logoPath() != other.logoPath()) <<
            (this->font() != other.font()) <<
            (this->mainColor() != other.mainColor()) <<
            (this->outlineColor() != other.outlineColor()) <<
            (!qFuzzyCompare(this->transparency(), other.transparency()));
    qDebug() << "   text:" << this->text() << other.text();
    qDebug() << "   font:" << this->font() << other.font() << this->font().toString() << other.font().toString();
    return     this->type() != other.type()
            || this->text() != other.text()
            || this->marginHorizontal() != other.marginHorizontal()
            || this->marginVertical() != other.marginVertical()
            || this->logoPath() != other.logoPath()
            || this->font() != other.font()
            || this->mainColor() != other.mainColor()
            || this->outlineColor() != other.outlineColor()
            || !qFuzzyCompare(this->transparency(), other.transparency());
}
