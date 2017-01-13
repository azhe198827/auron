
#ifndef _GLOBAL_PARA_H__
#define _GLOBAL_PARA_H__

#include <boost/property_tree/ptree.hpp>  
#include <boost/property_tree/ini_parser.hpp> 

class Config
{
	boost::property_tree::ptree m_pt;
public:
	void Load(const char* file_name)
	{
		boost::property_tree::ini_parser::read_ini(file_name, m_pt);
	}

	template<class T>
	void SetCustom(std::string key, T val)
	{
		m_pt.put<T>(key, val);
	}

	bool IsExists(const char* key)
	{
		return m_pt.find(key) == m_pt.not_found();
	}

	template<class T>
	T Get(const char* key)
	{
		return m_pt.get<T>(key);
	}
};

#endif