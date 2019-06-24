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
    entry.md5_str = "a95c530a7af5f492a74499e70578d150";  // validating 32 hexadecimal digits

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
Property(G getter, S setter);
```

`PropertyMode::GetOnly`

```cpp
template <class G>
Property(G getter);
```

`PropertyMode::SetOnly`

```cpp
template <class S>
Property(S setter);
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

#### Constructor

The initial value of the auto-property is given by the constructor. 

```cpp
Property();
```

```cpp
template <typename V>
Property(V&& initial_value);
```

### Get

`Property<T>` is implicitly casted to `T` through the get function.

```cpp
int a = 0;
auto p = Property<const int&, PropertyMode::GetOnly>([&a]() -> const int& { return a; });

// implicit casting
int b = p;         // a == b
const int& c = p;  // &a == &c

// explicit casting
auto d = p();         // auto -> int
const auto& e = p();  // const auto& -> const int&

// NG
auto f = p;           // Note: auto -> Property<const int&>
```

Because operator `.` cannot be overloaded, you should use explicit casting `operator()()` when `T` is a class.

```cpp
std::string str;
auto p = Property<std::string&, PropertyMode::GetOnly>([&str]() -> std::string& { return str; });
p().reserve(3);    // OK
p.reserve(3);      // NG
```

Use `operator()()` also when a type deducing problem occurs, for example duplicate definitions or overloading in other library.

### Set

`Property<T>` accepts any type as a right hand of `operator=` with the set function as long as the value is convertible to `V` (non-reference type of `T`).

```cpp
std::string str;
auto p = Property<std::string&, PropertyMode::SetOnly>([&str](const std::string& v) { str = v; });
p = "new string";  // char* -> std::string -> Property<std::string&>
```

### Copy

Copy constructors are deleted. Copy assign operators act as `set` the left hand side to the  `get` value of the right hand side.

```cpp
std::string str0 = "test0";
auto p0 = Property<std::string&, PropertyMode::GetOnly>([&str0]() -> std::string& { return str0; });

// copy constructor
// auto p1 = p0;    // NG: copy constructor is deleted

std::string str1 = "test1";
auto p1 = Property<std::string&, PropertyMode::SetOnly>([&str1](const std::string& v) { str1 = v; });

// copy assign operator
p1 = p0;    // p1 = p0() -> str1 = str0;

auto ap0 = AutoProperty<const std::string&>();

// copy constructor
auto ap1 = ap0;  // OK: copy constructor is not called
                 // ->  auto ap1 = AutoProperty<const std::string&>(ap0())
```

### Operators

Operators are implemented to act like its entity as much as possible.

## TODO

*   Support access specifier:
    *   public/private keywords for getter and setter respectively
*   Optimize for the performance
