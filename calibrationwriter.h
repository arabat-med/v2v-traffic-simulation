#ifndef CALIBRATIONWRITER_H
#define CALIBRATIONWRITER_H

#include <QObject>
#include <QUrl>
#include <QVariantList>

class CalibrationWriter : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
public:
    explicit CalibrationWriter(QObject* parent = nullptr) : QObject(parent) {}
    QString error() const { return m_error; }
    Q_INVOKABLE bool save(const QUrl& destination, int width, int height, const QVariantList& points);
signals:
    void errorChanged();
private:
    QString m_error;
};

#endif
