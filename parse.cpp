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
// Tree Node
struct Node
{
    string label;
    vector<Node> children;
};

vector<Node> allFunctions; // For trace generation
vector<json> trace;        // The execution trace

// Symbol Table Entry
struct SymbolEntry
{
    string name;
    string type;
    string scope;
    int value;
    bool hasValue;
};

vector<SymbolEntry> symbolTable;

// Add this line before the Parser class definition:
int evalExpr(const Node &expr, std::unordered_map<std::string, int> &vars);

class Parser
{
    vector<Token> tokens;
    size_t pos = 0;
    string currentScope = "global";

    Token peek()
    {
        if (pos < tokens.size())
            return tokens[pos];
        throw runtime_error("Unexpected end of input");
    }
    Token advance()
    {
        if (pos < tokens.size())
            return tokens[pos++];
        throw runtime_error("Unexpected end of input");
    }
    bool match(const string &val)
    {
        if (pos < tokens.size() && tokens[pos].value == val)
        {
            ++pos;
            return true;
        }
        return false;
    }
    bool matchType(const string &type)
    {
        if (pos < tokens.size() && tokens[pos].type == type)
        {
            ++pos;
            return true;
        }
        return false;
    }

public:
    Parser(const vector<Token> &tokens) : tokens(tokens) {}

    Node parse()
    {
        Node root = {"Program"};
        // Handle preprocessor directives at the top
        while (pos < tokens.size() && tokens[pos].type == "preprocessor")
        {
            root.children.push_back({"Include: " + tokens[pos].value});
            ++pos;
        }
        // Skip 'using namespace std ;'
        while (pos + 2 < tokens.size() &&
               tokens[pos].value == "using" &&
               tokens[pos + 1].value == "namespace" &&
               tokens[pos + 2].type == "identifier")
        {
            string ns = tokens[pos + 2].value;
            root.children.push_back({"Using: namespace " + ns});
            pos += 3;
            if (pos < tokens.size() && tokens[pos].value == ";")
                ++pos;
        }
        while (pos < tokens.size())
        {
            Node func = parseFunction();
            root.children.push_back(func);
            allFunctions.push_back(func);
        }
        return root;
    }

    Node parseFunction()
    {
        Node funcNode = {"Function"};

        // Accept multiple return types
        string returnType;
        if (match("int"))
            returnType = "int";
        else if (match("void"))
            returnType = "void";
        else if (match("float"))
            returnType = "float";
        else if (match("string"))
            returnType = "string";
        else
            throw runtime_error("Expected return type");

        Token name = advance();
        if (name.type != "identifier")
            throw runtime_error("Expected function name");

        funcNode.children.push_back({"ReturnType: " + returnType});
        funcNode.children.push_back({"FunctionName: " + name.value});

        // Add function to symbol table
        symbolTable.push_back({name.value, returnType + " (function)", "global", 0, false});

        string prevScope = currentScope;
        currentScope = name.value;

        if (!match("("))
            throw runtime_error("Expected (");
        Node paramList = {"Parameters"};
        if (!match(")"))
        {
            do
            {
                // Accept multiple parameter types
                string paramType;
                if (match("int"))
                    paramType = "int";
                else if (match("void"))
                    paramType = "void";
                else if (match("float"))
                    paramType = "float";
                else if (match("string"))
                    paramType = "string";
                else
                    throw runtime_error("Expected parameter type");
                Token paramName = advance();
                if (paramName.type != "identifier")
                    throw runtime_error("Expected parameter name");
                paramList.children.push_back({paramType + " " + paramName.value});
                // Add parameter to symbol table
                symbolTable.push_back({paramName.value, paramType, currentScope, 0, false});
            } while (match(","));
            if (!match(")"))
                throw runtime_error("Expected )");
        }
        funcNode.children.push_back(paramList);

        if (!match("{"))
            throw runtime_error("Expected {");

        Node body = {"Body"};
        while (!match("}"))
        {
            body.children.push_back(parseStatement());
        }
        funcNode.children.push_back(body);

        currentScope = prevScope;
        return funcNode;
    }

    Node parseStatement()
    {
        // Variable declaration for supported types
        string varType;
        if (match("int"))
            varType = "int";
        else if (match("void"))
            varType = "void";
        else if (match("float"))
            varType = "float";
        else if (match("string"))
            varType = "string";
        if (!varType.empty())
        {
            Token varName = advance();
            if (varName.type != "identifier")
                throw runtime_error("Expected variable name");
            Node decl = {"VarDecl"};
            decl.children.push_back({varType + " " + varName.value});
            int val = 0;
            bool hasVal = false;
            if (match("="))
            {
                Node expr = parseExpression();
                decl.children.push_back(expr);
                // Try to evaluate if possible
                unordered_map<string, int> dummyVars;
                val = evalExpr(expr, dummyVars);
                hasVal = true;
            }
            if (!match(";"))
                throw runtime_error("Expected ; after variable declaration");
            // Add variable to symbol table
            symbolTable.push_back({varName.value, varType, currentScope, val, hasVal});
            return decl;
        }
        if (match("return"))
        {
            Node retNode = {"Return"};
            retNode.children.push_back(parseExpression());
            if (!match(";"))
                throw runtime_error("Expected ; after return");
            return retNode;
        }
        if (match("if"))
        {
            Node ifNode = {"If"};
            if (!match("("))
                throw runtime_error("Expected ( after if");
            ifNode.children.push_back(parseExpression());
            if (!match(")"))
                throw runtime_error("Expected ) after if condition");
            ifNode.children.push_back(parseStatement());
            if (match("else"))
                ifNode.children.push_back(parseStatement());
            return ifNode;
        }
        if (match("while"))
        {
            Node whileNode = {"While"};
            if (!match("("))
                throw runtime_error("Expected ( after while");
            whileNode.children.push_back(parseExpression());
            if (!match(")"))
                throw runtime_error("Expected ) after while condition");
            whileNode.children.push_back(parseStatement());
            return whileNode;
        }
        if (match("cout"))
        {
            Node coutNode = {"Cout"};
            // Require at least one << and expression
            if (!match("<<"))
                throw runtime_error("Expected << after cout");
            coutNode.children.push_back(parseExpression());
            // Handle additional << expressions
            while (match("<<"))
            {
                coutNode.children.push_back(parseExpression());
            }
            if (!match(";"))
                throw runtime_error("Expected ; after cout");
            return coutNode;
        }
        if (match("cin"))
        {
            Node cinNode = {"Cin"};
            if (!match(">>"))
                throw runtime_error("Expected >> after cin");
            do
            {
                Token var = advance();
                if (var.type != "identifier")
                    throw runtime_error("Expected variable after >>");
                cinNode.children.push_back({"Var: " + var.value});
            } while (match(">>"));
            if (!match(";"))
                throw runtime_error("Expected ; after cin");
            return cinNode;
        }
        if (match("{"))
        {
            Node block = {"Block"};
            while (!match("}"))
            {
                block.children.push_back(parseStatement());
            }
            return block;
        }
        // Function call or assignment
        Token first = advance();
        if (first.type == "identifier")
        {
            if (match("="))
            {
                // Assignment
                Node assign = {"Assignment"};
                assign.children.push_back({"Var: " + first.value});
                Node expr = parseExpression();
                assign.children.push_back(expr);
                // Try to update value in symbol table if possible
                for (auto &entry : symbolTable)
                {
                    if (entry.name == first.value && entry.scope == currentScope)
                    {
                        unordered_map<string, int> dummyVars;
                        entry.value = evalExpr(expr, dummyVars);
                        entry.hasValue = true;
                    }
                }
                if (!match(";"))
                    throw runtime_error("Expected ; after assignment");
                return assign;
            }
            else if (match("("))
            {
                // Function call
                Node call = {"FunctionCall"};
                call.children.push_back({"Callee: " + first.value});
                Node args = {"Arguments"};
                if (!match(")"))
                {
                    do
                    {
                        args.children.push_back(parseExpression());
                    } while (match(","));
                    if (!match(")"))
                        throw runtime_error("Expected ) after function call arguments");
                }
                call.children.push_back(args);
                if (!match(";"))
                    throw runtime_error("Expected ; after function call");
                return call;
            }
        }
        throw runtime_error("Unknown statement starting with: " + first.value);
    }

    Node parseExpression()
    {
        Node left = parseSimpleExpression();
        // Handle binary operators (==, !=, <, >, <=, >=, +, -, *, /, %)
        while (pos < tokens.size() && tokens[pos].type == "symbol" &&
               (tokens[pos].value == "+" || tokens[pos].value == "-" ||
                tokens[pos].value == "*" || tokens[pos].value == "/" ||
                tokens[pos].value == "%" ||
                tokens[pos].value == "==" || tokens[pos].value == "!=" ||
                tokens[pos].value == "<" || tokens[pos].value == ">" ||
                tokens[pos].value == "<=" || tokens[pos].value == ">="))
        {
            Token op = advance();
            Node exprNode = {"Expr"};
            exprNode.children.push_back(left);
            exprNode.children.push_back({"Op: " + op.value});
            exprNode.children.push_back(parseSimpleExpression());
            left = exprNode;
        }
        return left;
    }

    Node parseSimpleExpression()
    {
        Token left = advance();
        if (left.type == "identifier" && pos < tokens.size() && tokens[pos].value == "(")
        {
            // Function call as expression
            advance(); // consume '('
            Node call = {"FunctionCall"};
            call.children.push_back({"Callee: " + left.value});
            Node args = {"Arguments"};
            if (pos < tokens.size() && tokens[pos].value != ")")
            {
                do
                {
                    args.children.push_back(parseExpression());
                } while (match(","));
            }
            if (!match(")"))
                throw runtime_error("Expected ) after function call arguments");
            call.children.push_back(args);
            return call;
        }
        Node exprNode = {"Expr"};
        exprNode.children.push_back({"Value: " + left.value});
        return exprNode;
    }
};

int evalExpr(const Node &expr, unordered_map<string, int> &vars)
{
    if (expr.label == "Expr")
    {
        if (expr.children.size() == 1)
        {
            string val = expr.children[0].label.substr(7); // "Value: "
            if (!val.empty() && isdigit(val[0]))
                return stoi(val);
            if (vars.count(val))
                return vars[val];
            return 0;
        }
        else if (expr.children.size() == 3)
        {
            int left = evalExpr(expr.children[0], vars);
            string op = expr.children[1].label.substr(4); // "Op: "
            int right = evalExpr(expr.children[2], vars);
            if (op == "+")
                return left + right;
            if (op == "-")
                return left - right;
            if (op == "*")
                return left * right;
            if (op == "/")
                return right != 0 ? left / right : 0;
            if (op == "%")
                return right != 0 ? left % right : 0;
            if (op == "==")
                return left == right;
            if (op == "!=")
                return left != right;
            if (op == "<")
                return left < right;
            if (op == ">")
                return left > right;
            if (op == "<=")
                return left <= right;
            if (op == ">=")
                return left >= right;
        }
    }
    return 0;
}
