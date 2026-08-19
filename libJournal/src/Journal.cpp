#include "Journal.h"

QHash<quint64, Journal*> Journal::m_instances;
QMutex Journal::m_instanceMutex;
JournalLevels Journal::m_journalLevel;

Journal::Journal(const quint64 index)
    : m_index(index)
{
    ///
}

//----------------------------------------------------
void Journal::addStorage( JournalStorage *storage )
{
    m_storages.push_back(storage);
}

//----------------------------------------------------
Journal::~Journal()
{
    QMutexLocker instanceMutexLocker(&m_instanceMutex);
    qDeleteAll(m_storages);
    delete m_instances[m_index];
    m_instances.remove(m_index);
}

//----------------------------------------------------
void Journal::write( JournalLevel::Level level, const QString& record )
{
    for(auto &iter : m_storages)
        iter->write( QDateTime::currentDateTime(), level, record );
}

//----------------------------------------------------
Journal* Journal::instance(const quint64 index)
{
    if(m_instances.contains(index))
        return m_instances[index];
    else
    {
        QMutexLocker instanceMutexLocker(&m_instanceMutex);
        Journal *journal = new Journal(index);
        m_instances[index] = journal;
        return journal;
    }
}

JournalLevels Journal::journalLevels()
{
    return m_journalLevel;
}

void Journal::setJournalLevel(const JournalLevels journalLevel)
{
    m_journalLevel = journalLevel;
}

Journal*Journal::instance()
{
    return instance(0);
}

