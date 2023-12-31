#include "include/visitor.h"
#include "include/scope.h"
#include <stdio.h>
#include <string.h>


static AST_T* builtin_function_input(visitor_T* visitor, AST_T** args, int args_size)
{
    for (int i = 0; i < args_size; i++)
    {
        AST_T* visited_ast = visitor_visit(visitor, args[i]);

        switch (visited_ast->type)
        {
            case AST_STRING: {
                printf("Enter value for '%s': ", visited_ast->string_value);

                char buffer[100];  // Adjust the buffer size as needed
                if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
                    // Remove newline character if present
                    char* newline = strchr(buffer, '\n');
                    if (newline != NULL) {
                        *newline = '\0';
                    }

                    // Allocate memory for the string value
                    visited_ast->string_value = malloc(strlen(buffer) + 1);
                    strcpy(visited_ast->string_value, buffer);
                } else {
                    printf("Error reading input.\n");
                }
                break;
            }
            default: printf("%p\n", visited_ast); break;
        }
    }

    return init_ast(AST_NOOP);
}


static AST_T* builtin_function_print(visitor_T* visitor, AST_T** args, int args_size)
{
    for (int i = 0; i < args_size; i++)
    {
        AST_T* visited_ast = visitor_visit(visitor, args[i]);

        switch (visited_ast->type)
        {
            case AST_STRING: printf("%s\n", visited_ast->string_value); break;
            default: printf("%p\n", visited_ast); break;
        }
    }
    return init_ast(AST_NOOP);
}

static AST_T* builtin_function_exit(visitor_T* visitor, AST_T** args, int args_size)
{
    for (int i = 0; i < args_size; i++)
    {
        AST_T* visited_ast = visitor_visit(visitor, args[i]);

        switch (visited_ast->type)
        {
            case AST_NOOP: printf("וואלה כפרה לאן יצאת\n"); exit(0); break;
            default: printf("%p\n", visited_ast); break;
        }
    }

    return init_ast(AST_NOOP);
}

static AST_T* builtin_function_clear(visitor_T* visitor, AST_T** args, int args_size)
{
    for (int i = 0; i < args_size; i++)
    {
        AST_T* visited_ast = visitor_visit(visitor, args[i]);

        switch (visited_ast->type)
        {
            case AST_NOOP: system("וואלה ביקשת ניקיון לפסח קיבלת"); break;
            default: printf("%p\n", visited_ast); break;
        }
    }

    return init_ast(AST_NOOP);
}

visitor_T* init_visitor()
{
    visitor_T* visitor = calloc(1, sizeof(struct VISITOR_STRUCT));

    return visitor;
}

AST_T* visitor_visit(visitor_T* visitor, AST_T* node)
{
    switch (node->type)
    {
        case AST_VARIABLE_DEFINITION: return visitor_visit_variable_definition(visitor, node); break;
        case AST_FUNCTION_DEFINITION: return visitor_visit_function_definition(visitor, node); break;
        case AST_VARIABLE: return visitor_visit_variable(visitor, node); break;
        case AST_FUNCTION_CALL: return visitor_visit_function_call(visitor, node); break;
        case AST_STRING: return visitor_visit_string(visitor, node); break;
        case AST_COMPOUND: return visitor_visit_compound(visitor, node); break;
        case AST_NOOP: return node; break;
    }

    printf("המשפט לא כזה תפוס `%d`\n", node->type);
    exit(1);

    return init_ast(AST_NOOP);
}

AST_T* visitor_visit_variable_definition(visitor_T* visitor, AST_T* node)
{
    scope_add_variable_definition(
        node->scope,
        node        
    ); 

    return node;
}

AST_T* visitor_visit_function_definition(visitor_T* visitor, AST_T* node)
{
    scope_add_function_definition(
        node->scope,
        node        
    );

    return node;
}

AST_T* visitor_visit_for(visitor_T* visitor, AST_T* node) {
    if (node->type != AST_FOR_LOOP) {
        printf("וואלה דפקת משהו\n");
        exit(1);
    }


    AST_T* initialization = visitor_visit(visitor, node->for_loop_initialization);
    AST_T* condition = visitor_visit(visitor, node->for_loop_condition);

    while (condition->type == AST_BOOLEAN && condition->boolean_value) {
        visitor_visit(visitor, node->for_loop_body);
        visitor_visit(visitor, node->for_loop_increment);
        condition = visitor_visit(visitor, node->for_loop_condition);
    }

    return init_ast(AST_NOOP);
}

AST_T* visitor_visit_while(visitor_T* visitor, AST_T* node) {
    if (node->type != AST_WHILE_LOOP) {
        printf("וואלה דפקת משהו\n");
        exit(1);
    }


    AST_T* condition = visitor_visit(visitor, node->while_loop_condition);
    AST_T* body = visitor_visit(visitor, node->while_loop_body);

    // Evaluate the condition and execute the loop body as long as the condition is true
    while (condition->type == AST_BOOLEAN && condition->boolean_value) {
        // Visit the body of the while loop
        visitor_visit(visitor, body);

        // Re-evaluate the condition for the next iteration
        condition = visitor_visit(visitor, node->while_loop_condition);
    }

    return init_ast(AST_NOOP);
}

AST_T* visitor_visit_if(visitor_T* visitor, AST_T* node) {
    if (node->type != AST_IF_STATEMENT) {
        printf("וואלה דפקת משהו\n");
        exit(1);
    }

    AST_T* condition = visitor_visit(visitor, node->if_condition);
    AST_T* if_body = visitor_visit(visitor, node->if_body);

    // Logic to handle 'if' statement elements
    if (condition->type == AST_BOOLEAN && condition->boolean_value) {
        visitor_visit(visitor, if_body);
    }

    return init_ast(AST_NOOP);
}

AST_T* visitor_visit_else(visitor_T* visitor, AST_T* node) {
    if (node->type != AST_ELSE_STATEMENT) {
        printf("וואלה דפקת משהו\n");
        exit(1);
    }

    AST_T* else_body = visitor_visit(visitor, node->else_body);

    // Logic to handle 'else' statement elements
    visitor_visit(visitor, else_body);

    return init_ast(AST_NOOP);
}

AST_T* visitor_visit_variable(visitor_T* visitor, AST_T* node)
{
    AST_T* vdef = scope_get_variable_definition(
        node->scope,
        node->variable_name
    );

    if (vdef != (void*) 0)
        return visitor_visit(visitor, vdef->variable_definition_value);

    printf("וואלה הכרזת המדינה לא עבדה משהו `%s`\n", node->variable_name);
    exit(1);
}

AST_T* visitor_visit_function_call(visitor_T* visitor, AST_T* node)
{
    if (strcmp(node->function_call_name, "תצעק") == 0)
    {
        return builtin_function_print(visitor, node->function_call_arguments, node->function_call_arguments_size);
    }

    if (strcmp(node->function_call_name, "די-מספיק") == 0)
    {
        return builtin_function_exit(visitor, node->function_call_arguments, node->function_call_arguments_size);
    }

    if (strcmp(node->function_call_name, "ניקוי-פסח") == 0)
    {
        return builtin_function_clear(visitor, node->function_call_arguments, node->function_call_arguments_size);
    }

    if (strcmp(node->function_call_name, "מה-אמרת") == 0)
    {
        return builtin_function_input(visitor, node->function_call_arguments, node->function_call_arguments_size);
    }

    AST_T* fdef = scope_get_function_definition(
        node->scope,
        node->function_call_name
    );

    if (fdef == (void*)0)
    {
        printf("אחושלוקי לא מוגדר `%s`\n", node->function_call_name);
        exit(1);
    }

    for (int i = 0; i < (int) node->function_call_arguments_size; i++)
    {
        // grab the variable from the function definition arguments
        AST_T* ast_var = (AST_T*) fdef->function_definition_args[i];

        // grab the value from the function call arguments
        AST_T* ast_value = (AST_T*) node->function_call_arguments[i];

        // create a new variable definition with the value of the argument
        // in the function call.
        AST_T* ast_vardef = init_ast(AST_VARIABLE_DEFINITION);
        ast_vardef->variable_definition_value = ast_value;

        // copy the name from the function definition argument into the new
        // variable definition
        ast_vardef->variable_definition_variable_name = (char*) calloc(strlen(ast_var->variable_name) + 1, sizeof(char));
        strcpy(ast_vardef->variable_definition_variable_name, ast_var->variable_name);

        // push our variable definition into the function body scope.
        scope_add_variable_definition(fdef->function_definition_body->scope, ast_vardef);
    }

    return visitor_visit(visitor, fdef->function_definition_body);
}

AST_T* visitor_visit_string(visitor_T* visitor, AST_T* node)
{
    return node;
}

AST_T* visitor_visit_compound(visitor_T* visitor, AST_T* node)
{
    for (int i = 0; i < node->compound_size; i++)
    {
        visitor_visit(visitor, node->compound_value[i]);
    }

    return init_ast(AST_NOOP);
}