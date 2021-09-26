#include <string>
# include <iostream>

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