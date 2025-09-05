grammar MiniML;
// Minimal sketch to expand later.

prog    : expr EOF ;
expr    : INT
        | ID
        | 'let' ID '=' expr 'in' expr
        | '\' ID '->' expr
        | expr expr        #app
        | '(' expr ')'
        ;

ID      : [a-zA-Z_][a-zA-Z0-9_]* ;
INT     : [0-9]+ ;
WS      : [ \t\r\n]+ -> skip ;
