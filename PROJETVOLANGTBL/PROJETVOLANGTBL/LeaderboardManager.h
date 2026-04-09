#pragma once
#include <QString>
#include <QVector>
#include <QPair>

class LeaderboardManager
{
public:
    struct Entry {
        QString name;
        int timeMs;
        QString formattedTime() const {
            int mins = timeMs / 60000;
            int secs = (timeMs % 60000) / 1000;
            int ms = timeMs % 1000;
            return QString("%1:%2.%3")
                .arg(mins)
                .arg(secs, 2, 10, QChar('0'))
                .arg(ms / 10, 2, 10, QChar('0'));
        }
    };

    static QVector<Entry> load(int trackIndex);
    static void save(int trackIndex, const QVector<Entry>& entries);
    static void addEntry(int trackIndex, const QString& name, int timeMs);

private:
    static QString filePath(int trackIndex);
};
