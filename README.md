# cpp-property

C# like properties in C++

## Requirement

*   C++17

## Example

```cpp
#include <string>
#include "cpp_property.h"

class Entry
{
private:
    std::string md5_;

public:
    // property
    Property<const std::string&> md5_str = {
        [this]() -> const std::string& { return md5_; },
        [this](const std::string& value) {
            constexpr auto Hexcheck = [](const auto c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'); };
            const auto check = value.size() == 32 && std::all_of(value.begin(), value.end(), Hexcheck);
            if (!check)
            {
                throw std::invalid_argument("");
            }
            md5_ = value;
        }};

    // auto-property
    AutoProperty<const std::string&> name;
    AutoProperty<const std::string&, PropertyMode::GetOnly> group;

    Entry(const std::string& name_str, const std::string& group_name) : name(name_str), group(group_name) {}
};

int main(void) {
    auto entry = Entry("FileX", "GroupX");
    entry.name = "File0";
    // entry.group = "Group0";  // compilation error: get-only property
    entry.md5_str = "a95c530a7af5f492a74499e70578d150x";  // validating 32 hexadecimal digits

    std::cout << "Name:\t" << entry.name << std::endl;
    std::cout << "Group:\t" << entry.group << std::endl;
    std::cout << "Hash:\t" << entry.md5_str << std::endl;
    return 0;
}
```

## Class

### Property

```cpp
template <class T, PropertyMode Mode = PropertyMode::Default>
class Property;
```

*   `T`: Return type of the property
*   `Mode`: Property Mode
    *   Default/Get-Only/Set-Only

`T` may be a const reference or just a value type typically, because the value should be able to be modified only through the setter.

#### Constructor

`PropertyMode::Default`

```cpp
template <class G, class S>
Property(G getter, S setter)
```

`PropertyMode::GetOnly`

```cpp
template <class G>
Property(G getter)
```

`PropertyMode::SetOnly`

```cpp
template <class S>
Property(S setter)
```

Requirements

*   `getter` should return `T` type without any parameter: `getter() -> T`.
*   `setter` should receive `V` type and return void: `setter(V) -> void`.
    *   `V` is non-reference type to which `T` refers.

### AutoProperty

```cpp
template <class T, PropertyMode Mode = PropertyMode::Default>
class AutoProperty;
```

*   `T&`: Return type of the property
    *   lvalue reference will be added: `T = int` is practically equivalent to `T = int&`
*   `Mode`: Property Mode
    *   Default/Get-Only/Set-Only

`T` may be a const reference type typically with the same reason above.

### Get

`Property<T>` is implicitly casted to `T` through the get function.

```cpp
int a = 0;
auto p = Property<const int&, PropertyMode::GetOnly>([&a]() -> const int& { return a; });

// implicit casting
int b = p;         // a == b
const int& c = p;  // &a == &c

// explicit casting
auto d = p();         // auto -> int ()
const auto& e = p();  // const auto& -> const int&

// NG
auto f = p;           // Note: auto -> Property<const int&>
```

Because operator `.` cannot be overloaded, you should use explicit casting `operator()()` when `T` is a class.

```cpp
std::string str;
auto p3 = Property<std::string&, PropertyMode::GetOnly>([&str]() -> std::string& { return str; });
p3().reserve(3);    // OK
p3.reserve(3);      // NG
```

Use `operator()()` also when a type deducing problem occurs, for example duplicate definitions or overloading in other library.

### Set

`Property<T>` accepts any type as a right hand of `operator=` with the set function as long as the value is convertible to `V` (non-reference type of `T`).

```cpp
std::string str;
auto p = Property<std::string&, PropertyMode::SetOnly>([&str](const std::string& v) { str = v; });
p = "new string";  // char* -> std::string -> Property<std::string&>
```

### Operators

Operators are implemented to act like its entity as much as possible.

## TODO

*   Support access specifier:
    *   public/private keywords for getter and setter respectively
*   Optimize for the performance
