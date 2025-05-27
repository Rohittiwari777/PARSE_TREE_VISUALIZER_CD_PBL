#include <iostream>
#include <vector>
#include <regex>

using namespace std;

struct Token
{
    string type;
    string value;
};

vector<Token> tokenize(const string &code)
{
    vector<Token> tokens;
    vector<regex> token_patterns = {
        regex("#[a-zA-Z_]+[^\n]*"), // Preprocessor directive
        regex("\\bint\\b"),
        regex("\\bvoid\\b"),
        regex("\\bfloat\\b"),
        regex("\\breturn\\b"),
        regex("\\bif\\b"),
        regex("\\belse\\b"),
        regex("\\bwhile\\b"),
        regex("\\bcout\\b"),
        regex("\\bcin\\b"),
        regex("\"[^\"]*\""), // string literal
        regex("<<|>>"),
        regex("==|!=|<=|>=|<|>"),
        regex("[a-zA-Z_][a-zA-Z0-9_]*"),
        regex("[0-9]+"),
        regex("[(){};,=+*/-<>%.]")};

    string::const_iterator it = code.begin();
    while (it != code.end())
    {
        if (isspace(*it))
        {
            ++it;
            continue;
        }
        bool matched = false;
        for (const auto &pat : token_patterns)
        {
            smatch match;
            if (regex_search(it, code.cend(), match, pat, regex_constants::match_continuous))
            {
                string val = match.str();
                string type;
                if (val == "int" || val == "void" || val == "float" || val == "return" || val == "if" || val == "else" || val == "while" || val == "cout" || val == "cin" || val == "string")
                    type = "keyword";
                else if (regex_match(val, regex("#[a-zA-Z_]+[^\n]*")))
                    type = "preprocessor";
                else if (regex_match(val, regex("\"[^\"]*\"")))
                    type = "string";
                else if (regex_match(val, regex("[0-9]+")))
                    type = "number";
                else if (regex_match(val, regex("[(){};,=+*/-<>%]")) ||
                         val == "==" || val == "!=" || val == "<=" || val == ">=" || val == "<" || val == ">" ||
                         val == "<<" || val == ">>")
                    type = "symbol";
                else
                    type = "identifier";
                tokens.push_back({type, val});
                it += val.length();
                matched = true;
                break;
            }
        }
        if (!matched)
            throw runtime_error("Unrecognized token: " + string(it, it + 1));
    }
    return tokens;
}
