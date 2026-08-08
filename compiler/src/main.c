#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "target/target.h"
#include "backend/na16_codegen.h"
#include "ir/ir.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "parser/token_stream.h"
#include "preprocessor/preprocessor.h"
#include "semantic_analyser/analyser.h"

int main(void) {
    Error code;
    SourceFile file = {0};

    if ((code = source_file_load(&file, "test.c")) != ERROR_OK) return (int)code;

    Lexer lexer;
    Preprocessor preprocessor;
    TokenStream stream;
    Parser parser;

    lexer_init(&lexer, &file);
    if ((code = preprocessor_init(&preprocessor, &lexer)) != ERROR_OK) return (int)code;
    if ((code = token_stream_init(&stream, &preprocessor)) != ERROR_OK) return (int)code;

    /*PPToken token;
    preprocessor_next(&preprocessor, &token, nullptr);
    size_t indent_count = 0;

    while (token.kind != PP_TOKEN_EOF) {
        if (token.start_of_line) {
            fwrite("\n", 1, 1, f);
        }

        if (token.leading_space && token.start_of_line) {
            fwrite("\t", 1, indent_count, f);
        } else if (token.leading_space) {
            fwrite(" ", 1, 1, f);
        }

        switch (token.kind) {
            case PP_TOKEN_IDENTIFIER:
            case PP_TOKEN_NUMBER:
                fwrite(token.data.string, strlen(token.data.string), 1, f);
                break;
            case PP_TOKEN_CHARACTER_CONSTANT:
                fwrite("'", 1, 1, f);
                fwrite(token.data.string, strlen(token.data.string), 1, f);
                fwrite("'", 1, 1, f);
                break;
            case PP_TOKEN_STRING_LITERAL:
                fwrite("\"", 1, 1, f);
                fwrite(token.data.string, strlen(token.data.string), 1, f);
                fwrite("\"", 1, 1, f);
                break;
            case PP_TOKEN_LEFT_BRACKET:
                fwrite("[", 1, 1, f);
                break;
            case PP_TOKEN_RIGHT_BRACKET:
                fwrite("]", 1, 1, f);
                break;
            case PP_TOKEN_LEFT_PAREN:
                fwrite("(", 1, 1, f);
                break;
            case PP_TOKEN_RIGHT_PAREN:
                fwrite(")", 1, 1, f);
                break;
            case PP_TOKEN_LEFT_BRACE:
                fwrite("{", 1, 1, f);
                indent_count++;
                break;
            case PP_TOKEN_RIGHT_BRACE:
                fwrite("}", 1, 1, f);
                indent_count--;
                break;
            case PP_TOKEN_DOT:
                fwrite(".", 1, 1, f);
                break;
            case PP_TOKEN_ARROW:
                fwrite("->", 2, 1, f);
                break;
            case PP_TOKEN_INCREMENT:
                fwrite("++", 2, 1, f);
                break;
            case PP_TOKEN_DECREMENT:
                fwrite("--", 2, 1, f);
                break;
            case PP_TOKEN_AMPERSAND:
                fwrite("&", 1, 1, f);
                break;
            case PP_TOKEN_ASTERISK:
                fwrite("*", 1, 1, f);
                break;
            case PP_TOKEN_PLUS:
                fwrite("+", 1, 1, f);
                break;
            case PP_TOKEN_MINUS:
                fwrite("-", 1, 1, f);
                break;
            case PP_TOKEN_TILDE:
                fwrite("~", 1, 1, f);
                break;
            case PP_TOKEN_EXCLAMATION:
                fwrite("!", 1, 1, f);
                break;
            case PP_TOKEN_SLASH:
                fwrite("/", 1, 1, f);
                break;
            case PP_TOKEN_PERCENT:
                fwrite("%", 1, 1, f);
                break;
            case PP_TOKEN_SHIFT_LEFT:
                fwrite("<<", 2, 1, f);
                break;
            case PP_TOKEN_SHIFT_RIGHT:
                fwrite(">>", 2, 1, f);
                break;
            case PP_TOKEN_LESS:
                fwrite("<", 1, 1, f);
                break;
            case PP_TOKEN_GREATER:
                fwrite(">", 1, 1, f);
                break;
            case PP_TOKEN_LESS_EQUAL:
                fwrite("<=", 2, 1, f);
                break;
            case PP_TOKEN_GREATER_EQUAL:
                fwrite(">=", 2, 1, f);
                break;
            case PP_TOKEN_EQUAL_EQUAL:
                fwrite("==", 2, 1, f);
                break;
            case PP_TOKEN_NOT_EQUAL:
                fwrite("!=", 2, 1, f);
                break;
            case PP_TOKEN_CARET:
                fwrite("^", 1, 1, f);
                break;
            case PP_TOKEN_PIPE:
                fwrite("|", 1, 1, f);
                break;
            case PP_TOKEN_LOGICAL_AND:
                fwrite("&&", 2, 1, f);
                break;
            case PP_TOKEN_LOGICAL_OR:
                fwrite("||", 2, 1, f);
                break;
            case PP_TOKEN_QUESTION:
                fwrite("?", 1, 1, f);
                break;
            case PP_TOKEN_COLON:
                fwrite(":", 1, 1, f);
                break;
            case PP_TOKEN_SEMICOLON:
                fwrite(";", 1, 1, f);
                break;
            case PP_TOKEN_ELLIPSIS:
                fwrite("...", 3, 1, f);
                break;
            case PP_TOKEN_ASSIGN:
                fwrite("=", 1, 1, f);
                break;
            case PP_TOKEN_MULTIPLY_ASSIGN:
                fwrite("*=", 2, 1, f);
                break;
            case PP_TOKEN_DIVIDE_ASSIGN:
                fwrite("/=", 2, 1, f);
                break;
            case PP_TOKEN_REMAINDER_ASSIGN:
                fwrite("%=", 2, 1, f);
                break;
            case PP_TOKEN_ADD_ASSIGN:
                fwrite("+=", 2, 1, f);
                break;
            case PP_TOKEN_SUBTRACT_ASSIGN:
                fwrite("-=", 2, 1, f);
                break;
            case PP_TOKEN_SHIFT_LEFT_ASSIGN:
                fwrite("<<=", 3, 1, f);
                break;
            case PP_TOKEN_SHIFT_RIGHT_ASSIGN:
                fwrite(">>=", 3, 1, f);
                break;
            case PP_TOKEN_AND_ASSIGN:
                fwrite("&=", 2, 1, f);
                break;
            case PP_TOKEN_XOR_ASSIGN:
                fwrite("^=", 2, 1, f);
                break;
            case PP_TOKEN_OR_ASSIGN:
                fwrite("|=", 2, 1, f);
                break;
            case PP_TOKEN_COMMA:
                fwrite(",", 1, 1, f);
                break;
            case PP_TOKEN_HASH:
                fwrite("#", 1, 1, f);
                break;
            case PP_TOKEN_HASH_HASH:
                fwrite("##", 1, 1, f);
                break;
            default:
                break;
        }

        preprocessor_next(&preprocessor, &token, nullptr);
    }*/

    parser_init(&parser, &stream);

    TranslationUnit unit;

    translation_unit_init(&unit);

    if ((code = parser_parse_translation_unit(&parser, &unit)) != ERROR_OK) return (int)code;

    SemanticContext semantic_context = {
        .target = target_na16(),
        .current_function_return_type = nullptr
    };

    analyse_function_definition(&semantic_context ,&unit.items[0].data.function_definition);

    IRModule module = {
        .function_capacity = 8,
        .function_count = 0
    };

    module.functions = malloc(sizeof(IRFunction) * module.function_capacity);

    lower_ast(&unit, &module);

    {
        FILE *output = fopen("a.asm", "w");
        na16_emit_module(output, &module, target_na16());
        fclose(output);
    }

    translation_unit_destroy(&unit);
    source_file_destroy(&file);

    return EXIT_SUCCESS;
}
