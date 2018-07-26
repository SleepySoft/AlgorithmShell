#ifndef _ALGORITHM_SHELL_SLEEPY_H_
#define _ALGORITHM_SHELL_SLEEPY_H_

#include <map>
#include <tuple>
#include <mutex>
#include <thread>
#include <vector>
#include <string>
#include <assert.h>
#include <iostream>
#include "Dataset.h"


#ifndef MASSERT
#   define MASSERT(Expr, Msg) assert((Msg, Expr))
#endif // MASSERT

#ifndef TOSTR
#   define TOSTR(s) #s
#endif // TOSTR


// Quick access of AlgorithmShell instance
#define ALGSHL (*AlgorithmShell::getInstance())

//#define ALGORITHM_INVOKE(algorithm, ...) unpacker(); 
//
//#define ALGORITHM_INPUT(key, type) type key = type(); key = anyloader< type >(ds, TOSTR(key), "Input value '" TOSTR(key) "' missing in algorithm: " __FUNCTION__);
//#define ALGORITHM_OUTPUT(key, type) anywriter< type > __aw_##key(ds, TOSTR(key), key, __FUNCTION__);
//#define ALGORITHM_DECLARE(algorithm) template< class T > void algorithm(Dataset* ds)

// If the algorithm has parameters, please use std::bind to provide a NO-PARAM function.
// MUST placed after function declaration.
#define ALGORITHM_LINKAGE(key, func) static alglinker __##key##_link_##func = alglinker(TOSTR(key), func, TOSTR(func));

// Depends on this data. Read only. Data will be prepared when this function enteres.
#define ALGORITHM_DEPENDS(key, type) type key = type(); key = anyloader< type >(ALGSHL.getDataset(), TOSTR(key), "Depends value '" TOSTR(key) "' missing in algorithm: " __FUNCTION__); ALGSHL.registerDepends(__FUNCTION__, TOSTR(key));

// Updates this data. Write only. Data will be updated when this function quit.
#define ALGORITHM_UPDATES(key, type) type key = type(); anywriter< type > __aw_##key(ALGSHL.getDataset(), TOSTR(key), key, __FUNCTION__); ALGSHL.registerUpdates(__FUNCTION__, TOSTR(key));

// Manages this data. Read and Write Data will be prepared when this function enteres and updated when this function quits. 
#define ALGORITHM_MANAGES(key, type) ALGORITHM_DEPENDS(key, type); anywriter< type > __aw_##key(ALGSHL.getDataset(), TOSTR(key), key, __FUNCTION__); ALGSHL.registerManages(__FUNCTION__, TOSTR(key));


typedef void(*LINKAGEFUNC)(void);
typedef std::vector< LINKAGEFUNC > LINKAGELIST;
typedef std::map< std::string, LINKAGELIST > LINKAGEDICT;


class AlgorithmShell : public IDataObserver
{
protected:
    Dataset m_dataset;
    bool m_linkageEnable;
    LINKAGEDICT m_algLinkageDict;

protected:
    AlgorithmShell();
    ~AlgorithmShell();
public:
    static AlgorithmShell* m_instance;
    static AlgorithmShell* getInstance();
    static void releaseInstance();

public:
    Dataset& getDataset();
    void enableLinkage(bool enable);
    void linkAlgorithm(LINKAGEFUNC func, std::string key);

    // For algorithm analysis
    void registerDepends(std::string alg, std::string key) { };
    void registerUpdates(std::string alg, std::string key) { };
    void registerManages(std::string alg, std::string key) { };
    void registerLinkage(std::string alg, std::string key) { };

public:
    virtual void onFirstData(std::string key, const dw::any& value);
    virtual void onDataChanged(std::string key, const dw::any& value);
    virtual void onBatchDataChanged(const DATALIST& values);

protected:
    LINKAGELIST getLinkedFunction(std::string key);
    void triggerLinkages(const LINKAGELIST& linkages);
};



template< typename T >
T anyloader(Dataset& ds, std::string key, const char* errmsg)
{
    dw::any any;
    bool ret = ds.get(key, any);
    MASSERT(ret, errmsg);
    return any.value_as< T >();
}

template< typename T >
class anywriter
{
protected:
    T& m_any;
    Dataset& m_ds;
    std::string m_key;
    const char* m_algorithm;
public:
    anywriter(Dataset& ds, std::string key, T& any, const char* algorithm)
        : m_any(any), m_ds(ds), m_key(key), m_algorithm(algorithm)
    {

    }
    ~anywriter()
    {
        m_ds.set(m_key, dw::any(m_any));
    }
};

class alglinker
{
public:
    alglinker(std::string key, LINKAGEFUNC func, const char* algorithm)
    {
        ALGSHL.linkAlgorithm(func, key);
        ALGSHL.registerLinkage(algorithm, key);
    }
    ~alglinker()
    {

    }
};


// TODO: We need a data cache for nest invoking. Not an autowriter, which is a bad design.


// ------------------- Auto Writer -------------------

//class writerbase
//{
//public:
//    writerbase() { };
//    virtual ~writerbase() { };
//
//    virtual void write() = 0;
//};

//template< typename T >
//class autowriter : public writerbase
//{
//protected:
//    T& m_any;
//    Dataset& m_ds;
//    std::string m_key;
//    const char* m_algorithm;
//public:
//    anywriter(Dataset& ds, std::string key, T& any, const char* algorithm)
//        : m_any(any), m_ds(ds), m_key(key), m_algorithm(algorithm)
//    {
//
//    }
//    virtual ~anywriter()
//    {
//        write();
//    }
//
//    virtual void write()
//    {
//        m_ds.set(m_key, dw::any(m_any));
//    }
//
//protected:
//    typedef std::tuple< const char*, writerbase* > datactx;
//    typedef std::list< datactx > callctx;
//    typedef std::map< thread::id, callctx > callmap;
//
//    static std::mutex m_lock = std::mutex();
//    static callmap m_callmap = callmap();
//
//    static void enter(writerbase* writer);
//    static void leave(writerbase* writer);
//
//    static void lock() { m_lock.lock(); };
//    static void unlock() { m_lock.unlock(); };
//
//    static bool checkflush();
//    static void flushupdates();
//};

#endif // _ALGORITHM_SHELL_SLEEPY_H_
