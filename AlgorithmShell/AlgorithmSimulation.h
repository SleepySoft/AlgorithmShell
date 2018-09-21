#ifndef _BATTERY_SIMULATION_SLEEPY_H_
#define _BATTERY_SIMULATION_SLEEPY_H_

#include "AlgorithmShell.h"
#include <map>
#include <string>
#include <vector>


class TinyCSV
{
protected:
    bool m_hasHeader;

    bool m_ctxInQuote;
    bool m_ctxInHeader;
    uint32_t m_ctxColumnIdx;
    std::string m_ctxField;
    std::vector< std::string > m_ctxLine;

    // Header name to index
    std::map< std::string, uint32_t > m_csvHeader;
    // Vert storage
    std::vector< std::vector< std::string > > m_csvSheet;
public:
    TinyCSV(bool header = true);
    ~TinyCSV();

    void reset();
    bool parse(const std::string& text);

    uint32_t rowCount() const;
    uint32_t columnCount() const;
    uint32_t titleToIndex(std::string title) const;
    std::vector< std::string > headerTitles() const;
    std::map< std::string, uint32_t > header() const;

    std::string getCell(uint32_t row, uint32_t col);
    std::string getCell(uint32_t row, std::string col);

    std::map< std::string, uint32_t > getHeader();
    std::vector< std::vector< std::string > > getSheet();
    static bool loadFile(const char* path, TinyCSV& csv);

protected:
    void appendCell();

    //bool parseHeri(const std::string& text)
    //{
    //    auto aChar = text.begin();
    //    for (; aChar != text.end(); ++aChar)
    //    {
    //        if ((*aChar) == '"')
    //        {
    //            m_ctxInQuote = !m_ctxInQuote;
    //            continue;
    //        }
    //        if (!m_ctxInQuote)
    //        {
    //            if (((*aChar) == '\r' || (*aChar) == '\n'))
    //            {
    //                if (!m_ctxLine.empty())
    //                {
    //                    if (m_csvHeader.empty())
    //                    {
    //                        uint32_t index = 0;
    //                        auto iter = m_ctxLine.begin();
    //                        for (; iter != m_ctxLine.end(); ++iter, ++index)
    //                        {
    //                            m_csvHeader[(*iter)] = index;
    //                        }
    //                    }
    //                    else
    //                    {
    //                        m_csvSheet.push_back(m_ctxLine);
    //                    }
    //                    m_ctxLine.clear();
    //                }
    //                continue;
    //            }
    //            else if ((*aChar) == ',')
    //            {
    //                m_ctxLine.push_back(m_ctxField);
    //                m_ctxField.clear();
    //                continue;
    //            }
    //            else if ((*aChar) == ' ')
    //            {
    //                continue;
    //            }
    //        }
    //        m_ctxField += (*aChar);
    //    }
    //}
};


class ISimuDataAdapter
{
public:
    ISimuDataAdapter() { };
    virtual ~ISimuDataAdapter() { };

    virtual bool adapt(std::string key) = 0;
    virtual bool update(std::string key, std::string val) = 0;
};

class Simulator
{
protected:
    TinyCSV m_csv;
    uint32_t m_simuIndex;
    std::vector< ISimuDataAdapter* > m_dataAdapters;
    std::map< std::string, uint32_t > m_csvDataHeaders;
public:
    Simulator();
    ~Simulator();

    bool loadCSV(const char* path);
    void addDataAdapter(ISimuDataAdapter* da);

    void first();
    void next();
    bool end();
    bool seek(uint32_t index);

    void trigger();

//protected:
//    void updateSelectedIndex();

    //void markKeyParameter(std::string keyParam);
    //void markSimuParameter(std::string simuParam);
};


class Ds_Adatper_Float : public ISimuDataAdapter
{
protected:
    std::string m_dsKey;
    std::string m_simuKey;
public:
    Ds_Adatper_Float(std::string dsKey, std::string simuKey);
    virtual ~Ds_Adatper_Float();

    virtual bool adapt(std::string key);
    virtual bool update(std::string key, std::string val);
};














//void Simulation_Capacity_Volatage()
//{
//    ALGORITHM_DEPENDS(batteryCapacityPct, float);
//
//    ALGORITHM_UPDATES(measuredBatteryVoltage, float);
//
//
//}
//ALGORITHM_LINKAGE(batteryCapacityPct, Simulation_Capacity_Volatage)









//char discharge_data[][] = 
//{
//
//}








#endif // _BATTERY_SIMULATION_SLEEPY_H_
