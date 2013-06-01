#ifndef PROFILE_H
#define PROFILE_H

#include <QFont>
#include <QString>
#include <QColor>
#include <QImage>


class Profile
{
public:

    enum WatermarkType {
        Text,
        Image
    };

    Profile(const QString &name="Default");

    static QStringList getProfiles();
    static Profile getProfile(const QString &name);

    bool operator!=(const Profile &other) const;

    WatermarkType type() const { return m_type; }
    void setType(WatermarkType t) { m_type = t; }

    void save();
    void remove();
    bool isValid();

    QImage logo() const;

    QString logoPath() const { return m_watermarkImage; }
    void setLogoPath(const QString &p) { m_watermarkImage = p; }

    QString text() const;
    void setText(const QString &t) { m_watermarkText = t; }

    qreal transparency() const;
    void setTransparency(qreal t) { m_transparency = t; }

    QFont font() const;
    void setFont(const QFont &f) { m_font = f; }

    QColor mainColor() const;
    void setMainColor(const QColor &c) { m_mainColor = c; }

    QColor outlineColor() const;
    void setOutlineColor(const QColor &c) { m_outlineColor = c; }

    QSize size(int w=0, int h=0);

private:
    QString m_name;
    WatermarkType m_type;

    QString m_watermarkText;
    QString m_watermarkImage;

    qreal m_transparency;

    QFont m_font;
    QColor m_mainColor;
    QColor m_outlineColor;

    void load();

};

#endif // PROFILE_H
