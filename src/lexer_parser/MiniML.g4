grammar MiniML;

prog        : expr EOF ;

expr
    : letExpr
    | ifExpr
    | lamExpr
    | orExpr
    ;

letExpr     : LET ID EQ expr IN expr ;
ifExpr      : IF expr THEN expr ELSE expr ;
lamExpr     : LAMBDA ID ARROW expr ;

orExpr      : andExpr (OR andExpr)* ;
andExpr     : eqExpr (AND eqExpr)* ;
eqExpr      : relExpr ((EQ | NEQ) relExpr)* ;
relExpr     : addExpr ((LT | LE | GT | GE) addExpr)* ;
addExpr     : mulExpr ((PLUS | MINUS) mulExpr)* ;
mulExpr     : appExpr ((STAR | SLASH) appExpr)* ;

appExpr     : atom (atom)+              #AppChain
            | atom                      #JustAtom
            ;

atom        : INT
            | TRUE
            | FALSE
            | NOT atom
            | ID
            | parenExpr
            | tupleLiteral
            ;

parenExpr   : LPAREN expr RPAREN
            ;

tupleLiteral: LPAREN expr COMMA expr (COMMA expr)* RPAREN
            ;

LET         : 'let' ;
IN          : 'in' ;
IF          : 'if' ;
THEN        : 'then' ;
ELSE        : 'else' ;
TRUE        : 'true';
FALSE       : 'false';
LAMBDA      : '\\' ;
ARROW       : '->' ;
NOT         : 'not';

// Operators
OR          : '||';
AND         : '&&';
EQ          : '=';
NEQ         : '<>';
LE          : '<=';
GE          : '>=';
LT          : '<';
GT          : '>';
PLUS        : '+';
MINUS       : '-';
STAR        : '*';
SLASH       : '/';

// Delimiters
LPAREN      : '(' ;
RPAREN      : ')' ;
COMMA       : ',' ;

// Identifiers & literals
ID          : [a-zA-Z_][a-zA-Z0-9_]* ;
INT         : [0-9]+ ;

// Whitespace & comments
WS          : [ \t\r\n]+ -> skip ;
BLOCK_COMMENT
    : '(*' .*? '*)' -> skip
    ;
