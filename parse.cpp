#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <regex>
#include<bits/stdc++.h>
#include <stdexcept>
#include <unordered_map>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

struct Token
{
    string type;
    string value;
};

// Tokenize the code .... 
// This function breaks a C++-like code string into tokens 
// (like keywords, identifiers, numbers, etc.) using regular expressions. 
// This is part of the lexical analysis phase of a compiler.

// Loop over each character in the input string code:
// Skip whitespace.

// Try each regex pattern in order to match a token from the current position.
// If a regex matches from the current character (it), it's a valid token.

// If nothing matches, throw an error (Unrecognized token).

// Return the full list of tokens.

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


// This is a recursive-descent parser that:
// Parses a C++-like source code from a list of tokens
// Builds a tree structure (Node) for each part (functions, statements, etc.)
// Builds a symbol table for variables/functions with scope
// Generates an AST (Abstract Syntax Tree) rooted at "Program"



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

    // ✅ Token peek()
    // Purpose: Look at the current token without moving forward in the token stream.
    // Returns: The token at tokens[pos].
    // Throws: Error if we’ve reached the end of tokens.
    // Use case: Just checking what's next, without consuming it.

    Token peek()
    {
        if (pos < tokens.size())
            return tokens[pos];
        throw runtime_error("Unexpected end of input");
    }

    // ✅ Token advance()
    // Purpose: Return the current token and then move to the next one.
    // Returns: The token at tokens[pos], then increments pos.
    // Throws: Error if there are no more tokens.
    // Use case: When you are ready to consume a token.


    Token advance()
    {
        if (pos < tokens.size())
            return tokens[pos++];
        throw runtime_error("Unexpected end of input");
    }

    ✅ bool match(const string &val)
    // Purpose: Check if the current token's value matches val.
    // If matched:
    // Moves forward (++pos)
    // Returns true
    // Else: Returns false
    // Use case: For checking specific symbols like "(", ";", etc.


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


    // What it does:
    // Creates a root node called "Program".
    // Handles preprocessor lines (e.g., #include <iostream>) and adds them as "Include: ...".
    // Skips using namespace std; and adds it as "Using: namespace std".
    // Parses all functions one by one using parseFunction() and adds them to the program's children.
    // Returns the complete syntax tree for the program.

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


// ---------------------------------------------------------------------------

    // What it does:
    // Parses a single statement (like variable declaration, return, if, while, etc.).
    // Returns the syntax tree for the statement.

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

    // What it does:
    // Parses an expression (like 1 + 2, a, a + b, etc.).
    // Returns the syntax tree for the expression.

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

    // What it does:
    // Parses a simple expression (like 1, a, a(), etc.).
    // Returns the syntax tree for the simple expression.

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

// --- Expression Evaluation ---

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

json nodeToJson(const Node &node)
{
    json j;
    j["name"] = node.label;
    j["children"] = json::array();
    for (const auto &child : node.children)
    {
        j["children"].push_back(nodeToJson(child));
    }
    return j;
}

// --- Trace Generation ---


void simulateExecution(const Node &node, unordered_map<string, int> &vars)
{
    if (node.label == "Function")
    {
        string funcName;
        for (const auto &child : node.children)
        {
            if (child.label.rfind("FunctionName:", 0) == 0)
            {
                funcName = child.label.substr(13);
                break;
            }
        }
        if (!funcName.empty())
        {
            trace.push_back({{"action", "call"}, {"function", funcName}});
            // Find body and simulate it
            for (const auto &child : node.children)
            {
                if (child.label == "Body")
                {
                    for (const auto &stmt : child.children)
                    {
                        simulateExecution(stmt, vars);
                    }
                }
            }
            trace.push_back({{"action", "return"}, {"function", funcName}});
        }
    }
    else if (node.label == "VarDecl")
    {
        string var;
        if (!node.children.empty())
        {
            string decl = node.children[0].label;
            size_t space = decl.find(' ');
            var = (space != string::npos) ? decl.substr(space + 1) : decl;
        }
        int val = 0;
        if (node.children.size() > 1)
            val = evalExpr(node.children[1], vars);
        vars[var] = val;
        trace.push_back({{"action", "vardecl"}, {"variable", var}});
    }
    else if (node.label == "Assignment")
    {
        string var;
        if (!node.children.empty())
            var = node.children[0].label.substr(5); // "Var: "
        int val = 0;
        if (node.children.size() > 1)
            val = evalExpr(node.children[1], vars);
        vars[var] = val;
        trace.push_back({{"action", "assign"}, {"variable", var}});
    }
    else if (node.label == "Return")
    {
        trace.push_back({{"action", "return_stmt"}});
        if (!node.children.empty())
            evalExpr(node.children[0], vars);
    }
    else if (node.label == "If")
    {
        trace.push_back({{"action", "if_enter"}});
        bool conditionTrue = false;
        if (!node.children.empty())
            conditionTrue = evalExpr(node.children[0], vars);
        if (conditionTrue)
        {
            trace.push_back({{"action", "if_taken"}, {"branch", "then"}});
            if (node.children.size() > 1)
                simulateExecution(node.children[1], vars);
        }
        else
        {
            trace.push_back({{"action", "if_taken"}, {"branch", "else"}});
            if (node.children.size() > 2)
                simulateExecution(node.children[2], vars);
        }
    }
    else if (node.label == "While")
    {
        trace.push_back({{"action", "while_enter"}});
        int loopCount = 0;
        while (evalExpr(node.children[0], vars) && loopCount < 10) // prevent infinite loop
        {
            if (node.children.size() > 1)
                simulateExecution(node.children[1], vars);
            loopCount++;
        }
    }
    else if (node.label == "Cout")
    {
        trace.push_back({{"action", "cout"}});
        for (const auto &child : node.children)
            evalExpr(child, vars);
    }
    else if (node.label == "Cin")
    {
        trace.push_back({{"action", "cin"}});
        // For demo, set input variable to 5 if not already set
        for (const auto &child : node.children)
        {
            if (child.label.rfind("Var: ", 0) == 0)
            {
                string var = child.label.substr(5);
                if (vars.count(var) == 0)
                    vars[var] = 5;
            }
        }
    }
    else if (node.label == "FunctionCall")
    {
        string callee;
        for (const auto &child : node.children)
        {
            if (child.label.rfind("Callee:", 0) == 0)
            {
                callee = child.label.substr(7);
                break;
            }
        }
        if (!callee.empty())
        {
            trace.push_back({{"action", "call"}, {"function", callee}});
            for (const auto &func : allFunctions)
            {
                string fname;
                for (const auto &fchild : func.children)
                {
                    if (fchild.label.rfind("FunctionName:", 0) == 0)
                    {
                        fname = fchild.label.substr(13);
                        break;
                    }
                }
                if (fname == callee)
                {
                    simulateExecution(func, vars);
                    break;
                }
            }
            trace.push_back({{"action", "return"}, {"function", callee}});
        }
    }
    else
    {
        for (const auto &child : node.children)
        {
            simulateExecution(child, vars);
        }
    }
}

    // --- Main ---

int main()
{
    ifstream file("input.cpp");
    if (!file.is_open())
    {
        cerr << "Failed to open input.cpp\n";
        return 1;
    }
    stringstream buffer;
    buffer << file.rdbuf();
    string code = buffer.str();

    try
    {
        auto tokens = tokenize(code);
        Parser parser(tokens);
        Node tree = parser.parse();
        json output = nodeToJson(tree);

        // Simulate execution starting from main
        for (const auto &func : allFunctions)
        {
            for (const auto &fchild : func.children)
            {
                if (fchild.label == "FunctionName: main")
                {
                    unordered_map<string, int> vars;
                    simulateExecution(func, vars);
                }
            }
        }

        // Write parse tree
        ofstream out("tree.json");
        out << output.dump(4);

        // Write trace
        ofstream traceOut("trace.json");
        traceOut << json(trace).dump(4);

        // Write symbol table
        json symtab = json::array();
        for (const auto &entry : symbolTable)
        {
            json row;
            row["name"] = entry.name;
            row["type"] = entry.type;
            row["scope"] = entry.scope;
            if (entry.hasValue)
                row["value"] = entry.value;
            symtab.push_back(row);
        }
        ofstream symtabOut("symbol_table.json");
        symtabOut << symtab.dump(4);

        cout << "\nParse tree generated and saved to tree.json\n";
        cout << "Execution trace generated and saved to trace.json\n";
        cout << "Symbol table generated and saved to symbol_table.json\n";
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}
