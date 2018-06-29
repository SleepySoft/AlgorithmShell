#include "dataset.h"

#include <algorithm>


ObserverOrganizer::ObserverOrganizer()
{

}
ObserverOrganizer::~ObserverOrganizer()
{
	destroyObservers();
}

bool ObserverOrganizer::addObserverTo(IDataObserver* ob, const std::string& key)
{
	bool ret = addObserverForKey(key, ob);
	if (ret)
	{
		if (std::find(m_observerList.begin(), m_observerList.end(), ob) == m_observerList.end())
		{
			m_observerList.push_back(ob);
		}
		if (std::find(m_observedList.begin(), m_observedList.end(), key) == m_observedList.end())
		{
			m_observedList.push_back(key);
		}
	}
	return ret;
}
void ObserverOrganizer::removeObserver(IDataObserver* ob)
{
	auto iterObserver = std::find(m_observerList.begin(), m_observerList.end(), ob);
	if (iterObserver != m_observerList.end())
	{
		auto iterObserved = m_observedList.begin();
		for (; iterObserved != m_observedList.end(); )
		{
			std::string key = (*iterObserved);
			removeObserverForKey(key, ob);

			// Remove from observed list if nobody observe it.
			if (m_observerDict.find(key) == m_observerDict.end())
			{
				iterObserved = m_observedList.erase(iterObserved);
			}
			else
			{
				++iterObserved;
			}
		}
		m_observerList.erase(iterObserver);
	}
}
void ObserverOrganizer::destroyObservers()
{
	for (auto iter = m_observerList.begin();
		iter != m_observerList.end(); ++iter)
	{
		delete (*iter);
		(*iter) = NULL;
	}
	m_updatesDict.clear();
	m_observerDict.clear();
	m_observerList.clear();
	m_observedList.clear();
}

void ObserverOrganizer::addUpdate(const std::string& key, const dw::any& value)
{
	// Is there any observer watching this data?
	auto obQueue = findObserverQueue(key);
	if (obQueue != NULL)
	{
		// If yes. For each observer.
		auto iter = obQueue->begin();
		for (; iter != obQueue ->end(); ++iter)
		{
			// Add this update to its list.
			IDataObserver* ob = (*iter);
			DATALIST* obUpdates = findCreateObserverDataList(ob);
			if (obUpdates != NULL)
			{
				obUpdates->push_back(std::make_tuple(key, value));
			}
		}
	}
}

void ObserverOrganizer::flushUpdatesToObservers()
{
    // To avoid recursive invoking

    UPDATESDICT tempDict;
    std::swap(tempDict, m_updatesDict);

	auto iter = tempDict.begin();
	for (; iter != tempDict.end(); ++iter)
	{
		IDataObserver* ob = (*iter).first;
		DATALIST obUpdates = (*iter).second;
		if (obUpdates.size() == 1)
		{
			// If only one data changed, just invoke the single-notify function.
			ob->onDataChanged(std::get<0>(obUpdates.front()), std::get<1>(obUpdates.front()));
		}
		else
		{
			// Otherwise the batch-notify function
			ob->onBatchDataChanged(obUpdates);
		}
	}
}

bool ObserverOrganizer::addObserverForKey(const std::string& key, IDataObserver* ob)
{
	bool ret = false;
	auto keyObs = findObserverQueue(key);
	if (keyObs == NULL)
	{
		m_observerDict[key] = OBSERVERLIST();
		keyObs = &(m_observerDict[key]);
	}
	if (std::find(keyObs->begin(), keyObs->end(), ob) == keyObs->end())
	{
		ret = true;
		keyObs->push_back(ob);
	}
	return ret;
}
void ObserverOrganizer::removeObserverForKey(const std::string& key, IDataObserver* ob)
{
	auto keyObs = findObserverQueue(key);
	if (keyObs != NULL)
	{
		auto iter = std::find(keyObs->begin(), keyObs->end(), ob);
		if (iter != keyObs->end())
		{
			keyObs->erase(iter);
		}
		// Remove if empty.
		if (keyObs->empty())
		{
			//auto keyIter = find(m_observedList.begin(), m_observedList.end(), key);
			//if (keyIter != m_observedList.end())
			//{
			//	m_observedList.erase(keyIter);
			//}
			m_observerDict.erase(key);
		}
	}
}

DATALIST* ObserverOrganizer::findCreateObserverDataList(IDataObserver* ob)
{
	DATALIST* obDataList = NULL;
	auto iter = m_updatesDict.find(ob);
	if (iter != m_updatesDict.end())
	{
		obDataList = &((*iter).second);
	}
	else
	{
		m_updatesDict[ob] = DATALIST();
		obDataList = &(m_updatesDict[ob]);
	}
	return obDataList;
}

std::deque< IDataObserver* >* ObserverOrganizer::findObserverQueue(const std::string& key)
{
	std::deque< IDataObserver* >* obQueue = NULL;
	auto iter = m_observerDict.find(key);
	if (iter != m_observerDict.end())
	{
		obQueue = &((*iter).second);
	}
	return obQueue;
}



Dataset::Dataset()
{
	m_lockType = true;
}


Dataset::~Dataset()
{

}

void Dataset::clear()
{
	requestDataAccess();

	auto iter = m_dataDict.begin();
	for (; iter != m_dataDict.end(); ++iter)
	{
		delete (*iter).second;
		(*iter).second = NULL;
	}
	m_dataDict.clear();

	releaseDataAccess();
}

void Dataset::lockType(bool lock)
{
	m_lockType = lock;
}

dw::any Dataset::get(std::string key) const
{
	dw::any value;
	requestDataAccess();

	dw::any* data = find(key);
	if (data != NULL)
	{
		value = (*data);
	}

	releaseDataAccess();
	return value;
}
bool Dataset::get(std::string key, dw::any& value) const
{
	bool ret = false;
	requestDataAccess();

	dw::any* data = find(key);
	if (data != NULL)
	{
		value = (*data);
		ret = true;
	}

	releaseDataAccess();
	return ret;
}
bool Dataset::set(std::string key, const dw::any& value)
{
	bool ret = false;
	requestDataAccess();

	if (update(key, value))
	{
		addUpdate(key, value);
		commitUpdates();
		ret = true;
	}

	releaseDataAccess();
	return ret;
}

DATADICT Dataset::gets(std::vector< std::string > keys)
{
	DATADICT datadict;
	requestDataAccess();

	auto iter = keys.begin();
	for (; iter != keys.end(); ++iter)
	{
		dw::any* data = find(*iter);
		if (data != NULL)
		{
			datadict[(*iter)] = (*data);
		}
	}

	releaseDataAccess();
	return datadict;
}
void Dataset::sets(const DATADICT& values)
{
	requestDataAccess();

	auto iter = values.begin();
	for (; iter != values.end(); ++iter)
	{
		update((*iter).first, (*iter).second);
		addUpdate((*iter).first, (*iter).second);
	}
	commitUpdates();

	releaseDataAccess();
}

bool Dataset::addDataObserver(std::string key, IDataObserver* ob)
{
	bool ret = false;
	requestDataAccess();

	if ((!key.empty()) && (ob != NULL))
	{
		ret = m_obOrganizer.addObserverTo(ob, key);
		if (ret)
		{
			dw::any* data = find(key);
			if (data != NULL)
			{
				dw::any value(*data);
				ob->onFirstData(key, value);
			}
		}
	}

	releaseDataAccess();
	return ret;
}
void Dataset::removeDataObserver(IDataObserver* ob)
{
	requestDataAccess();
	m_obOrganizer.removeObserver(ob);
	releaseDataAccess();
}

// protected:

bool Dataset::requestDataAccess() const
{
	return true;
}
bool Dataset::releaseDataAccess() const
{
	return true;
}

void Dataset::addUpdate(const std::string& key, const dw::any& value)
{
	m_obOrganizer.addUpdate(key, value);
}
void Dataset::commitUpdates()
{
	m_obOrganizer.flushUpdatesToObservers();
}

dw::any* Dataset::find(const std::string& key) const
{
	dw::any* data = NULL;
	auto iter = m_dataDict.find(key);
	if (iter != m_dataDict.end())
	{
		data = (*iter).second;
	}
	return data;
}
bool Dataset::update(const std::string& key, const dw::any& value)
{
	bool ret = false;
	dw::any* data = find(key);
	if (data == NULL)
	{
		data = create(key, value);
		ret = (data != NULL);
	}
	else
	{
		if ((m_lockType == false) || 
			(m_lockType && (value.type_info() == data->type_info())))
		{
			ret = data->set(value);
		}
	}
	return ret;
}
dw::any* Dataset::create(const std::string& key, const dw::any& value)
{
	dw::any* data = new dw::any(value);
	m_dataDict[key] = data;
	return data;
}
