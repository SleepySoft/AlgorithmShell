#include "AlgorithmShell.h"

#include <set>

// To ensure nobody can access dataset outside.

static Dataset g_dataset = Dataset();



//AlgorithmBase::AlgorithmBase()
//{
//
//}
//AlgorithmBase::~AlgorithmBase()
//{
//	g_dataset.removeDataObserver(this);
//}
//
//void AlgorithmBase::setup()
//{
//	createLinkage();
//	reloadDepends();
//}
//void AlgorithmBase::teardown()
//{
//	commitUpdates();
//}
//
//void AlgorithmBase::ALGORITHM_LINKAGE(std::string key)
//{
//	AlgorithmShell::getInstance()->addAlgorithmLinkage(name(), key);
//}
//
//void AlgorithmBase::getAny(const std::string& key, dw::any& any)
//{
//	any = g_dataset.get(key);
//}
//
//void AlgorithmBase::setAny(const std::string& key, const dw::any& any)
//{
//	g_dataset.set(key, any);
//}




AlgorithmShell::AlgorithmShell()
{
    m_linkageEnable = false;
}


AlgorithmShell::~AlgorithmShell()
{
    m_dataset.removeDataObserver(this);
}

AlgorithmShell* AlgorithmShell::m_instance = NULL;

AlgorithmShell* AlgorithmShell::getInstance()
{
    if (m_instance == NULL)
    {
        m_instance = new AlgorithmShell();
    }
    return m_instance;
}

void AlgorithmShell::releaseInstance()
{
    delete m_instance;
    m_instance = NULL;
}

Dataset& AlgorithmShell::getDataset()
{
    return m_dataset;
}

void AlgorithmShell::enableLinkage(bool enable)
{
    m_linkageEnable = enable;
}

void AlgorithmShell::linkAlgorithm(LINKAGEFUNC func, std::string key)
{
    auto iter = m_algLinkageDict.find(key);
    if (iter == m_algLinkageDict.end())
    {
        m_algLinkageDict[key] = LINKAGELIST();
        m_algLinkageDict[key].push_back(func);
    }
    else
    {
        LINKAGELIST& linkage = (*iter).second;
        if (std::find(linkage.begin(), linkage.end(), func) == linkage.end())
        {
            linkage.push_back(func);
        }
    }
    m_dataset.addDataObserver(key, this);
}


//AlgorithmBase* AlgorithmShell::getAlgorithm(std::string alg_name)
//{
//	AlgorithmBase* alg = NULL;
//	ALGCREATOR creator = getAlgCreator(alg_name);
//	if (creator != NULL)
//	{
//		alg = creator();
//	}
//	return alg;
//}
//
//void AlgorithmShell::addAlgorithm(std::string alg_name, ALGCREATOR alg_creator)
//{
//	m_algCreatorDict[alg_name] = alg_creator;
//}
//
//void AlgorithmShell::addAlgorithmLinkage(std::string alg_name, std::string data_name)
//{
//	ALGNAMELIST linkage = getAlgLinkage(data_name);
//	if (std::find(linkage.begin(), linkage.end(), data_name) == linkage.end())
//	{
//		linkage.push_back(alg_name);
//		m_algLinkageDict[data_name] = linkage;
//	}
//}

void AlgorithmShell::onFirstData(std::string key, const dw::any& value)
{
    (void)(key);
    (void)(value);
}
void AlgorithmShell::onDataChanged(std::string key, const dw::any& value)
{
    if (m_linkageEnable)
    {
        LINKAGELIST linkage = getLinkedFunction(key);
        triggerLinkages(linkage);
    }
}
void AlgorithmShell::onBatchDataChanged(const DATALIST& values)
{
    if (m_linkageEnable)
    {
        LINKAGELIST all_linkage = LINKAGELIST();
        for (auto iter = values.begin(); iter != values.end(); ++iter)
        {
            std::string key = std::get<0>((*iter));
            LINKAGELIST linkage = getLinkedFunction(key);
            all_linkage.insert(all_linkage.end(), linkage.begin(), linkage.end());
        }

        std::set< LINKAGEFUNC > s(all_linkage.begin(), all_linkage.end());
        all_linkage.assign(s.begin(), s.end());

        triggerLinkages(all_linkage);
    }
}

LINKAGELIST AlgorithmShell::getLinkedFunction(std::string key)
{
    LINKAGELIST linkage = LINKAGELIST();
    auto iter = m_algLinkageDict.find(key);
    if (iter != m_algLinkageDict.end())
    {
        linkage = (*iter).second;
    }
    return linkage;
}

//ALGCREATOR AlgorithmShell::getAlgCreator(std::string alg_name)
//{
//	ALGCREATOR creator = NULL;
//	auto iter = m_algCreatorDict.find(alg_name);
//	if (iter != m_algCreatorDict.end())
//	{
//		creator = (*iter).second;
//	}
//	return creator;
//}
//LINKAGELIST AlgorithmShell::getAlgLinkage(std::string data_name)
//{
//	LINKAGELIST linkage = LINKAGELIST();
//	auto iter = m_algLinkageDict.find(data_name);
//	if (iter != m_algLinkageDict.end())
//	{
//		linkage = (*iter).second;
//	}
//	return linkage;
//}

void AlgorithmShell::triggerLinkages(const LINKAGELIST& linkages)
{
    for (auto iter = linkages.begin(); iter != linkages.end(); ++iter)
    {
        (*iter)();
    }
}
