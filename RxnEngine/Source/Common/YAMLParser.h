#pragma once
#include <unordered_map>
#include <stack>
#include <vector>
#include <iostream>
#include <filesystem>
#include <fstream>

namespace Rxn::Common
{
    class Node
    {
    public:

        Node();
        ~Node();

    public:
        
        void SetNodeKey(const std::string& k);
        void SetScalarValue(const std::string& v);
        void SetListValue(const std::vector<Node>& v);
        void SetMapValue(const std::unordered_map<std::string, Node>& v);
        void SetNodeDepth(int depth);

        bool IsScalar();

        std::string GetScalarValue();
        std::vector<Node> GetListValue();
        std::unordered_map<std::string, Node> GetMapValue();
        int GetNodeDepth();

    private:

        bool m_bIsScalar;

        std::unordered_map<std::string, Node> m_umMapValue;
        std::vector<Node> m_ulListValue;
        std::string m_szScalarValue;

        int m_iNodeDepth;

        std::string m_szNodeKey;

    };

    class RXN_ENGINE_API YAMLParser
    {
    public:
        
        YAMLParser();
        ~YAMLParser();

    public:

        void ParseYAML(std::string szFileName);
        
        
    private:

        std::stack<std::string> m_szLines;
        std::unordered_map<std::string, Node> m_umHashMap;

        bool IsListNode(const std::string line);
        bool IsNested(const std::string line);
        int GetIndentDepth(const std::string line);

        void ParseNode(Node& node, std::ifstream& filestream);
        void ParseKeyValue(Node& node, const std::string line);
        void ParseList(std::vector<Node> node, std::ifstream& filestream);
        void ParseValue(Node node, const std::string& line);
        
        std::stringstream CreateStringStream(const std::string str);

        void Trim(std::string& str);
        std::string TrimLine(const std::string line);

        std::unordered_map<std::string, Node> GetLastMapValue(Node node);
        std::unordered_map<std::string, Node> GetLastMapValue(std::vector<Node> node);

       

    };
}
