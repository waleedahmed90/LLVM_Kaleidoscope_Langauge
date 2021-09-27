#include <string>
#include <iostream>
#include <memory>
#include <vector>
// Following is an alternative to using std::[something]

//using namespace std;

enum Token{
    // End of file Token
    tok_eof = -1,

    // commands
    tok_def = -2,
    tok_extern = -3,

    // primary
    tok_identifier = -4,
    tok_number = -5,

};

/* The lexer returns token in range 0-255 if it is an unknown character,
or else, one of the above mentioned tokens*/

/* Now we will define two global variables*/

// whenever the lexer discovers an Identifier string, its is going to save it in
// the following variable

static std::string IdentifierStr;

// whenever the lexer discovers a number, it is going to save it in the following
// variable

static double NumVal;


/* gettok - Return the next token from standard input*/

static int gettok() {
    static int LastChar = ' ';

    // to skip whitespaces
    while(isspace(LastChar)){
        LastChar = getchar();
    }


    // identifiers
    // rule (the first character of an identifier is always an alphabet/letter)
    // the subsequent are letters or numbers
    // [a-zA-Z][a-zA-Z0-9] //PARSING/LEXES AN IDENTIFIER
    if (isalpha(LastChar)) {
        IdentifierStr = LastChar;
        while (isalnum((LastChar = getchar()))){// alnum = alphanumeric
            IdentifierStr += LastChar;
        }

        if (IdentifierStr == "def"){
            return tok_def;
        }
        if (IdentifierStr == "extern"){
            return tok_extern;
        }
        return tok_identifier;
    }

    // for matching numbers
    // won't be perfect as it would process 1.2.344 as a digit too
    
    if (isdigit(LastChar) || LastChar == '.'){
        std::string NumStr;
        do{
            NumStr += LastChar;
            LastChar = getchar();
        } while (isdigit(LastChar) || LastChar =='.');

        NumVal = strtod(NumStr.c_str(), 0);
        return tok_number;
    }

    // handling comments

    if (LastChar == '#'){
        // Comment until the end of the line
        do{
            LastChar = getchar();
        }while(LastChar !=EOF && LastChar != '\n' && LastChar != '\r');
    // we have to do the following part if the previous loop exits for one of 
    // the last two reasons. We would still want to check the next character
    // as the file hasn't ended yet...
        if (LastChar != EOF){
            return gettok();
        }
    }

    // final part to see if the file has ended

    if (LastChar == EOF){
        return tok_eof;
    }
    // other wise just return the ASCII value of the token
    int ThisChar = LastChar;
    LastChar = getchar();
    return ThisChar;
}


// testing

// int main(){

//     while (true){
//         int tok = gettok();
//         std::cout << "got token: " << tok << std::endl;
//     }
//     return 0;
// }


/*
PARSER PART
*/

// Base Class
// EXRPESSIONS
class ExprAST{
public:
    // virtual destructor
    // virtual; because it's subclasses may choose to override
    // / its implementation
    virtual ~ExprAST() {}
};

// Number expression AST (for numerals like "1.0")
// colon (:) means extension

class NumberExprAST : public ExprAST{
    double Val;

// constructor (takes value V) : Initializer list assigns (V) to  double Val
public:
    NumberExprAST(double V) : Val(V) {}
};

// Variable Expression AST
// Expression class for referencing a variable

class VariableExprAST : public ExprAST {
    std::string Name;

public:
    VariableExprAST(const std::string &Name) : Name(Name){}
};


// Binary Expression AST
// Expression class for binary operator.

class BinaryExprAST : public ExprAST {
    // an operator +, -, *, /, <, >
    char Op;
    // Left, Right side of operator
    // unique pointer (smart pointer)
    std::unique_ptr<ExprAST> LHS, RHS;

public:
    BinaryExprAST(char op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS): Op(op), LHS(std::move(LHS)), RHS(std::move(RHS)){}

};


// Call Expression AST
// Expression class for function calls

class CallExprAST : public ExprAST {
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;

public:
    CallExprAST(const std::string &Callee, std::vector<std::unique_ptr<ExprAST>> Args) : Callee(Callee), Args(std::move(Args)) {}

};


// Prototype AST
// This class represents the "prototype" for a function which captures
// /. its namem and its argument names (thus implicitly, the number of
// /. arguments the function takes)


class PrototypeAST {
    std::string Name;
    std::vector<std::string> Args;

public:
    PrototypeAST(const std::string &name, std::vector<std::string> Args) : Name(name), Args(std::move(Args)) {}

    const std::string &getName() const {return Name; }
};

// Function AST
// This class represents the function definition itself

class FunctionAST {
    std::unique_ptr<PrototypeAST> Proto;
    std::unique_ptr<ExprAST> Body;

public:
    FunctionAST(std::unique_ptr<PrototypeAST> Proto, std::unique_ptr<ExprAST> Body): Proto(std::move(Proto)), Body(std::move(Body)) {}

};

// Cur Tok/getNextToken 
// Provide a simple token buffer.
// CurTok is the current token
// getNextToken() reads another token from the lexer and updates CurTok with its results

static int CurTok;
static int getNextToken() {
    CurTok = gettok();
    return CurTok;
}


// Helper Functions
// LogError* 
// For error reporting and handling

std::unique_ptr<ExprAST> LogError(const char *Str) {
  fprintf(stderr, "LogError: %s\n", Str);
  return nullptr;
}

std::unique_ptr<PrototypeAST> LogErrorP(const char *Str) {
  LogError(Str);
  return nullptr;
}


// numberexpr ::=number
// for parsing numbers

static std::unique_ptr<ExprAST> ParseNumberExpr () {
    // auto is used not to write the type of a variable explicitly
    // after auto, Result's data type will be inferred by C++

    auto Result = std::make_unique<NumberExprAST>(NumVal);
    getNextToken();
    return std::move(Result);
}

// parenexpr ::= '(' expression ')'
// for parsing expressions inside parantheses ()

static std::unique_ptr<ExprAST> ParseParenExpr() {
    getNextToken(); //consume '('
    auto V = ParseExpression();

    if (!V) {
        return nullptr;
    }

    if (CurTok == ')') {
        getNextToken(); //consume ')'
        return V;
    } else {
        return LogError("expected ')'");
    }
}


// identifier expression
// ::= identifier
// ::= identifier '(' expression* ')'

static std::unique_ptr<ExprAST> ParseIdentifierOrCallExpr() {
    std::string IdName = IdentifierStr;
    getNextToken(); //consumes the identifier

    //simple variable reference
    if (CurTok == '('){
        getNextToken(); //consumes the open parenthesis
        std::vector<std::unique_ptr<ExprAST>> Args;
        while (true) {
            auto Arg = parseExpression();
            if(Arg) {
                Args.push_back(Arg);
            } else {
                return nullptr;
            }

            if (CurTok == ')') {
                getNextToken(); // consumes the closing parantheses
                break;
            } else if (CurTok == ',') {
                getNextToken(); //consume this comma
                continue;
            }else{
                return LogError("Expected ')' or ',' in argument list");
            }
        }

        return std::make_unique<CallExprAST>(IdName, std::move(Args));
    } else {    
        return std::make_unique<VariableExprAST>(IdName);
    } 
}


// Primary Parser

static std::unique_ptr<ExprAST> ParsePrimary() {
    switch (CurTok) {
        case tok_identifier:
            return ParseIdentifierOrCallExpr();
        case tok_number:
            return ParseNumberExpr();
        case '(':
            return ParseParenExpr();
        default:
            return LogError("unknown token while expecting an expression");
    }
}


// Binary Operator Precedence
// This holds the precedence for each binary operator that is defined

static std::map<char, int> BinopPrecedence;

/// GetTokPrecedence - Get the precedence of the pending binary operator token.
static int GetTokPrecedence() {
  if (!isascii(CurTok))
    return -1;

  // Make sure it's a declared binop.
  int TokPrec = BinopPrecedence[CurTok];
  if (TokPrec <= 0) return -1;
  return TokPrec;
}

int main() {
  // Install standard binary operators.
  // 1 is lowest precedence.
  BinopPrecedence['<'] = 10;
  BinopPrecedence['+'] = 20;
  BinopPrecedence['-'] = 20;
  BinopPrecedence['*'] = 40;  // highest.
  //...
}
