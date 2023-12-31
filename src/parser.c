#include "include/parser.h"
#include "include/scope.h"
#include <stdio.h>
#include <string.h>


parser_T* init_parser(lexer_T* lexer)
{
    parser_T* parser = calloc(1, sizeof(struct PARSER_STRUCT));
    parser->lexer = lexer;
    parser->current_token = lexer_get_next_token(lexer);
    parser->prev_token = parser->current_token;

    parser->scope = init_scope();

    return parser;
}

void parser_eat(parser_T* parser, int token_type)
{
    if (parser->current_token->type == token_type)
    {
        parser->prev_token = parser->current_token;
        parser->current_token = lexer_get_next_token(parser->lexer);
    }
    else
    {
        printf(" וואלה לא צפוי`%s`, בקטע עם  %d",
            parser->current_token->value,
            parser->current_token->type
        );
        exit(1);
    }
}

AST_T* parser_parse(parser_T* parser, scope_T* scope)
{
    return parser_parse_statements(parser, scope);
}

AST_T* parser_parse_statement(parser_T* parser, scope_T* scope)
{
    switch (parser->current_token->type)
    {
        case TOKEN_ID: return parser_parse_id(parser, scope);
    }

    return init_ast(AST_NOOP);
}

AST_T* parser_parse_statements(parser_T* parser, scope_T* scope)
{
    AST_T* compound = init_ast(AST_COMPOUND);
    compound->scope = scope;
    compound->compound_value = calloc(1, sizeof(struct AST_STRUCT*));

    AST_T* ast_statement = parser_parse_statement(parser, scope);
    ast_statement->scope = scope;
    compound->compound_value[0] = ast_statement;
    compound->compound_size += 1;

    while (parser->current_token->type == TOKEN_SEMI)
    {
        parser_eat(parser, TOKEN_SEMI);

        AST_T* ast_statement = parser_parse_statement(parser, scope);

        if (ast_statement)
        {
            compound->compound_size += 1;
            compound->compound_value = realloc(
                compound->compound_value,
                compound->compound_size * sizeof(struct AST_STRUCT*)
            );
            compound->compound_value[compound->compound_size-1] = ast_statement;
        }
    }

    return compound;
}

AST_T* parser_parse_if_statement(parser_T* parser, scope_T* scope)
{
    parser_eat(parser, TOKEN_ID); // Eat the 'if' token
    AST_T* if_statement = init_ast(AST_IF_STATEMENT);
    if_statement->scope = scope;

    // Parse the condition
    if_statement->if_condition = parser_parse_expr(parser, scope);

    // Parse the body of the if block
    parser_eat(parser, TOKEN_LBRACE);
    if_statement->if_body = parser_parse_statements(parser, scope);
    parser_eat(parser, TOKEN_RBRACE);

    return if_statement;
}

AST_T* parser_parse_else_statement(parser_T* parser, scope_T* scope)
{
    parser_eat(parser, TOKEN_ID); // Eat the 'else' token
    AST_T* else_statement = init_ast(AST_ELSE_STATEMENT);
    else_statement->scope = scope;

    // Parse the body of the else block
    parser_eat(parser, TOKEN_LBRACE);
    else_statement->else_body = parser_parse_statements(parser, scope);
    parser_eat(parser, TOKEN_RBRACE);
}


AST_T* parser_parse_while_loop(parser_T* parser, scope_T* scope) {
    AST_T* ast_while_loop = init_ast(AST_WHILE_LOOP);

    parser_eat(parser, TOKEN_ID); // Consume the 'while' token
    parser_eat(parser, TOKEN_LPAREN); // Expect '('

    // Parse condition
    AST_T* condition = parser_parse_expr(parser, scope);
    parser_eat(parser, TOKEN_RPAREN); // Expect ')'

    // Parse the loop body
    AST_T* loop_body = parser_parse_statement(parser, scope);

    ast_while_loop->for_loop_condition = condition;
    ast_while_loop->for_loop_body = loop_body;

    ast_while_loop->scope = scope;

    return ast_while_loop;
}

AST_T* parser_parse_for_loop(parser_T* parser, scope_T* scope) {
    AST_T* ast_for_loop = init_ast(AST_FOR_LOOP);

    parser_eat(parser, TOKEN_ID); // Consume the 'for' token
    parser_eat(parser, TOKEN_LPAREN); // Expect '('

    // Parse initialization
    AST_T* initialization = parser_parse_variable_definition(parser, scope);
    parser_eat(parser, TOKEN_SEMI); // Expect ';'

    // Parse condition
    AST_T* condition = parser_parse_expr(parser, scope);
    parser_eat(parser, TOKEN_SEMI); // Expect ';'

    // Parse increment
    AST_T* increment = parser_parse_expr(parser, scope);
    parser_eat(parser, TOKEN_RPAREN); // Expect ')'

    // Parse the loop body
    AST_T* loop_body = parser_parse_statement(parser, scope);

    ast_for_loop->for_loop_initialization = initialization;
    ast_for_loop->for_loop_condition = condition;
    ast_for_loop->for_loop_increment = increment;
    ast_for_loop->for_loop_body = loop_body;

    ast_for_loop->scope = scope;

    return ast_for_loop;
}

AST_T* parser_parse_expr(parser_T* parser, scope_T* scope)
{
    switch (parser->current_token->type)
    {
        case TOKEN_STRING: return parser_parse_string(parser, scope);
        case TOKEN_ID: return parser_parse_id(parser, scope);
    }

    return init_ast(AST_NOOP);
}

AST_T* parser_parse_factor(parser_T* parser, scope_T* scope)
{
}

AST_T* parser_parse_term(parser_T* parser, scope_T* scope)
{
}

AST_T* parser_parse_function_call(parser_T* parser, scope_T* scope)
{
    AST_T* function_call = init_ast(AST_FUNCTION_CALL);

    function_call->function_call_name = parser->prev_token->value;
    parser_eat(parser, TOKEN_LPAREN); 

    function_call->function_call_arguments = calloc(1, sizeof(struct AST_STRUCT*));

    AST_T* ast_expr = parser_parse_expr(parser, scope);
    function_call->function_call_arguments[0] = ast_expr;
    function_call->function_call_arguments_size += 1;

    while (parser->current_token->type == TOKEN_COMMA)
    {
        parser_eat(parser, TOKEN_COMMA);

        AST_T* ast_expr = parser_parse_expr(parser, scope);
        function_call->function_call_arguments_size += 1;
        function_call->function_call_arguments = realloc(
            function_call->function_call_arguments,
            function_call->function_call_arguments_size * sizeof(struct AST_STRUCT*)
        );
        function_call->function_call_arguments[function_call->function_call_arguments_size-1] = ast_expr;
    }
    parser_eat(parser, TOKEN_RPAREN);

    function_call->scope = scope;

    return function_call;
}

AST_T* parser_parse_variable_definition(parser_T* parser, scope_T* scope)
{
    parser_eat(parser, TOKEN_ID); // var
    char* variable_definition_variable_name = parser->current_token->value;
    parser_eat(parser, TOKEN_ID); // var name
    parser_eat(parser, TOKEN_EQUALS);
    AST_T* variable_definition_value = parser_parse_expr(parser, scope);

    AST_T* variable_definition = init_ast(AST_VARIABLE_DEFINITION);
    variable_definition->variable_definition_variable_name = variable_definition_variable_name;
    variable_definition->variable_definition_value = variable_definition_value;

    variable_definition->scope = scope;

    return variable_definition;
}

AST_T* parser_parse_function_definition(parser_T* parser, scope_T* scope)
{
    AST_T* ast = init_ast(AST_FUNCTION_DEFINITION);
    parser_eat(parser, TOKEN_ID); // function starting

    char* function_name = parser->current_token->value; //function name
    ast->function_definition_name = calloc(
            strlen(function_name) + 1, sizeof(char)
    );
    strcpy(ast->function_definition_name, function_name);

    parser_eat(parser, TOKEN_ID); // function name

    parser_eat(parser, TOKEN_LPAREN);

    ast->function_definition_args =
        calloc(1, sizeof(struct AST_STRUCT*));

    AST_T* arg = parser_parse_variable(parser, scope);
    ast->function_definition_args_size += 1;
    ast->function_definition_args[ast->function_definition_args_size-1] = arg;

    while (parser->current_token->type == TOKEN_COMMA)
    {
        parser_eat(parser, TOKEN_COMMA);

        ast->function_definition_args_size += 1;

        ast->function_definition_args =
            realloc(
                    ast->function_definition_args,
                    ast->function_definition_args_size * sizeof(struct AST_STRUCT*)
                   );

        AST_T* arg = parser_parse_variable(parser, scope);
        ast->function_definition_args[ast->function_definition_args_size-1] = arg;
    }

    parser_eat(parser, TOKEN_RPAREN);

    parser_eat(parser, TOKEN_LBRACE);

    ast->function_definition_body = parser_parse_statements(parser, scope);

    parser_eat(parser, TOKEN_RBRACE);

    ast->scope = scope;

    return ast;
}

AST_T* parser_parse_variable(parser_T* parser, scope_T* scope)
{
    char* token_value = parser->current_token->value;
    parser_eat(parser, TOKEN_ID); // var name or function call name

    if (parser->current_token->type == TOKEN_LPAREN)
        return parser_parse_function_call(parser, scope);

    AST_T* ast_variable = init_ast(AST_VARIABLE);
    ast_variable->variable_name = token_value;

    ast_variable->scope = scope;

    return ast_variable;
}

AST_T* parser_parse_string(parser_T* parser, scope_T* scope)
{
    AST_T* ast_string = init_ast(AST_STRING);
    ast_string->string_value = parser->current_token->value;

    parser_eat(parser, TOKEN_STRING);

    ast_string->scope = scope;

    return ast_string;
}

AST_T* parser_parse_id(parser_T* parser, scope_T* scope)
{
    if (strcmp(parser->current_token->value, "מכריז-בזאת") == 0)
    {
        return parser_parse_variable_definition(parser, scope);
    }    
    else
    if (strcmp(parser->current_token->value, "בראשית") == 0)
    {
        return parser_parse_function_definition(parser, scope);
    }
    else
    if (strcmp(parser->current_token->value, "סעמק") == 0){
      return parser_parse_for_loop(parser, scope);
    }
    else
    if (strcmp(parser->current_token->value, "כוסעמק") == 0){
      return parser_parse_while_loop(parser, scope);
    }
    else
    if (strcmp(parser->current_token->value, "וואלה-מה") == 0){
      return parser_parse_if_statement(parser, scope);
    }
    else
    if (strcmp(parser->current_token->value, "וואלה-לא") == 0){
      return parser_parse_else_statement(parser, scope);
    }
    else
    {
        return parser_parse_variable(parser, scope);
    }
}