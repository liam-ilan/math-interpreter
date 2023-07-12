#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

// token
// linked list element containing pointer to next token, type of token, and value
typedef struct Token {
  char *val;
  char *type;
  struct Token *p_next;
} Token;

// print token list
void Token_print(Token *p_head) {
  Token *p_curr = p_head;
  while (p_curr != NULL) {
    printf("%s: %s\n", p_curr->type, p_curr->val);
    p_curr = p_curr->p_next;
  }
}

// add token to list
void Token_push(Token *p_head, char *val, char* type) {
    // loop to end of list
    Token *p_curr = p_head;
    while (p_curr->p_next != NULL) {
      p_curr = p_curr->p_next;
    }

    // allocate memory for new token
    Token *p_newToken = (Token *)(malloc(sizeof(Token)));

    // add new node to end of list
    p_curr->p_next = p_newToken;

    // write data
    p_curr->p_next->val = val;
    p_curr->p_next->type = type;
    p_curr->p_next->p_next = NULL;
}

// node in ast
// each node is an element in a linked list of it's siblings
// additionally, each node contains a pointer to a "head" child node (may be null)
// each node has an opCode to designate an opperation when the tree is traversed afterwards (string)
// each node has a val (string)
typedef struct AstNode {
  struct AstNode *p_headChild;
  struct AstNode *p_next;
  char *opCode;
  char *val;
} AstNode;

// print ast nicely
void AstNode_print(AstNode *p_head, int depth) {

  // print current node
  printf("%s\n", p_head->opCode);

  // for each child
  AstNode *p_curr = p_head->p_headChild;
  while (p_curr != NULL) {
    
    // create appropriate whitespace and dash
    for (int x = 0; x < depth; x++) printf("  ");
    printf("- ");

    // print child
    AstNode_print(p_curr, depth + 1);

    p_curr = p_curr->p_next;
  }
}

// parse unary
// EBNF: unary = ("-", unary) | int | float
// length tells us when to stop parsing
AstNode *parseUnary(Token *p_head, int length) {
  
  if (length > 1 && strcmp(p_head->type, "sub") == 0) {
    
    // case of "-", unary
    // allocate memory
    AstNode *res = (AstNode *)(malloc(sizeof(AstNode)));

    // populate memory
    res->opCode = "neg";
    res->val = NULL;
    res->p_next = NULL;
    res->p_headChild = parseUnary(p_head->p_next, length - 1);
    
    return res;
  } else if (length > 0) {
    
    // float and int cases
    if (strcmp(p_head->type, "int") == 0) {

      // int case
      // allocate memory
      AstNode *res = (AstNode *)(malloc(sizeof(AstNode)));

      // populate memory
      res->opCode = "int";
      res->val = p_head->val;
      res->p_next = NULL;
      res->p_headChild = NULL;

      return res;

    } else if (strcmp(p_head->type, "float") == 0) {

      // float case
      // allocate memory
      AstNode *res = (AstNode *)(malloc(sizeof(AstNode)));

      // populate memory
      res->opCode = "float";
      res->val = p_head->val;
      res->p_next = NULL;
      res->p_headChild = NULL;

      return res;
    }
  }
}

// parse factor
// EBNF: factor = (factor, ("*" | "/"), unary) | unary;
// length tells us when to stop parsing
AstNode *parseFactor(Token *p_head, int length) {
  
  // loop through tokens until specified length reached
  // keep track of length of first factor
  // p_sep contains pointer to token seperating
  Token *p_curr = p_head;
  Token *p_sep = NULL;
  int factorLength = 0;
  int i = 0;

  while (p_curr != NULL && i < length) {
    if (strcmp(p_curr->type, "mult") == 0 || strcmp(p_curr->type, "div") == 0) {
      factorLength = i;
      p_sep = p_curr;
    };
    
    p_curr = p_curr->p_next;
    i++;
  }

  // now p_sep contains mult or div, or NULL
  // factorLength contains length of factor to parse
  if (p_sep == NULL) {
    // case where whole expression is unary
    return parseUnary(p_head, length);
  } else { 
    
    // case where factor present
    // test for division/multiplication
    if (strcmp(p_sep->type, "div") == 0) {
      
      // div case
      // allocate memory
      AstNode *res = (AstNode *)(malloc(sizeof(AstNode)));

      // populate memory
      res->opCode = "div";
      res->val = NULL;
      res->p_next = NULL;
      res->p_headChild = parseFactor(p_head, factorLength);
      res->p_headChild->p_next = parseUnary(p_sep->p_next, length - factorLength);

      return res;

    } else if (strcmp(p_sep->type, "mult") == 0) {

      // mult case
      // allocate memory
      AstNode *res = (AstNode *)(malloc(sizeof(AstNode)));

      // populate memory
      res->opCode = "mult";
      res->val = NULL;
      res->p_next = NULL;
      printf("%s: %i\n", p_sep->type, factorLength);
      res->p_headChild = parseFactor(p_head, factorLength);
      
      res->p_headChild->p_next = parseUnary(p_sep->p_next, length - factorLength);

      return res;
    }
  }
}

// parse expression
// EBNF: expression = (factor, ("+" | "-"), expression) | factor;
// length tells us when to stop parsing
AstNode *parseExpression(Token *p_head, int length) {
  
  // loop until "+" or "-" found
  // note: unary "-" takes precedent over expression "-"
  // thus we must filter out negations, and only consider (float | int), "-" as a subtraction
  Token *p_curr = p_head;
  int factorLength = 0;

  // true when, if the next token is a "-", it can be considered as subtraction, rather than negation
  bool subFlag = false;

  while (
    p_curr != NULL
    && strcmp(p_curr->type, "add") != 0
    && !(strcmp(p_curr->type, "sub") == 0 && subFlag)
    && factorLength <= length
  ) {
    // handle subFlag
    if (strcmp(p_curr->type, "int") == 0 || strcmp(p_curr->type, "float") == 0) {
      subFlag = true;
    } else {
      subFlag = false;
    }
    factorLength++;
    p_curr = p_curr->p_next;
  }

  // now p_curr contains either NULL, or token containing add/sub symbol
  // factorLength contains the length of the first factor
  if (factorLength == length + 1) {
    
    // case where whole expression is factor
    return parseFactor(p_head, length);
  } else { 
    
    // case where add/sub present
    // test for add/sub
    if (strcmp(p_curr->type, "add") == 0) {
      
      // div case
      // allocate memory
      AstNode *res = (AstNode *)(malloc(sizeof(AstNode)));

      // populate memory
      res->opCode = "add";
      res->val = NULL;
      res->p_next = NULL;
      res->p_headChild = parseFactor(p_head, factorLength);
      res->p_headChild->p_next = parseExpression(p_curr->p_next, length - factorLength - 1);
      

      return res;

    } else if (strcmp(p_curr->type, "sub") == 0) {

      // mult case
      // allocate memory
      AstNode *res = (AstNode *)(malloc(sizeof(AstNode)));

      // populate memory
      res->opCode = "sub";
      res->val = NULL;
      res->p_next = NULL;
      res->p_headChild = parseFactor(p_head, factorLength);
      res->p_headChild->p_next = parseExpression(p_curr->p_next, length - factorLength - 1);

      return res;
    }
  }
}

// parse program
// EBNF: program = SOF, expression, EOF
// length tells us when to stop parsing
AstNode *parseProgram(Token *p_head, int length) {
  return parseExpression(p_head->p_next, length - 2);
}

double execute(AstNode *node) {
  if (strcmp(node->opCode, "int") == 0) {
    return atof(node->val);
  } else if (strcmp(node->opCode, "float") == 0) {
    return atof(node->val);
  } else if (strcmp(node->opCode, "neg") == 0) {
    return -execute(node->p_headChild);
  } else if (strcmp(node->opCode, "mult") == 0) {
    return execute(node->p_headChild) * execute(node->p_headChild->p_next);
  } else if (strcmp(node->opCode, "div") == 0) {
    return execute(node->p_headChild) / execute(node->p_headChild->p_next);
  } else if (strcmp(node->opCode, "add") == 0) {
    return execute(node->p_headChild) + execute(node->p_headChild->p_next);
  } else if (strcmp(node->opCode, "sub") == 0) {
    return execute(node->p_headChild) - execute(node->p_headChild->p_next);
  }
}

int main() {

  // code label
  printf("\n\e[4m\e[1mCODE\e[0m\n");

  // open file
  FILE *p_file = fopen("code.txt", "r");

  // go to end, and record position (this will be the length of the file)
  fseek(p_file, 0, SEEK_END);
  long fileLength = ftell(p_file);

  // rewind to start
  rewind(p_file);

  // allocate memory (+1 for 0 terminated string)
  char *code = malloc(fileLength + 1);

  // read file and close
  fread(code, fileLength, 1, p_file);
  fclose(p_file);

  // set terminator to 0
  code[fileLength] = 0;

  // print code
  printf("%s\n", code);

  // tokens label
  printf("\n\e[4m\e[1mTOKENS\e[0m\n");

  // token list
  Token headToken = {"SOF", "SOF", NULL};
  Token *p_prevToken = &headToken;

  // lexing flags
  bool isNumber = false;
  bool isFloat = false;

  // current scanned number starting index
  int currNumberStart = 0;

  // token count
  int tokenCount = 1;

  // for each char (including terminator)
  for (int i = 0; i <= fileLength; i++) {
    char c = code[i];

    // if is a digit char
    if(isdigit(c) > 0 || c == '.') {
      if (c == '.') {
        isFloat = true;
      }

      if (isNumber == false) {
        currNumberStart = i;
      }

      isNumber = true;
    } else if (isNumber == true) {

      char *val = malloc(i - currNumberStart + 1);
      val[i - currNumberStart] = 0;
      strncpy(val, &code[currNumberStart], i - currNumberStart);

      Token_push(&headToken, val, isFloat ? "float" : "int");
      tokenCount++;

      isNumber = false;
      isFloat = false;
    }

    if (c == '+') {
      Token_push(&headToken, "+", "add"); 
      tokenCount++;
    } else if (c == '/') {
      Token_push(&headToken, "/", "div"); 
      tokenCount++;
    } else if (c == '-') {
      Token_push(&headToken, "-", "sub"); 
      tokenCount++;
    } else if (c == '*') {
      Token_push(&headToken, "*", "mult"); 
      tokenCount++;
    } else if (c == '%') {
      Token_push(&headToken, "\%", "mod");
      tokenCount++;
    } else if (c == '(') {
      Token_push(&headToken, "(", "open");
      tokenCount++;
    } else if (c == ')') {
      Token_push(&headToken, ")", "close");
      tokenCount++;
    }
  }

  Token_push(&headToken, "EOF", "EOF"); 
  tokenCount++;

  // print tokens
  Token_print(&headToken);

  // ast label
  printf("\n\e[4m\e[1mAST\e[0m\n");

  // parse
  AstNode *p_headAstNode = parseProgram(&headToken, tokenCount);

  // print AST
  AstNode_print(p_headAstNode, 0);

  // execute label
  printf("\n\e[4m\e[1mEXECUTE\e[0m\n");
  printf("%f\n", execute(p_headAstNode));
  return 0;
}