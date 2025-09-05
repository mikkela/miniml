grammar MiniML;

prog        : expr EOF ;

expr
    : letExpr
    | ifExpr
    | lamExpr
    | appExpr
    ;

letExpr     : LET ID EQ expr IN expr ;
ifExpr      : IF expr THEN expr ELSE expr ;
lamExpr     : LAMBDA ID ARROW expr ;

appExpr     : atom (atom)+              #AppChain
            | atom                      #JustAtom
            ;

atom        : INT
            | ID
            | LPAREN expr RPAREN
            ;

LET         : 'let' ;
IN          : 'in' ;
IF          : 'if' ;
THEN        : 'then' ;
ELSE        : 'else' ;

LAMBDA      : '\\' ;
ARROW       : '->' ;
LPAREN      : '(' ;
RPAREN      : ')' ;
EQ          : '=' ;

ID          : [a-zA-Z_][a-zA-Z0-9_]* ;
INT         : [0-9]+ ;

WS          : [ \t\r\n]+ -> skip ;
LINE_COMMENT: '//' ~[\r\n]* -> skip ;
