#include "AlgorithmSimulation.h"



TinyCSV::TinyCSV(bool header) : m_hasHeader(header)
{
    m_ctxInQuote = false;
    m_ctxInHeader = m_hasHeader;
    m_ctxColumnIdx = 0;
}
TinyCSV::~TinyCSV()
{

}

void TinyCSV::reset()
{
    m_ctxInQuote = false;
    m_ctxInHeader = m_hasHeader;
    m_ctxColumnIdx = 0;

    m_ctxField.clear();
    m_ctxLine.clear();
    m_csvHeader.clear();
    m_csvSheet.clear();
}

bool TinyCSV::parse(const std::string& text)
{
    auto aChar = text.begin();
    for (; aChar != text.end(); ++aChar)
    {
        if ((*aChar) == '"')
        {
            m_ctxInQuote = !m_ctxInQuote;
            continue;
        }
        if (!m_ctxInQuote)
        {
            if (((*aChar) == '\r' || (*aChar) == '\n'))
            {
                if (m_ctxColumnIdx != 0)
                {
                    appendCell();
                    // It means it's the first to entery the new line.
                    for (uint32_t i = m_ctxColumnIdx; i < m_csvSheet.size(); i++)
                    {
                        // Fill the missing fields (optional).
                        m_csvSheet.at(i).push_back(std::string());
                    }
                }
                m_ctxInHeader = false;
                m_ctxColumnIdx = 0;
                continue;
            }
            else if ((*aChar) == ',')
            {
                appendCell();
                continue;
            }
            else if ((*aChar) == ' ')
            {
                continue;
            }
        }
        m_ctxField += (*aChar);
    }
    return true;
}

uint32_t TinyCSV::rowCount() const
{
    return m_csvSheet.empty() ? 0 : m_csvSheet.at(0).size();
}
uint32_t TinyCSV::columnCount() const
{
    return m_csvSheet.size();
}
uint32_t TinyCSV::titleToIndex(std::string title) const
{
    return m_csvHeader.find(title) != m_csvHeader.end() ? m_csvHeader.at(title) : -1;
}
std::vector< std::string > TinyCSV::headerTitles() const
{
    std::vector< std::string > titles;
    auto iter = m_csvHeader.begin();
    for ( ; iter != m_csvHeader.end(); ++iter)
    {
        titles.push_back((*iter).first);
    }
    return titles;
}
std::map< std::string, uint32_t > TinyCSV::header() const
{
    return m_csvHeader;
}

std::string TinyCSV::getCell(uint32_t row, uint32_t col)
{
    if ((col < m_csvSheet.size()) &&
        (row < m_csvSheet.at(col).size()))
    {
        return m_csvSheet[col][row];
    }
    return std::string();
}
std::string TinyCSV::getCell(uint32_t row, std::string col)
{
    auto iter = m_csvHeader.find(col);
    if (iter != m_csvHeader.end())
    {
        return getCell(row, (*iter).second);
    }
    return std::string();
}

std::map< std::string, uint32_t > TinyCSV::getHeader()
{
    return m_csvHeader;
}
std::vector< std::vector< std::string > > TinyCSV::getSheet()
{
    return m_csvSheet;
}

bool TinyCSV::loadFile(const char* path, TinyCSV& csv)
{
    bool ret = false;
    FILE* file = fopen(path, "rb");
    if (file != NULL)
    {
        char buffer[1024];
        while (1)
        {
            memset(buffer, 0, sizeof(buffer));
            size_t rlen = fread(buffer, 1, sizeof(buffer) - 1, file);
            if ((rlen > 0) && (rlen < sizeof(buffer)))
            {
                ret = csv.parse(std::string(buffer));
            }
            else
            {
                break;
            }
        }
        fclose(file);
        file = NULL;
    }
    return ret;
}

void TinyCSV::appendCell()
{
    while (m_ctxColumnIdx >= m_csvSheet.size())
    {
        m_csvSheet.push_back(std::vector< std::string >());
    }
    if (m_ctxInHeader)
    {
        m_csvHeader[m_ctxField] = m_ctxColumnIdx;
    }
    else
    {
        m_csvSheet.at(m_ctxColumnIdx).push_back(m_ctxField);
    }
    ++m_ctxColumnIdx;
    m_ctxField.clear();
}






Simulator::Simulator()
{
    m_simuIndex = 0;
}
Simulator::~Simulator()
{

}

bool Simulator::loadCSV(const char* path)
{
    bool ret = TinyCSV::loadFile(path, m_csv);
    if (ret)
    {
        m_csvDataHeaders = m_csv.header();
    }
    return ret;
}
void Simulator::addDataAdapter(ISimuDataAdapter* da)
{
    m_dataAdapters.push_back(da);
}

void Simulator::first()
{
    m_simuIndex = 0;
}
void Simulator::next()
{
    ++m_simuIndex;
}
bool Simulator::end()
{
    return m_simuIndex >= m_csv.rowCount();
}
bool Simulator::seek(uint32_t index)
{
    bool ret = false;
    if (index < m_csv.rowCount())
    {
        m_simuIndex = index;
        ret = true;
    }
    return ret;
}

void Simulator::trigger()
{
    if (!end())
    {
        auto titleIter = m_csvDataHeaders.begin();
        for (; titleIter != m_csvDataHeaders.end(); ++titleIter)
        {
            auto adapterIter = m_dataAdapters.begin();
            for (; adapterIter != m_dataAdapters.end(); ++adapterIter)
            {
                if ((*adapterIter)->adapt((*titleIter).first))
                {
                    std::string val = m_csv.getCell(m_simuIndex, (*titleIter).second);
                    (*adapterIter)->update((*titleIter).first, val);
                }
            }
        }
    }
}



Ds_Adatper_Float::Ds_Adatper_Float(std::string dsKey, std::string simuKey)
    : m_dsKey(dsKey), m_simuKey(simuKey)
{

}
Ds_Adatper_Float::~Ds_Adatper_Float()
{

}

bool Ds_Adatper_Float::adapt(std::string key)
{
    return (key == m_simuKey);
}
bool Ds_Adatper_Float::update(std::string key, std::string val)
{
    bool ret = false;
    if (key == m_simuKey)
    {
        float fval = (float)atof(val.c_str());
        ALGSHL.getDataset().set(m_dsKey, dw::any(fval));
        ret = true;
    }
    return ret;
}



//void Simulator::updateSelectedIndex()
//{
//    std::vector< std::string > selectedTitles;
//    std::vector< std::string > titles = m_csv.headerTitles();
//
//    auto titleIter = titles.begin();
//    for ( ; titleIter != titles.end(); ++titleIter)
//    {
//        auto adapterIter = m_dataAdapters.begin();
//        for ( ; adapterIter != m_dataAdapters.end(); ++adapterIter)
//        {
//            if ((*adapterIter)->adapt(*titleIter))
//            {
//                selectedTitles.push_back(*titleIter);
//                break;
//            }
//        }
//    }
//
//    m_selIndex.clear();
//    auto selIter = selectedTitles.begin();
//    for (; selIter != selectedTitles.end(); ++selIter)
//    {
//        m_selIndex.push_back(m_csv.titleToIndex(*selIter));
//    }
//}



//void Simulator::setKeyParameter(std::string keyParam)
//{
//    m_keyParam = keyParam;
//}
//void Simulator::addSimuParameter(std::string simuParam)
//{
//    m_simuParam.push_back(simuParam);
//}
//
//void Simulator::next()
//{
//    ++m_simuIndex;
//}
//
//std::string Simulator::get(std::string key)
//{
//    return m_csv.getCell(m_simuIndex, key);
//}
