/// local
#define ARENA_IMPLEMENTATION
#define ALLOCATOR_IMPLEMENTATION
#define ASCII_IMPLEMENTATION
#define STRINGS_IMPLEMENTATION
#define STRINGS_BUILDER_IMPLEMENTATION
#define INTERN_IMPLEMENTATION

#include "mem/allocator.h"
#include "mem/arena.h"
#include "strings.h"
#include "intern.h"

#include "types/types.h"
#include "types/lexer.h"

/// standard
#include <string.h>
#include <setjmp.h>
#include <stdarg.h>

typedef struct CType_Expr CType_Expr;
struct CType_Expr {
    CType               type;
    CType_QualifierFlag qualifiers;
    CType_BasicFlag     flags;
    jmp_buf             caller;
};

__attribute__((format (printf, 2, 3), noreturn))
static void
expr_throw(CType_Expr *expr, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    if (fmt != NULL) {
        fputs("[ERROR]: ", stdout);
        vfprintf(stdout, fmt, args);
        putc('\n', stdout);
    }
    va_end(args);
    longjmp(expr->caller, 1);
}

/**
 * @note
 *      This function is an absolute mess! But it *does* work.
 */
static void
expr_set_basic(CType_Expr *expr, CType_BasicKind kind)
{
    // We don't call this function with `CType_BasicKind_Long_Long` because 'long long'
    // is not a single lexeme. The same is true for `CType_BasicKind_Long_Double`.
    assert(kind == CType_BasicKind_Bool
        || (CType_BasicKind_Char <= kind && kind <= CType_BasicKind_Long)
        || (CType_BasicKind_Float <= kind && kind <= CType_BasicKind_Double)
        || kind == CType_BasicKind_Void);

    CType type = expr->type;
    // Have a type but it's not a basic one?
    if (type.kind != CType_Kind_Invalid && type.kind != CType_Kind_Basic) have_nonbasic_type: {
        expr_throw(expr, "Cannot assign '%s' to '%s'",
            ctype_basic_types[kind].basic.name.data,
            ctype_kind_strings[type.kind].data);
    }

    // Haven't yet set a type?
    if (type.basic.kind == CType_Kind_Invalid) {
        type.basic.kind = kind;
        goto good_combination;
    }

    // Assumed to already have a basic type.
    switch (kind) {
    case CType_BasicKind_Bool:
        goto have_nonbasic_type;

    case CType_BasicKind_Short:
        // Allow `int short`.
        if (type.basic.kind == CType_BasicKind_Int)
            type.basic.kind = CType_BasicKind_Short;
        else
            goto bad_combination;
        break;

    case CType_BasicKind_Int:
        // Allow `short int`, `long int` and `long long int`. Nothing to change.
        // Implicitly allows `long int long`.
        if (type.basic.kind == CType_BasicKind_Short
            || type.basic.kind == CType_BasicKind_Long
            || type.basic.kind == CType_BasicKind_Long_Long)
            break;
        goto bad_combination;

    case CType_BasicKind_Long:
        // Allow `int long`.
        if (type.basic.kind == CType_BasicKind_Int)
            type.basic.kind = CType_BasicKind_Long;
        // Allow `long long`.
        else if (type.basic.kind == CType_BasicKind_Long)
            type.basic.kind = CType_BasicKind_Long_Long;
        // Allow `long double`.
        else if (type.basic.kind == CType_BasicKind_Double)
            type.basic.kind = CType_BasicKind_Long_Double;
        else
            goto bad_combination;
        break;

    case CType_BasicKind_Long_Long:
        // Allow `int long long`.
        if (type.basic.kind == CType_BasicKind_Int)
            type.basic.kind = CType_BasicKind_Long_Long;
        break;

    case CType_BasicKind_Double:
        // Allow `long double`.
        if (type.basic.kind == CType_BasicKind_Long)
            type.basic.kind = CType_BasicKind_Long_Double;
        else
            goto bad_combination;
        break;

    case CType_BasicKind_Void:
        goto have_nonbasic_type;

    default: bad_combination:
        expr_throw(expr, "Cannot combine '%s' with '%s'",
            type.basic.name.data,
            ctype_basic_types[kind].basic.name.data);
        return;
    }

    good_combination:

    // Implicitly sets `expr->type.basic.kind`.
    expr->type = ctype_basic_types[type.basic.kind];
    printfln("%s: '%s'",
        ctype_kind_strings[expr->type.kind].data,
        expr->type.basic.name.data);
}

/**
 * @note
 *      `modifier` cannot be a combination of flags.
 */
static void
expr_set_modifier(CType_Expr *expr, CType_BasicFlag modifier)
{
    assert(modifier == CType_BasicFlag_Signed
        || modifier == CType_BasicFlag_Unsigned
        || modifier == CType_BasicFlag_Complex);

    const char *name;
    switch (modifier) {
    case CType_BasicFlag_Signed:   name = "signed";   break;
    case CType_BasicFlag_Unsigned: name = "unsigned"; break;
    case CType_BasicFlag_Complex:  name = "complex";  break;

    default:
        assert(false);
    }

    // Modifier is already set to something?
    // Disallows `signed signed`, `signed unsigned`, etc.
    if (expr->flags & (CType_BasicFlag_Signed | CType_BasicFlag_Unsigned | CType_BasicFlag_Complex)) {
        // Perhaps there's a better way to do this...
        const char *prev_name = (expr->flags & CType_BasicFlag_Signed)
                ? "signed"
                : (expr->flags & CType_BasicFlag_Unsigned)
                    ? "unsigned"
                    : (expr->flags & CType_BasicFlag_Complex)
                        ? "complex"
                        : "<invalid>";

        expr_throw(expr, "Cannot combine modifiers '%s' and '%s'", prev_name, name);
    }

    expr->flags |= modifier;
    printfln("CType_BasicFlag: '%s'", name);
}

/**
 * @note
 *      `qualifier` cannot be a combination of flags.
 */
static void
expr_set_qualifier(CType_Expr *expr, CType_QualifierFlag qualifier)
{
    assert(qualifier == CType_QualifierFlag_Const
        || qualifier == CType_QualifierFlag_Volatile
        || qualifier == CType_QualifierFlag_Restrict);

    const char *name;
    switch (qualifier) {
    case CType_QualifierFlag_Const:      name = "const";     break;
    case CType_QualifierFlag_Volatile:   name = "volatile";  break;
    case CType_QualifierFlag_Restrict:   name = "restrict";  break;

    default:
        assert(false);
    }

    // Qualifier is already set?
    // NOTE: 'const const int' is valid in C99, but I dislike it.
    if (expr->qualifiers & qualifier)
        expr_throw(expr, "Duplicate qualifier '%s'", name);
    expr->qualifiers |= qualifier;
    printfln("CType_QualifierFlag: '%s'", name);
}

static void
expr_print(const CType_Expr *expr)
{
    CType type = expr->type;
    printf("Expr: %s = ", ctype_kind_strings[type.kind].data);

    // Qualifiers
    if (expr->qualifiers & CType_QualifierFlag_Const)
        printf("const ");
    if (expr->qualifiers & CType_QualifierFlag_Volatile)
        printf("volatile ");
    if (expr->qualifiers & CType_QualifierFlag_Restrict)
        printf("restrict ");

    // No need to print the modifiers since they're already part of the basic types.
    switch (type.kind) {
    case CType_Kind_Invalid:
        println("<invalid>");
        break;
    case CType_Kind_Basic:
        printfln("%s", type.basic.name.data);
        break;
    case CType_Kind_Pointer:
    case CType_Kind_Struct:
    case CType_Kind_Enum:
    case CType_Kind_Union:
    default:
        println("<unimplemented>");
        break;
    }
}

/**
 * @note
 *      Assumes we don't have duplicate types, modifiers and qualifiers.
 *      Also assumes that we have at most 1 modifier.
 *      We should have thrown errors for those beforehand.
 */
static void
expr_check_semantics(CType_Expr *expr)
{
    // Use a pointer to sync w/ changes.
    CType *type = &expr->type;

    // No type was given?
    if (type->kind == CType_Kind_Invalid) {
        // `signed` resolves to `int`.
        if (expr->flags & CType_BasicFlag_Signed)
            *type = ctype_basic_types[CType_BasicKind_Int];
        // `unsigned` resolves to `unsigned int`.
        else if (expr->flags & CType_BasicFlag_Unsigned)
            *type = ctype_basic_types[CType_BasicKind_Unsigned_Int];
        // `complex` resolves to `complex double`.
        // TODO: allow `long complex` to resolve to `complex long double`.
        else if (expr->flags & CType_BasicFlag_Complex)
            *type = ctype_basic_types[CType_BasicKind_Complex_Double];
        else
            expr_throw(expr, "No base type received.");
    }

    // Modifier or a qualifier name for error messages
    const char *name = NULL;

    // Ensure modifiers have correct usage
    if (expr->flags & CType_BasicFlag_Signed) {
        name = "signed";
        if (!(type->basic.flags & CType_BasicFlag_Integer))
            goto bad_combination;

        // All other integer types are already signed.
        if (type->basic.kind == CType_BasicKind_Char)
            *type = ctype_basic_types[CType_BasicKind_Signed_Char];

    } else if (expr->flags & CType_BasicFlag_Unsigned) {
        name = "unsigned";
        if (!(type->basic.flags & CType_BasicFlag_Integer))
            goto bad_combination;

        // Ensure we have the correct basic type data.
        switch (type->basic.kind) {
        case CType_BasicKind_Char:
            *type = ctype_basic_types[CType_BasicKind_Unsigned_Char];
            break;
        case CType_BasicKind_Short:
            *type = ctype_basic_types[CType_BasicKind_Unsigned_Short];
            break;
        case CType_BasicKind_Int:
            *type = ctype_basic_types[CType_BasicKind_Unsigned_Int];
            break;
        case CType_BasicKind_Long:
            *type = ctype_basic_types[CType_BasicKind_Unsigned_Long];
            break;
        case CType_BasicKind_Long_Long:
            *type = ctype_basic_types[CType_BasicKind_Unsigned_Long_Long];
            break;

        // Unreachable.
        default:
            break;
        }

    } else if (expr->flags & CType_BasicFlag_Complex) {
        name = "complex";
        if (!(type->basic.flags & CType_BasicFlag_Float))
            goto bad_combination;

        // Ensure we have the correct basic type data.
        switch (type->basic.kind) {
        case CType_BasicKind_Float:
            expr->type = ctype_basic_types[CType_BasicKind_Complex_Float];
            break;
        case CType_BasicKind_Double:
            expr->type = ctype_basic_types[CType_BasicKind_Complex_Double];
            break;
        case CType_BasicKind_Long_Double:
            expr->type = ctype_basic_types[CType_BasicKind_Complex_Long_Double];
            break;

        // Unreachable.
        default:
            break;
        }
    }

    // Finally, check the qualifiers. We only really care about 'restrict'.
    if (expr->qualifiers & CType_QualifierFlag_Restrict) {
        name = "restrict";
        if (type->kind != CType_Kind_Pointer) bad_combination: {
            // NOTE: Cannot guarantee we can use `type->basic`!
            expr_throw(expr, "Cannot use '%s' with '%s'", name, type->basic.name.data);
        }
    }
}

static void
parse(CType_Expr *expr, CLexer *lexer)
{
    CToken token;

    // TODO: refactor into an actual loop
    loop_start:

    token = clexer_scan(lexer);
    if (!token.type) {
        expr_throw(expr, "Invalid token " STRING_QFMTSPEC ".", string_fmtarg(token.word));
    } else if (token.type == CTokenType_Eof) {
        expr_check_semantics(expr);
        return;
    }

    switch (token.type) {
    // Boolean
    case CTokenType_Bool:   expr_set_basic(expr, CType_BasicKind_Bool);  break;
    // Integer Types
    case CTokenType_Char:   expr_set_basic(expr, CType_BasicKind_Char);  break;
    case CTokenType_Short:  expr_set_basic(expr, CType_BasicKind_Short); break;
    case CTokenType_Int:    expr_set_basic(expr, CType_BasicKind_Int);   break;
    case CTokenType_Long:   expr_set_basic(expr, CType_BasicKind_Long);  break;

    // Floating-Point Types
    case CTokenType_Float:  expr_set_basic(expr, CType_BasicKind_Float); break;
    case CTokenType_Double: expr_set_basic(expr, CType_BasicKind_Double); break;


    // User-defined types
    case CTokenType_Struct:
    case CTokenType_Enum:
    case CTokenType_Union:
    case CTokenType_Ident:  goto unsupported_token;

    // Modifiers
    case CTokenType_Signed:    expr_set_modifier(expr, CType_BasicFlag_Signed);   break;
    case CTokenType_Unsigned:  expr_set_modifier(expr, CType_BasicFlag_Unsigned); break;
    case CTokenType_Complex:   expr_set_modifier(expr, CType_BasicFlag_Complex);  break;

    // Qualifiers
    case CTokenType_Const:     expr_set_qualifier(expr, CType_QualifierFlag_Const);    break;
    case CTokenType_Volatile:  expr_set_qualifier(expr, CType_QualifierFlag_Volatile); break;
    case CTokenType_Restrict:  expr_set_qualifier(expr, CType_QualifierFlag_Restrict); break;

    // Misc.
    case CTokenType_Void:  expr_set_basic(expr, CType_BasicKind_Void); break;
    case CTokenType_Asterisk:
        goto unsupported_token;

    default: unsupported_token:
        expr_throw(expr, STRING_QFMTSPEC " ('%s') is unsupported!",
            string_fmtarg(token.word),
            ctoken_strings[token.type].data);
        break;
    }

    goto loop_start;
}

static Arena arena;

static void
run_interactive(Arena *arena, CType_Table *table)
{
    Allocator temp_allocator = arena_allocator(arena);
    char buf[256];
    for (;;) {
        fputs(">>> ", stdout);
        if (!fgets(buf, cast(int)sizeof(buf), stdin)) {
            fputc('\n', stdout);
            break;
        }

        CLexer      lexer = clexer_make(buf, strcspn(buf, "\r\n"));
        CType_Expr  expr  = {
            .type       = ctype_basic_types[CType_BasicKind_Invalid],
            .qualifiers = 0,
            .flags      = 0,
        };
        println("=== TOKENS ===");
        if (setjmp(expr.caller) == 0) {
            parse(&expr, &lexer);
            expr_print(&expr);

            // WARNING: Assumes a lot about `expr.type`!
            const CType_Info *info = ctype_table_get_basic_qual(table, expr.type.basic.kind, expr.qualifiers);
            if (info) {
                printfln("Found info @ %p", cast(void *)info);
            } else {
                println("First time seeing info. Adding...");
                info = ctype_table_add_basic_qual(table, expr.type.basic.kind, expr.qualifiers);
                if (info)
                    printfln("Found info @ %p", cast(void *)info);
                else
                    println("Could not add info!");
            }
        }
        println("==============\n");

        size_t total;
        size_t used  = arena_get_usage(arena, &total);
        printfln(
            "=== ARENA INFO ===\n"
            "Begin: %p\n"
            "End:   %p\n"
            "Usage: %zu bytes (out of %zu)\n"
            "==================\n",
            cast(void *)arena->begin,
            cast(void *)arena->end,
            used, total);
        mem_free_all(temp_allocator);
    }
}

int
main(void)
{
    Allocator_Error error = arena_init(&arena);
    if (error)
        return 1;

    CType_Table table;
    error = ctype_table_init(&table, GLOBAL_PANIC_ALLOCATOR);
    if (error)
        return 1;

    // testing for alignment
    {
        char *ch = arena_rawalloc(&arena, sizeof(char), alignof(char));
        *ch = 'a';
        printfln("char *ch: %p; *ch = '%c'", cast(void *)ch, *ch);

        int *i  = arena_rawalloc(&arena, sizeof(int), alignof(int));
        *i = 23;
        printfln("int *i: %p; *i = %i", cast(void *)i, *i);

        // `p` points to a `void *`.
        void **p = arena_rawalloc(&arena, sizeof(void *), alignof(void *));
        *p = ch;
        printfln("void **p: %p; *p = %p\n", cast(void *)p, *p);

        arena_free_all(&arena);
    }

    run_interactive(&arena, &table);
    arena_destroy(&arena);
    ctype_table_destroy(&table);
    return 0;
}
