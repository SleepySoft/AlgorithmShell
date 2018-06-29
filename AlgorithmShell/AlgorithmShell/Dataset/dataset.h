#ifndef _DATASET_H
#define _DATASET_H

#include <map>
#include <list>
#include <tuple>
#include <deque>
#include <vector>
#include <string>

#include "data_wrapper.h"

typedef std::map< std::string, dw::any > DATADICT;
typedef std::deque< std::tuple< std::string , dw::any > > DATALIST;

class IDataObserver
{
public:
	IDataObserver() { }
	virtual ~IDataObserver() { }

	virtual void onFirstData(std::string key, const dw::any& value) = 0;
	virtual void onDataChanged(std::string key, const dw::any& value) = 0;
	virtual void onBatchDataChanged(const DATALIST& values) = 0;
};


typedef std::deque< IDataObserver* > OBSERVERLIST;
typedef std::map< std::string, OBSERVERLIST > OBSERVERDICT;
typedef std::map< IDataObserver*, DATALIST > UPDATESDICT;


class ObserverOrganizer
{
protected:
	UPDATESDICT m_updatesDict;						// Chache the updates for each ob
	OBSERVERDICT m_observerDict;					// Store the observer for each data

	OBSERVERLIST m_observerList;					// Store the observers, unique.
	std::deque< std::string > m_observedList;

public:
	ObserverOrganizer();
	~ObserverOrganizer();

	// Return false if the observer already exists on this key.
	bool addObserverTo(IDataObserver* ob, const std::string& key);
	void removeObserver(IDataObserver* ob);
	void destroyObservers();

	void addUpdate(const std::string& key, const dw::any& value);

	void flushUpdatesToObservers();

protected:
	bool addObserverForKey(const std::string& key, IDataObserver* ob);
	void removeObserverForKey(const std::string& key, IDataObserver* ob);

	DATALIST* findCreateObserverDataList(IDataObserver* ob);
	std::deque< IDataObserver* >* findObserverQueue(const std::string& key);
};


class Dataset
{
protected:
	typedef std::map< std::string, dw::any* > DATAPTDICT;
protected:
	bool m_lockType;
	DATAPTDICT m_dataDict;
	ObserverOrganizer m_obOrganizer;

public:
	Dataset();
	~Dataset();

	void clear();
	void lockType(bool lock);

	dw::any get(std::string key) const;
	bool get(std::string key, dw::any& value) const;
	bool set(std::string key, const dw::any& value);

    template< typename T >
    bool check(std::string key, const T& expect) const
    {
        dw::any value;
        bool ret = get(key, value);
        ret = ret && value.check< T >(expect);
        return ret;
    }
    bool check(std::string key, const char* expect) const
    {
        dw::any value;
        bool ret = get(key, value);
        ret = ret && value.check(expect);
        return ret;
    }

	DATADICT gets(std::vector< std::string > keys);
	void sets(const DATADICT& values);

	bool addDataObserver(std::string key, IDataObserver* ob);
	void removeDataObserver(IDataObserver* ob);

protected:
	bool requestDataAccess() const;
	bool releaseDataAccess() const;

	void addUpdate(const std::string& key, const dw::any& value);
	void commitUpdates();

	dw::any* find(const std::string&) const;
	bool update(const std::string&, const dw::any& value);
	dw::any* create(const std::string& key, const dw::any& value);
};

#endif // _DATASET_H