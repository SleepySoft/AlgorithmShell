#include "AlgorithmShell.h"

#include <set>

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

void AlgorithmShell::registerAlgInfo(LINKAGEFUNC func, std::string name)
{
    m_algInfoDict[func] = name;
}

void AlgorithmShell::onFirstData(std::string key, const dw::any& value)
{
    (void)(key);
    (void)(value);
}
void AlgorithmShell::onDataChanged(std::string key, const dw::any& value)
{
    (void)(value);
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

void AlgorithmShell::triggerLinkages(const LINKAGELIST& linkages)
{
    static uint32_t indent = 0;
    for (auto iter = linkages.begin(); iter != linkages.end(); ++iter)
    {
        printf("%*c|-%s\n", indent + 1, ' ', getAlgorithmName((*iter)).c_str());
        indent += 2;
        (*iter)();
        indent -= 2;
    }
}

std::string AlgorithmShell::getAlgorithmName(LINKAGEFUNC func)
{
    std::string name;
    if (m_algInfoDict.find(func) != m_algInfoDict.end())
    {
        name = m_algInfoDict[func];
    }
    return name;
}



anycache::anycache()
{

}
anycache::~anycache()
{

}

anycache& anycache::getInstance()
{
    static anycache instance;
    return instance;
}

void anycache::registerWriter(const std::string& key, anysyncif* writer)
{
    uint32_t wcount = 0;
    uint32_t rcount = 0;

    auto iter = m_caches.begin();
    for (; iter != m_caches.end(); iter++)
    {
        if ((*iter)->key() == key)
        {
            wcount += (*iter)->w() ? 1 : 0;
            rcount += (*iter)->r() ? 1 : 0;
        }
    }
    m_caches.push_back(writer);
}
void anycache::unregisterWriter(anysyncif* writer)
{
    auto iter = std::find(m_caches.begin(), m_caches.end(), writer);
    if (iter != m_caches.end())
    {
        m_caches.erase(iter);
    }
    if (writer->w())
    {
        syncReferenceVariant(writer);
    }
}
bool anycache::getCachedData(const std::string& key, dw::any& any) const
{
    auto iter = m_caches.rbegin();
    for (; iter != m_caches.rend(); ++iter)
    {
        if ((*iter)->w() && ((*iter)->key() == key))
        {
            any = (*iter)->val();
            break;
        }
    }
    return (iter != m_caches.rend());
}


void anycache::syncReferenceVariant(anysyncif* sync)
{
    uint32_t rcount = 0;
    std::string key(sync->key());

    auto iter = m_caches.begin();
    for (; iter != m_caches.end(); iter++)
    {
        if ((*iter)->key() == key)
        {
            if ((*iter)->r())
            {
                rcount++;
                if (!(*iter)->set(sync->val()))
                {
                    printf("Warning : Type mismatch while updating previous value.\n");
                }
            }
        }
    }

    if (rcount > 0)
    {
        printf("%s sync %d variants.\n", key.c_str(), rcount);
    }
}
