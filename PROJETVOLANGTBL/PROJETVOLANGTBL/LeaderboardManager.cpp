#include "LeaderboardManager.h"
#include <QFile>
#include <QTextStream>
#include <algorithm>

QString LeaderboardManager::filePath(int trackIndex)
{
    return QString("tracks/leaderboard_track%1.txt").arg(trackIndex + 1);
}

QVector<LeaderboardManager::Entry> LeaderboardManager::load(int trackIndex)
{
    QVector<Entry> entries;
    QFile file(filePath(trackIndex));

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return entries;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList parts = line.split(',');
        if (parts.size() < 2) continue;

        Entry e;
        e.name = parts[0].trimmed();
        e.timeMs = parts[1].trimmed().toInt();
        entries.append(e);
    }

    // Trie par temps croissant
    std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) {
        return a.timeMs < b.timeMs;
        });

    return entries;
}

void LeaderboardManager::save(int trackIndex, const QVector<Entry>& entries)
{
    QFile file(filePath(trackIndex));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    for (const Entry& e : entries) {
        out << e.name << "," << e.timeMs << "\n";
    }
}

void LeaderboardManager::addEntry(int trackIndex, const QString& name, int timeMs)
{
    QVector<Entry> entries = load(trackIndex);
    Entry e;
    e.name = name;
    e.timeMs = timeMs;
    entries.append(e);

    std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) {
        return a.timeMs < b.timeMs;
        });

    save(trackIndex, entries);
}