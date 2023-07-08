#include "Rxn.h"
#include "YAMLParser.h"


namespace Rxn::Common
{
    YAMLParser::YAMLParser()
    {
    }
    YAMLParser::~YAMLParser()
    {
    }

    void YAMLParser::ParseYAML(std::string szFileName)
    {
        std::filesystem::path filePath = szFileName;

        if (!std::filesystem::exists(filePath))
        {
            RXN_LOGGER::Error(L"config '%s' does not exist.", szFileName);
        }

        std::ifstream fileStream(filePath, std::ios::in);

        if (!fileStream.is_open())
        {
            Logger::Error(L"Failed to open config %s", szFileName);
        }

        //std::string line;
        Node rootNode;
        ParseNode(rootNode, fileStream);

        fileStream.close();
    }

    void YAMLParser::ParseNode(Node &node, std::ifstream &filestream)
    {
        std::string line;
        while (std::getline(filestream, line))
        {

            size_t depth = GetIndentDepth(line);
            std::string value = TrimLine(line);

            if (depth == 0)
            {
                // Root level key
                node.SetMapValue(std::unordered_map<std::string, Node>());
                //ParseKeyValue(node.GetMapValue(), value);
            }
            else
            {
                // Nested key
                std::unordered_map<std::string, Node> mapValue = GetLastMapValue(node);
                //ParseKeyValue(mapValue, value);
            }
        }

    }

    void YAMLParser::ParseKeyValue(Node &node, const String line)
    {
        size_t colonIndex = line.find(':');

        std::vector<String> pairs;
        std::stringstream stream(line);
        String streamedString;
        while (std::getline(stream, streamedString, ':'))
        {
            pairs.push_back(streamedString);
        }

        if (pairs.size() > 1)
        {
            std::string key = pairs.at(0);
            std::string value = pairs.at(1);

            Trim(key);
            Trim(value);

            Node node;
            ParseValue(node, value);
            //map[key] = node;
        }
    }

    void YAMLParser::ParseList(std::vector<Node> listNode, std::ifstream &filestream)
    {
        /*std::string line;

        while (std::getline(filestream, line)) {
            size_t indentLevel = GetIndentDepth(line);
            std::string value = TrimLine(line);

            if (indentLevel > 0) {
                std::unordered_map<std::string, Node> mapValue = GetLastMapValue(listNode);
                ParseKeyValue(mapValue, value, indentLevel);
            }
            else
            {
                Node node;
                ParseValue(node, value);
                listNode.push_back(node);
            }
        }*/
    }

    void YAMLParser::ParseValue(Node node, const std::string &value)
    {
        /*if (IsNested(value))
        {
            std::unordered_map<std::string, Node> mapNode;
            ParseNode(mapNode, CreateStringStream(value));
            node.SetMapValue(mapNode);
        }
        else if (IsListNode(value))
        {
            std::vector<Node> listNode;
            ParseList(listNode, CreateStringStream(value));
            node.SetListValue(listNode);
        }
        else
        {
            node.SetScalarValue(value);
        }*/
    }

    std::stringstream YAMLParser::CreateStringStream(const std::string str)
    {
        std::stringstream stream(str);
        return stream;
    }

    void YAMLParser::Trim(std::string &str)
    {
        size_t startPos = str.find_first_not_of(' ');
        size_t endPos = str.find_last_not_of(' ');

        if (startPos != std::string::npos && endPos != std::string::npos)
        {
            str = str.substr(startPos, endPos - startPos + 1);
        }
        else
        {
            str.clear();
        }
    }

    std::unordered_map<std::string, Node> YAMLParser::GetLastMapValue(Node node)
    {
        /*if (node.IsScalar())
        {
            node.SetMapValue(std::unordered_map<std::string, Node>());
        }
        return node.GetMapValue();*/
        return std::unordered_map<std::string, Node>();
    }

    std::unordered_map<std::string, Node> YAMLParser::GetLastMapValue(std::vector<Node> node)
    {
        /* Node lastNode = node.back();
         if (lastNode.IsScalar())
         {
             lastNode.SetMapValue(std::unordered_map<std::string, Node>());
         }
         return lastNode.GetMapValue();*/
        return std::unordered_map<std::string, Node>();
    }

    bool YAMLParser::IsNested(const std::string line)
    {
        return line.find(':') != std::string::npos;
    }

    std::string YAMLParser::TrimLine(const std::string line)
    {
        size_t startPos = line.find_first_not_of(' ');
        size_t endPos = line.find_last_not_of(' ');

        if (startPos != std::string::npos && endPos != std::string::npos) {
            return line.substr(startPos, endPos - startPos + 1);
        }

        return "";
    }

    int YAMLParser::GetIndentDepth(const std::string line)
    {
        int spaces = 0;
        while (spaces < line.size() && line[spaces] == ' ')
        {
            ++spaces;
        }
        return spaces;
    }

    bool YAMLParser::IsListNode(const std::string line)
    {
        return line.find('-') != std::string::npos;
    }


    Node::Node()
        : m_bIsScalar(true)
    {
    }

    Node::~Node() = default;

    void Node::SetNodeKey(const std::string &k)
    {
        this->m_szNodeKey = k;
    }

    void Node::SetScalarValue(const std::string &v)
    {
        this->m_bIsScalar = true;
        this->m_szScalarValue = v;
    }

    void Node::SetListValue(const std::vector<Node> &v)
    {
        this->m_bIsScalar = false;
        this->m_ulListValue = v;
    }

    void Node::SetMapValue(const std::unordered_map<std::string, Node> &v)
    {
        this->m_bIsScalar = false;
        this->m_umMapValue = v;
    }

    void Node::SetNodeDepth(int depth)
    {
        this->m_iNodeDepth = depth;
    }

    bool Node::IsScalar()
    {
        return this->m_bIsScalar;
    }

    std::string Node::GetScalarValue()
    {
        return this->m_szScalarValue;
    }

    std::vector<Node> Node::GetListValue()
    {
        return this->m_ulListValue;
    }

    std::unordered_map<std::string, Node> Node::GetMapValue()
    {
        return this->m_umMapValue;
    }

    int Node::GetNodeDepth()
    {
        return this->m_iNodeDepth;
    }
}


