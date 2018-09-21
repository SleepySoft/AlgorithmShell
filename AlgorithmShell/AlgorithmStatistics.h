#ifndef _ALGORITHM_STATISTICS_SLEEPY_H_
#define _ALGORITHM_STATISTICS_SLEEPY_H_

#include <string>
#include <vector>
#include <ostream>
#include <sstream>
#include "AlgorithmShell.h"


#define ALGSTATS (*AlgorithmStatistics::getInstance())

#define STATISTICS_VAL(key, type) ALGSTATS.addStatisticWrapper(new AlgorithmStatistics::wrapper< type >(TOSTR(key)))
#define STATISTICS_TRIGGER ALGSTATS.triggerStatistic();
#define STATISTICS_TEXTSTR ALGSTATS.getStatisticText()
#define STATISTICS_TEXTRAW ALGSTATS.getStatisticText().c_str()

#define STATISTICS_DUMP2FILE(path) {\
    std::string text(ALGSTATS.getStatisticText());\
    FILE* file = fopen(path, "wt");\
    if (file != NULL) { fwrite(text.c_str(), 1, text.size(), file); fclose(file); } }


class AlgorithmStatistics
{
public:
    class wrapperif
    {
    public:
        wrapperif() { }
        virtual ~wrapperif() { }

        virtual std::string key() const = 0;
        virtual void toOStream(std::ostream& os) const = 0;
    };

    template< class T >
    class wrapper : public wrapperif
    {
    protected:
        std::string m_key;
    public:
        wrapper(const char* key) : m_key(key) { }
        virtual ~wrapper() { }

        virtual std::string key() const
        {
            return m_key;
        }
        virtual void toOStream(std::ostream& os) const
        {
            dw::any any = ALGSHL.getDataset().get(m_key);
            os << any.value_as< T >();
        }
    };
protected:
    bool m_first;
    std::ostream* m_ostream;
    std::stringstream m_strstream;
    std::vector< wrapperif* > m_statisticList;
protected:
    AlgorithmStatistics();
    ~AlgorithmStatistics();
public:
    static AlgorithmStatistics* m_instance;
    static AlgorithmStatistics* getInstance();
    static void releaseInstance();

public:
    void addStatisticWrapper(wrapperif* sw);

    void setOutputStream(std::ostream* os);
    std::string getStatisticText() const;
    void restoreStatistic();

    void triggerStatistic();
protected:
    void doWriteHeader(std::ostream* os);
    void doWriteContent(std::ostream* os);
};

#endif // _ALGORITHM_STATISTICS_SLEEPY_H_
