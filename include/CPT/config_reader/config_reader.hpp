/**
 * @file config_reader.hpp
 * @brief define the config reader and config reader base type.
 */
#pragma once

#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <CPT/logger.hpp>
/**
 * @brief The ConfigReaderBase is a base class of ConfigReader 
 * which combine the configs given by user 
 * and provide parse_node and check_flag to guarantee every config have parse at least one node and no excess nodes in config file.
 * 
 * @details The ConfigReaderBase will inheritance all template parameter and get the parent methods to parse the config file.
 * 
 * @tparam Config1 Any Config type require parse_node : bool(auto) and check_flag : bool(void).
 * @tparam Configs Other variadic Config types.
 */
template<class Config1, class... Configs>
class ConfigReaderBase : public Config1, public ConfigReaderBase<Configs...>
{
public : 
    /**
     * @brief The parser of node in property tree.
     * @details Each node in property tree will be "try" to parsed by every parent Config class, 
     * and the config which can do the parse will do the parsing job.
     * This class is design base on <a href="http://www.boost.org/doc/libs/1_59_0/doc/html/property_tree.html">boost property_tree</a> 
     * 
     * @param node A node in property tree.
     * @return If exist a config which can parse the node, then return true, else return false ( abort ).
     */
    template<class NODE>
    bool parse_node(NODE&& node)
    {
        if ( Config1::parse_node(node) )
        {
            return true; 
        }
        else if ( ConfigReaderBase<Configs...>::parse_node(node) )
        {
            return true;
        }
        else
        {
            cpt::fatal << "unkown config option : " << node.first << std::endl;
            exit(1);
            return false;
        }
        
    }
    /**
     * @brief Check every Configs which have parsed the node.
     * @details All parameters ( or configurations ) in config file are all necessary.
     * ConfigReaderBase have to guarantee every configs have processed at least one node.
     * 
     * @return True if all configs have processed at least one node else false.
     */
    bool check_flag()
    {
        return Config1::check_flag() && ConfigReaderBase<Configs...>::check_flag();
    }

};
/**
 * @brief The recursive end condition of ConfigReaderBase.
 */
template<class Config1>
class ConfigReaderBase<Config1> : public Config1
{
public : 
	boost::property_tree::ptree p_tree_;
    template<class NODE>
    bool parse_node(NODE&& node)
    {
        bool res ( Config1::parse_node(node) );
        if ( res )
        {}
        else
        {
            cpt::fatal << "unkown config option : " << node.first << std::endl;
            exit(1);
        }
        return res;
        
    }
    bool check_flag()
    {
        return Config1::check_flag();
    }
};
/**
 * @brief The wrap of ConfigReaderBase.
 * @details Provide the interface to parse the config file.
 * 
 * @tparam Config Any Config type require parse_node : bool(auto) and check_flag : bool(void).
 */
template<class... Config>
class ConfigReader : public ConfigReaderBase<Config...>
{
public: 
    void parse(const std::string& path)
    {
        std::ifstream fin ( path ); //ihandler
        boost::property_tree::json_parser::read_json ( fin, this->p_tree_ ); // stream policy
        for(auto&& node : this->p_tree_)
        {
            ConfigReaderBase<Config...>::parse_node(node);
        }
        if( ConfigReaderBase<Config...>::check_flag() ) return;
        else 
        {
            cpt::fatal << "Config file has miss option, program exit" << std::endl;
            exit(1);
        }
    }
};
