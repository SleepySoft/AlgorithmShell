#include "AlgorithmStatistics.h"


AlgorithmStatistics::AlgorithmStatistics()
    : m_first(true), m_ostream(NULL)
{
}


AlgorithmStatistics::~AlgorithmStatistics()
{
}


AlgorithmStatistics* AlgorithmStatistics::m_instance = NULL;

AlgorithmStatistics* AlgorithmStatistics::getInstance()
{
    if (m_instance == NULL)
    {
        m_instance = new AlgorithmStatistics();
    }
    return m_instance;
}

void AlgorithmStatistics::releaseInstance()
{
    delete m_instance;
    m_instance = NULL;
}



void AlgorithmStatistics::addStatisticWrapper(wrapperif* sw)
{
    m_statisticList.push_back(sw);
}

void AlgorithmStatistics::setOutputStream(std::ostream* os)
{
    m_ostream = os;
}
std::string AlgorithmStatistics::getStatisticText() const
{
    return m_strstream.str();
}
void AlgorithmStatistics::restoreStatistic()
{
    m_strstream.str(std::string(""));
    m_strstream.clear();
    if (m_ostream != NULL)
    {
        m_ostream->clear();
    }
}

void AlgorithmStatistics::triggerStatistic()
{
    if (m_first)
    {
        doWriteHeader(m_ostream);
        doWriteHeader(&m_strstream);
        m_first = false;
    }
    doWriteContent(m_ostream);
    doWriteContent(&m_strstream);
}

void AlgorithmStatistics::doWriteHeader(std::ostream* os)
{
    if (os != NULL)
    {
        auto iter = m_statisticList.begin();
        for (; iter != m_statisticList.end(); ++iter)
        {
            if (iter != m_statisticList.begin())
            {
                (*os) << ", ";
            }
            (*os) << (*iter)->key();
        }
        (*os) << std::endl;
    }
}
void AlgorithmStatistics::doWriteContent(std::ostream* os)
{
    if (os != NULL)
    {
        auto iter = m_statisticList.begin();
        for (; iter != m_statisticList.end(); ++iter)
        {
            if (iter != m_statisticList.begin())
            {
                (*os) << ", ";
            }
            (*iter)->toOStream(*os);
        }
        (*os) << std::endl;
    }
}
