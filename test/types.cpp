#include <typeinfo>
#include <cstdio>
#include <string>

struct Empty {};

template<class T>
static void
print_typeid(const char *name)
{
    std::printf("typeid(%s).name:      '%s'\n"
                "typeid(%s).hash_code:  %zu\n"
                "\n",
                name, typeid(T).name(), // implementation defined!
                name, typeid(T).hash_code());

}

#define print_typeid(T)         print_typeid<T>(#T)
#define print_boolexpr(expr)    std::printf(#expr " ? %s\n", (expr) ? "true" : "false")

int
main()
{
    print_boolexpr(typeid(int) == typeid(const int &));
    print_boolexpr(typeid(int) == typeid(int *));

    print_typeid(int);
    print_typeid(const int);
    print_typeid(int *);
    print_typeid(const int *);
    print_typeid(int &);
    print_typeid(const int &);

    print_typeid(char);
    print_typeid(const char *);
    print_typeid(struct Empty);

    print_typeid(std::string);

    // print_typeid(struct { int x;}); // not allowed!
    struct {int x;} anon1;
    decltype(anon1) anon2;
    using Anonymous = decltype(anon1); // allowed
    print_typeid(decltype(anon1));
    print_typeid(Anonymous);
    print_boolexpr(typeid(anon1) == typeid(anon2));
    return 0;
}
