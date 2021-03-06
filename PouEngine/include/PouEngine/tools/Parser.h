#ifndef PARSER_H
#define PARSER_H

#include "PouEngine/core/Singleton.h"

///I should maybe switch to functions in a namespace

namespace pou
{

class Parser : public Singleton<Parser>
{
    public:
        friend class Singleton<Parser>;

        static bool isBool(const std::string&);
        static bool isInt(const std::string&);
        static bool isFloat(const std::string&);

        static bool  parseBool(const std::string&);
        static int   parseInt(const std::string&);
        static float parseFloat(const std::string&);

        static void parseSegment(const std::string&, std::string &lhs, std::string &rhs);
        static std::pair<float, float> parseFloatSegment(const std::string&);
        static std::pair<int, int> parseIntSegment(const std::string&);

        static std::string findFileDirectory(const std::string&);
        static std::string findFileExtension(const std::string&);
        static std::string removeFileExtension(const std::string&);
        static std::string simplifyFilePath(const std::string &);

    protected:
        Parser();
        virtual ~Parser();


    private:
};

}

#endif // PARSER_H
