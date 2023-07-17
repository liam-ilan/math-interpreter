#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
// #include <emscripten.h>

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

// frees ast
void AstNode_free(AstNode *p_head) {
  // for each child, free memory
  AstNode *p_curr = p_head->p_headChild;
  AstNode *p_tmp = NULL;
  
  while (p_curr != NULL) {
    p_tmp = p_curr;
    p_curr = p_curr->p_next;
    AstNode_free(p_tmp);
  }

  // free self
  free(p_head);
}

// print ast nicely
void AstNode_print(AstNode *p_head, int depth) {

  // print current node
  printf("%s\n", p_head->opCode);

  // for each child
  AstNode *p_curr = p_head->p_headChild;
  while (p_curr != NULL) {
    
    // create appropriate whitespace and dash
    for (int x = 0; x < depth + 1; x++) printf("  ");
    printf("- ");

    // print child
    AstNode_print(p_curr, depth + 1);

    p_curr = p_curr->p_next;
  }
}

// error AST node
AstNode *errorNode(char* errorMessage) {
    // allocate memory
    AstNode *res = (AstNode *)(malloc(sizeof(AstNode)));

    // populate memory
    res->opCode = "err";
    res->val = errorMessage;
    res->p_next = NULL;
    res->p_headChild = NULL;
    
    return res;
}

// parse expression prototype
AstNode *parseExpression(Token *, int);

// parse unary
// EBNF: unary = ("-", unary) | int | float | ("(", expression, ")")
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

  } else if (length > 1 && strcmp(p_head->type, "open") == 0) {
    // case of "(", expression, ")"

    // loop to last token
    Token *p_curr = p_head;
    int i = 0;
    
    while (p_curr->p_next != NULL && i < length - 1) {
      p_curr = p_curr->p_next;
      i++;
    }

    // if last token is close, parse expression
    // if not, this is incomplete brackets, return error.
    if (strcmp(p_curr->type, "close") == 0) return parseExpression(p_head->p_next, length - 2);
    else return errorNode("Syntax Error: Incomplete brackets.\n");

  } else if (length > 0) {
    // error catching for case where there is close brackets where there shouldn't be
    // loop to last token
    Token *p_curr = p_head;
    int i = 0;
    
    while (p_curr->p_next != NULL && i < length - 1) {
      p_curr = p_curr->p_next;
      i++;
    }

    // return appropriate error
    if (strcmp(p_curr->type, "close") == 0) return errorNode("Syntax Error: Incomplete brackets.\n");

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
    } else {
      return errorNode("Syntax Error: Unexpected token, where integer or float was expected.\n");
    }
  }

  return errorNode("Syntax Error: Unidentifiable issue.\n");
}

// parse factor
// EBNF: factor = (factor, ("*" | "/"), unary) | unary;
// length tells us when to stop parsing
AstNode *parseFactor(Token *p_head, int length) {
  // error handling for first token
  if (strcmp(p_head->type, "mult") == 0 || strcmp(p_head->type, "div") == 0) {
    return errorNode("Syntax Error: Binary multiplication or division operator supplied single value.\n");
  }

  // loop through tokens until specified length reached
  // keep track of length of first factor
  // p_sep contains pointer to token seperating the factor and unary
  Token *p_curr = p_head;
  Token *p_sep = NULL;
  int factorLength = 0;
  int i = 0;

  // we must ignore signs in brackets
  int bracketCount = 0;

  while (p_curr != NULL && i < length) {
    if (
      bracketCount == 0 
      && (strcmp(p_curr->type, "mult") == 0 || strcmp(p_curr->type, "div") == 0)
    ) {
      factorLength = i;
      p_sep = p_curr;
    }

    if (strcmp(p_curr->type, "mult") == 0 || strcmp(p_curr->type, "div") == 0) {
      // error handling for last token
      if (p_curr->p_next == NULL || i == length - 1) {
        return errorNode("Syntax Error: Binary multiplication or division operator supplied single value.\n");
      }

      // error handling for consecutive +/-
      if (strcmp(p_curr->p_next->type, "mult") == 0 || strcmp(p_curr->p_next->type, "div") == 0) {
        return errorNode("Syntax Error: Multiple multiplication or division operators in a row.\n");
      }
    }
    // handle bracket count
    if (strcmp(p_curr->type, "open") == 0) bracketCount++;
    else if (strcmp(p_curr->type, "close") == 0) bracketCount--;

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
      res->p_headChild->p_next = parseUnary(p_sep->p_next, length - factorLength - 1);

      return res;

    } else if (strcmp(p_sep->type, "mult") == 0) {

      // mult case
      // allocate memory
      AstNode *res = (AstNode *)(malloc(sizeof(AstNode)));

      // populate memory
      res->opCode = "mult";
      res->val = NULL;
      res->p_next = NULL;
      res->p_headChild = parseFactor(p_head, factorLength);
      res->p_headChild->p_next = parseUnary(p_sep->p_next, length - factorLength - 1);

      return res;
    }
  }

  return errorNode("Syntax Error: Unidentifiable issue.\n");
}

// parse expression
// EBNF: expression = (expression, ("+" | "-"), factor) | factor;
// length tells us when to stop parsing
AstNode *parseExpression(Token *p_head, int length) {
  // error handling for first token
  if (strcmp(p_head->type, "add") == 0) {
    return errorNode("Syntax Error: Binary addition or subtraction operator supplied single value.\n");
  }

  // loop through tokens until specified length reached
  // keep track of length of first expression
  // p_sep contains pointer to token seperating the expression and factor
  // note: unary "-" takes precedent over expression "-"
  // thus we must filter out negations, and only consider (float | int), "-" as a subtraction
  Token *p_curr = p_head;
  Token *p_sep = NULL;
  int expressionLength = 0;
  int i = 0;

  // true when, if the next token is a "-", it can be considered as subtraction, rather than negation
  bool subFlag = false;

  // we must ignore signs in brackets
  int bracketCount = 0;

  while (p_curr != NULL && i < length) {
    if (
      bracketCount == 0
      && (strcmp(p_curr->type, "add") == 0 
      || (strcmp(p_curr->type, "sub") == 0 && subFlag))
    ) {
      expressionLength = i;
      p_sep = p_curr;
    }

    // handle subFlag
    if (
      strcmp(p_curr->type, "int") == 0 
      || strcmp(p_curr->type, "float") == 0 
      || strcmp(p_curr->type, "close") == 0
    ) {
      subFlag = true;
    } else {
      subFlag = false;
    }
    
    if (strcmp(p_curr->type, "add") == 0 || strcmp(p_curr->type, "sub") == 0) {
      // error handling for last token
      if (p_curr->p_next == NULL || i == length - 1) {
        return errorNode("Syntax Error: Binary addition or subtraction operator supplied single value.\n");
      }

      // error handling for consecutive +/-
      if (strcmp(p_curr->p_next->type, "add") == 0 || strcmp(p_curr->p_next->type, "sub") == 0) {
        return errorNode("Syntax Error: Multiple addition or subtraction operators in a row.\n");
      }
    }

    // handle bracket count
    if (strcmp(p_curr->type, "open") == 0) bracketCount++;
    else if (strcmp(p_curr->type, "close") == 0) bracketCount--;
    
    i++;
    p_curr = p_curr->p_next;
  }

  // now p_sep contains add or sub, or NULL
  // expressionLength contains length of expression to parse
  if (p_sep == NULL) {
    
    // case where whole expression is factor
    return parseFactor(p_head, length);
  } else { 
    
    // case where add/sub present
    // test for add/sub
    if (strcmp(p_sep->type, "add") == 0) {
      
      // div case
      // allocate memory
      AstNode *res = (AstNode *)(malloc(sizeof(AstNode)));

      // populate memory
      res->opCode = "add";
      res->val = NULL;
      res->p_next = NULL;
      res->p_headChild = parseExpression(p_head, expressionLength);
      res->p_headChild->p_next = parseFactor(p_sep->p_next, length - expressionLength - 1);

      return res;

    } else if (strcmp(p_sep->type, "sub") == 0) {

      // mult case
      // allocate memory
      AstNode *res = (AstNode *)(malloc(sizeof(AstNode)));

      // populate memory
      res->opCode = "sub";
      res->val = NULL;
      res->p_next = NULL;
      res->p_headChild = parseExpression(p_head, expressionLength);
      res->p_headChild->p_next = parseFactor(p_sep->p_next, length - expressionLength - 1);
    
      return res;
    }
  }

  return errorNode("Syntax Error: Unidentifiable issue.\n");
}

// parse program
// EBNF: program = SOF, expression, EOF
// length tells us when to stop parsing
AstNode *parseProgram(Token *p_head, int length) {
  // no tokens
  if (length == 2) return errorNode("Syntax Error: Empty input.\n");

  // otherwise parse expression
  return parseExpression(p_head->p_next, length - 2);
}

// struct to be returned from execute
typedef struct executeRes {
  bool isInt;
  double val;
} executeRes;

// traverse tree and execute each node
executeRes *execute(AstNode *node) {
  executeRes *res = (executeRes *)(malloc(sizeof(executeRes)));

  if (strcmp(node->opCode, "int") == 0) {
    res->val = atof(node->val);
    res->isInt = true;

  } else if (strcmp(node->opCode, "float") == 0) {
    res->val = atof(node->val);
    res->isInt = false;
    
  } else if (strcmp(node->opCode, "neg") == 0) {
    executeRes *arg1 = execute(node->p_headChild);
    res->val = -arg1->val;
    res->isInt = arg1->isInt;

  } else if (strcmp(node->opCode, "mult") == 0) {
    executeRes *arg1 = execute(node->p_headChild);
    executeRes *arg2 = execute(node->p_headChild->p_next);

    res->val = arg1->val * arg2->val;
    res->isInt = arg1->isInt && arg2->isInt;

  } else if (strcmp(node->opCode, "div") == 0) {
    executeRes *arg1 = execute(node->p_headChild);
    executeRes *arg2 = execute(node->p_headChild->p_next);

    res->val = arg1->val / arg2->val;
    res->isInt = false;

  } else if (strcmp(node->opCode, "add") == 0) {
    executeRes *arg1 = execute(node->p_headChild);
    executeRes *arg2 = execute(node->p_headChild->p_next);

    res->val = arg1->val + arg2->val;
    res->isInt = arg1->isInt && arg2->isInt;

  } else if (strcmp(node->opCode, "sub") == 0) {
    executeRes *arg1 = execute(node->p_headChild);
    executeRes *arg2 = execute(node->p_headChild->p_next);

    res->val = arg1->val - arg2->val;
    res->isInt = arg1->isInt && arg2->isInt;

  } else if (strcmp(node->opCode, "err") == 0) {
    printf("%s", node->val);
    res->val = NAN;
    res->isInt = false;
  } 

  return res;
}

int dub(int i) {
  return i * 2;
}

// runs code given string
void run(char *code, int fileLength) {

  // code label
  printf("CODE\n");

  // print code
  printf("%s\n", code);

  // tokens label
  printf("\nTOKENS\n");

  // token list
  Token headToken = {"SOF", "SOF", NULL};

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
    
    // error check for invalid chars
    if (strchr(" .1234567890/*+-()\0\t\n\r", c) == NULL) {
      printf("Syntax Error: Unexpected char '%c'.\n", c);
      return;
    }

    // if is a digit char
    if(isdigit(c) > 0 || c == '.') {
      // numbers with decimals are floats
      if (c == '.') {
        if (isFloat == false) isFloat = true;
        else {
          printf("Syntax Error: Too many decimal points in a single float.\n");
          return;
        }
      }

      if (isNumber == false) {
        currNumberStart = i;
      }

      isNumber = true;
    } else if (isNumber == true) {
      // reached end of digits, create number token
      char *val = malloc(i - currNumberStart + 1);
      val[i - currNumberStart] = 0;
      strncpy(val, &code[currNumberStart], i - currNumberStart);

      // add token
      Token_push(&headToken, val, isFloat ? "float" : "int");
      tokenCount++;

      // reset flags
      isNumber = false;
      isFloat = false;
    }
    
    // push operator tokens
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
  printf("\nAST\n");

  // parse
  AstNode *p_headAstNode = parseProgram(&headToken, tokenCount);

  // print AST
  AstNode_print(p_headAstNode, 0);

  // execute label
  printf("\nEXECUTE\n");

  // execute
  executeRes *res = execute(p_headAstNode);

  if (res->isInt) {
    printf("%i\n", (int) res->val);
  } else {
    printf("%f\n", res->val);
  }

  // free tokens
  Token *p_tmp = NULL;
  Token *p_curr = headToken.p_next;
  
  while (p_curr != NULL) {
    p_tmp = p_curr;
    p_curr = p_curr->p_next;

    // free val (also malloced if int or float, thus must be freed)
    if(strcmp(p_tmp->type, "int") == 0 || strcmp(p_tmp->type, "float") == 0) {
      free(p_tmp->val);
    }
    
    free(p_tmp);
  }

  // free ast
  AstNode_free(p_headAstNode);

  // free res
  free(res);
}

#ifndef __EMSCRIPTEN__
int main() {
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

  run(code, fileLength);

  free(code);
  return 0;
}
#endif