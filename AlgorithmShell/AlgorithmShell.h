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

#define ALGORITHM_GET(key, type) type key(ALGSHL.getDataset().get(TOSTR(key)).value_as< type >());
#define ALGORITHM_SET(key, value) ALGSHL.getDataset().set(TOSTR(key), dw::any(value));
#define ALGORITHM_TSET(key, type, value) ALGSHL.getDataset().set(TOSTR(key), dw::any((type)(value)));

// Use in a function to make the update do not trigger the linkage.
#define ALGORITHM_SILENCE algsilence __algsilence();

// If the algorithm has parameters, please use std::bind to provide a NO-PARAM function.
// MUST placed after function declaration.
#define ALGORITHM_LINKAGE(key, func) static alglinker __##key##_link_##func = alglinker(TOSTR(key), func, TOSTR(func));

// Depends on this data. Read only. Data will be prepared when this function enteres.
#define ALGORITHM_DEPENDS(key, type) type key = type(); anysync< type > __as_##key(ALGSHL.getDataset(), TOSTR(key), key, true, false, __FUNCTION__); ALGSHL.registerDepends(__FUNCTION__, TOSTR(key));

// Updates this data. Write only. Data will be updated when this function quit.
#define ALGORITHM_UPDATES(key, type) type key = type(); anysync< type > __as_##key(ALGSHL.getDataset(), TOSTR(key), key, false, true, __FUNCTION__); ALGSHL.registerUpdates(__FUNCTION__, TOSTR(key));

// Manages this data. Read and Write Data will be prepared when this function enteres and updated when this function quits. 
#define ALGORITHM_MANAGES(key, type) type key = type(); anysync< type > __as_##key(ALGSHL.getDataset(), TOSTR(key), key, true, true, __FUNCTION__); ALGSHL.registerManages(__FUNCTION__, TOSTR(key));

// Discard updates and manages
#define ALGORITHM_DISCARD(key) __as_##key.discard()

typedef void(*LINKAGEFUNC)(void);
typedef std::vector< LINKAGEFUNC > LINKAGELIST;
typedef std::map< std::string, LINKAGELIST > LINKAGEDICT;
typedef std::map< LINKAGEFUNC, std::string > ALGINFODICT;


class AlgorithmShell : public IDataObserver
{
protected:
    Dataset m_dataset;
    bool m_linkageEnable;
    ALGINFODICT m_algInfoDict;
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

    void registerAlgInfo(LINKAGEFUNC func, std::string name);

public:
    virtual void onFirstData(std::string key, const dw::any& value);
    virtual void onDataChanged(std::string key, const dw::any& value);
    virtual void onBatchDataChanged(const DATALIST& values);

protected:
    LINKAGELIST getLinkedFunction(std::string key);
    void triggerLinkages(const LINKAGELIST& linkages);

    std::string getAlgorithmName(LINKAGEFUNC func);
};


class anysyncif;
class anycache
{
protected:
    std::vector< anysyncif* > m_caches;
protected:
    anycache();
    ~anycache();
public:
    static anycache& getInstance();
public:
    void registerWriter(const std::string& key, anysyncif* writer);
    void unregisterWriter(anysyncif* writer);
    bool getCachedData(const std::string& key, dw::any& any) const;
protected:
    void syncReferenceVariant(anysyncif* sync);
};

//template< typename T >
//T anyloader(Dataset& ds, std::string key, const char* errmsg)
//{
//    dw::any any;
//    bool ret = anycache::getInstance().getCachedData(key, any) || 
//               ds.get(key, any);
//    MASSERT(ret, errmsg);
//    return any.value_as< T >();
//}

template <typename T, typename R, typename = R>
struct equality_d : std::false_type {};

template <typename T, typename R>
struct equality_d<T, R, decltype(std::declval<T>() == std::declval<T>())>
    : std::true_type {};

template<typename T, typename R = bool>
struct equality : equality_d<T, R> {};

class anysyncif
{
public:
    anysyncif() { };
    virtual ~anysyncif() { };

    virtual bool r() const = 0;
    virtual bool w() const = 0;
    virtual dw::any val() const = 0;
    virtual std::string key() const = 0;
    virtual bool set(const dw::any& any) = 0;
};

template< typename T >
class anysync : anysyncif
{
protected:
    T& m_any;
    Dataset& m_ds;
    std::string m_key;
    bool m_load;
    bool m_write;
    const char* m_algorithm;
    static const T m_defaultVal;
public:
    anysync(Dataset& ds, std::string key, T& any, bool _load, bool write, const char* algorithm)
        : m_any(any), m_ds(ds), m_key(key), m_load(_load), m_write(write), m_algorithm(algorithm)
    {
        if (m_load)
        {
            load();
        }
        anycache::getInstance().registerWriter(key, this);
    }
    ~anysync()
    {
        if (m_write)
        {
            warning();
            write();
        }
        anycache::getInstance().unregisterWriter(this);
    }

    bool load()
    {
        dw::any any;
        bool ret = anycache::getInstance().getCachedData(m_key, any) ||
            m_ds.get(m_key, any);
        if (ret)
        {
            m_any = any.value_as< T >();
        }
        else 
        {
            printf("Load %s as %s Fail.\n", m_key.c_str(), typeid(T).name());
        }
        return ret;
    }

    bool write()
    {
        return m_ds.set(m_key, dw::any(m_any));
    }

    void discard()
    {
        m_write = false;
    }

    virtual bool r() const
    {
        return m_load;
    }
    virtual bool w() const
    {
        return m_write;
    }
    virtual std::string key() const
    {
        return m_key;
    }
    virtual dw::any val() const
    {
        return dw::any(m_any);
    }
    virtual bool set(const dw::any& any)
    {
        bool ret = false;
        if (any.istype< T >())
        {
            m_any = any.value_as< T >();
            ret = true;
        }
        return ret;
    }

    void warning()
    {
        if ((!m_load) && memcmp(&m_any, &m_defaultVal, sizeof(T) == 0))
        {
            printf("Warning: ALGORITHM_UPDATES(%s) in %s is still default value.\n", m_key.c_str(), m_algorithm);
        }
    }
};

template< typename T >
const T anysync< T >::m_defaultVal = T();

class alglinker
{
public:
    alglinker(std::string key, LINKAGEFUNC func, const char* name)
    {
        ALGSHL.linkAlgorithm(func, key);
        ALGSHL.registerLinkage(name, key);
        ALGSHL.registerAlgInfo(func, name);
    }
    ~alglinker()
    {

    }
};


class algsilence
{
public:
    algsilence()
    {
        ALGSHL.enableLinkage(false);
    }
    ~algsilence()
    {
        ALGSHL.enableLinkage(true);
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
