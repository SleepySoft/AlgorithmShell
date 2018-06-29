#ifndef _ALGORITHM_SHELL_SLEEPY_H_
#define _ALGORITHM_SHELL_SLEEPY_H_

#include <map>
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

//typedef std::vector< std::string > STRLIST;
//
//#define ALGORITHM_START(alg)
//
//#define ALGORITHM_LINKAGE(key)
//
//#define ALGORITHM_INPUTS(type, var)
//#define ALGORITHM_OUTPUTS(type, var)
//
//#define ALGORITHM_DEPENDS(key, type, var)
//#define ALGORITHM_UPDATES(key, var)
//
//#define ALGORITHM_END(alg)
//

//#define ALGORITHM_INVOKE(alg, ...) \
//{\
//	AlgorithmBase* alg_base = AlgorithmShell::getInstance()->getAlgorithm(#alg);\
//	alg* alg_specified = dynamic_cast< alg* >(alg_base);\
//	if (alg_specified != NULL)\
//	{\
//		alg_specified->setup();\
//		alg_specified->invoke(##__VA_ARGS__);\
//		alg_specified->teardown();\
//	}\
//}
//
//#define ALGORITHM_DECLARE_START(name) 
//#define ALGORITHM_DECLARE_VARIANT() 
//#define ALGORITHM_DECLARE_LINKAGE() 
//#define ALGORITHM_DECLARE_DEPENDS() 
//#define ALGORITHM_DECLARE_UPDATES() 
//#define ALGORITHM_DECLARE_METHODS() 
//#define ALGORITHM_DECLARE_METHODS() 
//#define ALGORITHM_DECLARE_END(name) 


//class AlgorithmBase : public IDataObserver
//{
//public:
//	AlgorithmBase();
//	virtual ~AlgorithmBase();
//
//	virtual const char* name() const = 0;
//
//	virtual void createLinkage() { /*Put your ALGORITHM_LINKAGE() here.*/ };
//	virtual void reloadDepends() { /*Put your ALGORITHM_DEPENDS() here.*/ };
//	virtual void commitUpdates() { /*Put your ALGORITHM_UPDATES() here.*/ };
//
//	// Default invoke - no parameter.
//	virtual void invoke() { assert(false);  };
//
//public:
//	void setup();
//	void teardown();
//
//protected:
//	void ALGORITHM_LINKAGE(std::string key);
//
//	template< typename T >
//	void ALGORITHM_DEPENDS(std::string key, T& var)
//	{
//		var = getAny(key).value_as< T >(var);
//	}
//
//	template< typename T >
//	void ALGORITHM_UPDATES(std::string key, const T& var)
//	{
//		setAny(key, dw::any(var));
//	}
//
//private:
//	void getAny(const std::string& key, dw::any& any);
//	void setAny(const std::string& key, const dw::any& any);
//};


// Quick access of AlgorithmShell instance
#define ALGSHL (*AlgorithmShell::getInstance())

// If the algorithm has parameters, please use std::bind to provide a NO-PARAM function.
// MUST placed after function declaration.
#define ALGORITHM_LINKAGE(key, func) static alglinker __##key##_link_##func = alglinker(TOSTR(key), func);

// Depends on this data. Read only. Data will be prepared when this function enteres.
#define ALGORITHM_DEPENDS(key, type) type key = type(); key = anyloader< type >(ALGSHL.getDataset(), TOSTR(key), "Depends value '" TOSTR(key) "' missing in algorithm: " __FUNCTION__);

// Updates this data. Write only. Data will be updated when this function quit.
#define ALGORITHM_UPDATES(key, type) type key = type(); anywriter< type > __aw_##key(ALGSHL.getDataset(), TOSTR(key), key, __FUNCTION__);

// Manages this data. Read and Write Data will be prepared when this function enteres and updated when this function quits. 
#define ALGORITHM_MANAGES(key, type) ALGORITHM_DEPENDS(key, type); anywriter< type > __aw_##key(ALGSHL.getDataset(), TOSTR(key), key, __FUNCTION__);

typedef void(*LINKAGEFUNC)(void);
typedef std::vector< LINKAGEFUNC > LINKAGELIST;
typedef std::map< std::string, LINKAGELIST > LINKAGEDICT;


//class AlgorithmTemplate : public AlgorithmBase
//{
//protected:
//	static AlgorithmBase* creator(void) { return new AlgorithmTemplate(); }
//	AlgorithmTemplate() { AlgorithmShell::getInstance()->addAlgorithm("AlgorithmTemplate", AlgorithmTemplate::creator); };
//public:
//	virtual ~AlgorithmTemplate() { };
//	virtual const char* name() const { return "AlgorithmTemplate"; };
//
//public:
//};



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

public:
    virtual void onFirstData(std::string key, const dw::any& value);
    virtual void onDataChanged(std::string key, const dw::any& value);
    virtual void onBatchDataChanged(const DATALIST& values);

protected:
    LINKAGELIST getLinkedFunction(std::string key);

    //protected:
    //	ALGCREATOR getAlgCreator(std::string alg_name);
    //	ALGNAMELIST getAlgLinkage(std::string data_name);
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
    alglinker(std::string key, LINKAGEFUNC func)
    {
        ALGSHL.linkAlgorithm(func, key);
    }
    ~alglinker()
    {

    }
};



#endif // _ALGORITHM_SHELL_SLEEPY_H_