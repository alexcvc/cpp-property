#include <cassert>
#include <functional>
#include <iostream>
#include <type_traits>

enum class PropertyMode
{
    Default,
    GetOnly,
    SetOnly
};

template <class...>
class PropertyBase;

template <class...>
struct isProperty : std::false_type
{
};

template <template <typename, PropertyMode> class D, typename T, PropertyMode M>
struct isProperty<D<T, M>> : std::is_base_of<PropertyBase<D<T, M>>, D<T, M>>
{
};

template <class T>
constexpr bool isPropertyV = isProperty<T>::value;

template <class T>
struct isPropertyIgnRef : isProperty<std::remove_reference_t<T>>
{
};

template <class T>
constexpr bool isPropertyIgnRefV = isPropertyIgnRef<T>::value;

template <template <typename, PropertyMode> class D, typename T, PropertyMode Mode>
class PropertyBase<D<T, Mode>>
{
    using DerivedType = D<T, Mode>;
    const DerivedType& Derived() const& noexcept { return static_cast<const DerivedType&>(*this); }
    DerivedType& Derived() & noexcept { return static_cast<DerivedType&>(*this); }
    DerivedType&& Derived() && noexcept { return static_cast<DerivedType&&>(*this); }

    using constraint_func_type = void (PropertyBase<D<T, Mode>>::*)();
    template <constraint_func_type _Tp1>
    struct Check
    {
    };

    void Constraints()
    {
        [[maybe_unused]] auto dummy = decltype(
            std::declval<DerivedType&>().Get(), std::declval<const DerivedType&>().Get(),
            std::declval<DerivedType&&>().Get(), std::declval<DerivedType&>().Set(std::declval<const ValueType&>()),
            std::declval<const DerivedType&>().Set(std::declval<const ValueType&>()),
            std::declval<DerivedType&&>().Set(std::declval<const ValueType&>()), nullptr)(nullptr);
    }

public:
    using ValueType = std::remove_cv_t<std::remove_reference_t<T>>;  // std::remove_cvref_t for C++20
    using ReturnType = T;

    // copy constructors are prohibited
    PropertyBase(const PropertyBase&) = delete;
    PropertyBase(PropertyBase&&) = delete;

    // explicit cast (default)
    decltype(auto) operator()() const { return Derived().Get(); }

    // implicit cast (default)
    operator ReturnType() const { return Derived().Get(); }

    // copy assign operators are defined in derived classes

    // equal operator (default)
    template <
        typename V,
        std::enable_if_t<std::is_convertible_v<V, ValueType> /* && !std::is_base_of_v<PropertyBase, V>*/>* = nullptr>
    decltype(auto) operator=(V&& value) const
    {
        Derived().Set(value);
        return std::forward<V>(value);
    }

protected:
    PropertyBase()
    {
        static_assert(std::is_base_of_v<PropertyBase, DerivedType>,
                      "Template parameter class D<T, Mode> must be base of class PropertyBase");

        Check<&PropertyBase<D<T, Mode>>::Constraints>();
    }

    void CheckGetAccess() const
    {
        static_assert(Mode != PropertyMode::SetOnly, "Set-only property cannot return the value");
    }

    void CheckSetAccess() const
    {
        static_assert(Mode != PropertyMode::GetOnly, "Get-only property cannot set the value");
    }

public:
#pragma region lvalue operators
    decltype(auto) operator[](std::size_t i) const& { return Derived()()[i]; }
    decltype(auto) operator++(int) const&
    {
        auto prev = Derived()();
        operator=(prev + 1);
        return prev;
    }
    decltype(auto) operator--(int) const&
    {
        auto prev = Derived()();
        operator=(prev - 1);
        return prev;
    }
    decltype(auto) operator++() const& { return operator=(Derived()() + 1); }
    decltype(auto) operator--() const& { return operator=(Derived()() - 1); }
    decltype(auto) operator~() const& { return ~Derived()(); }
    decltype(auto) operator!() const& { return !Derived()(); }
    decltype(auto) operator-() const& { return -Derived()(); }
    decltype(auto) operator+() const& { return +Derived()(); }

    template <typename U>
    decltype(auto) operator*=(const U& right) const&
    {
        return operator=(Derived()() * right);
    }
    template <typename U>
    decltype(auto) operator/=(const U& right) const&
    {
        return operator=(Derived()() / right);
    }
    template <typename U>
    decltype(auto) operator%=(const U& right) const&
    {
        return operator=(Derived()() % right);
    }
    template <typename U>
    decltype(auto) operator+=(const U& right) const&
    {
        return operator=(Derived()() + right);
    }
    template <typename U>
    decltype(auto) operator-=(const U& right) const&
    {
        return operator=(Derived()() - right);
    }
    template <typename U>
    decltype(auto) operator<<=(const U& right) const&
    {
        return operator=(Derived()() << right);
    }
    template <typename U>
    decltype(auto) operator>>=(const U& right) const&
    {
        return operator=(Derived()() >> right);
    }
    template <typename U>
    decltype(auto) operator&=(const U& right) const&
    {
        return operator=(Derived()() & right);
    }
    template <typename U>
    decltype(auto) operator|=(const U& right) const&
    {
        return operator=(Derived()() | right);
    }
    template <typename U>
    decltype(auto) operator^=(const U& right) const&
    {
        return operator=(Derived()() ^ right);
    }
#pragma endregion
#pragma region rvalue operators
/*
    decltype(auto) operator[](std::size_t i) && { return std::move(*this).Derived()()[i]; }
    decltype(auto) operator++(int) && { return std::move(*this).Derived()()++; }
    decltype(auto) operator--(int) && { return std::move(*this).Derived()()--; }
    decltype(auto) operator++() && { return ++std::move(*this).Derived()(); }
    decltype(auto) operator--() && { return --std::move(*this).Derived()(); }
    decltype(auto) operator~() && { return ~std::move(*this).Derived()(); }
    decltype(auto) operator!() && noexcept { return !std::move(*this).Derived()(); }
    decltype(auto) operator-() && { return -std::move(*this).Derived()(); }
    decltype(auto) operator+() && { return +std::move(*this).Derived()(); }

    template <typename U>
    decltype(auto) operator*=(const U& right) &&
    {
        return operator=(std::move(*this).Derived()() * right);
    }
    template <typename U>
    decltype(auto) operator/=(const U& right) &&
    {
        return operator=(std::move(*this).Derived()() / right);
    }
    template <typename U>
    decltype(auto) operator%=(const U& right) &&
    {
        return operator=(std::move(*this).Derived()() % right);
    }
    template <typename U>
    decltype(auto) operator+=(const U& right) &&
    {
        return operator=(std::move(*this).Derived()() + right);
    }
    template <typename U>
    decltype(auto) operator-=(const U& right) &&
    {
        return operator=(std::move(*this).Derived()() - right);
    }
    template <typename U>
    decltype(auto) operator<<=(const U& right) &&
    {
        return operator=(std::move(*this).Derived()() << right);
    }
    template <typename U>
    decltype(auto) operator>>=(const U& right) &&
    {
        return operator=(std::move(*this).Derived()() >> right);
    }
    template <typename U>
    decltype(auto) operator&=(const U& right) &&
    {
        return operator=(std::move(*this).Derived()() & right);
    }
    template <typename U>
    decltype(auto) operator|=(const U& right) &&
    {
        return operator=(std::move(*this).Derived()() | right);
    }
    template <typename U>
    decltype(auto) operator^=(const U& right) &&
    {
        return operator=(std::move(*this).Derived()() ^ right);
    }
*/
#pragma endregion
};

#pragma region global operators(property vs.general)
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator*(const V& t1, U&& t2)
{
    return t1() * std::forward<U>(t2);
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator/(const V& t1, U&& t2)
{
    return t1() / std::forward<U>(t2);
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator%(const V& t1, U&& t2)
{
    return t1() % std::forward<U>(t2);
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator+(const V& t1, U&& t2)
{
    return t1() + std::forward<U>(t2);
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator-(const V& t1, U&& t2)
{
    return t1() - std::forward<U>(t2);
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator<<(const V& t1, U&& t2)
{
    return t1() << std::forward<U>(t2);
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator>>(const V& t1, U&& t2)
{
    return t1() >> std::forward<U>(t2);
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator<(const V& t1, U&& t2)
{
    return t1() < std::forward<U>(t2);
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator>(const V& t1, U&& t2)
{
    return t1() > std::forward<U>(t2);
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator<=(const V& t1, U&& t2)
{
    return t1() <= std::forward<U>(t2);
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator>=(const V& t1, U&& t2)
{
    return t1() >= std::forward<U>(t2);
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator==(const V& t1, U&& t2)
{
    return t1() == std::forward<U>(t2);
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator!=(const V& t1, U&& t2)
{
    return t1() != std::forward<U>(t2);
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator&(const V& t1, U&& t2)
{
    return t1() & std::forward<U>(t2);
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator^(const V& t1, U&& t2)
{
    return t1() ^ std::forward<U>(t2);
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator|(const V& t1, U&& t2)
{
    return t1() | std::forward<U>(t2);
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator&&(const V& t1, U&& t2)
{
    return t1() && std::forward<U>(t2);
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator||(const V& t1, U&& t2)
{
    return t1() || std::forward<U>(t2);
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator*(U&& t1, const V& t2)
{
    return std::forward<U>(t1) * t2();
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator/(U&& t1, const V& t2)
{
    return std::forward<U>(t1) / t2();
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator%(U&& t1, const V& t2)
{
    return std::forward<U>(t1) % t2();
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator+(U&& t1, const V& t2)
{
    return std::forward<U>(t1) + t2();
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator-(U&& t1, const V& t2)
{
    return std::forward<U>(t1) - t2();
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator<<(U&& t1, const V& t2)
{
    return std::forward<U>(t1) << t2();
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator>>(U&& t1, const V& right)
{
    return std::forward<U>(t1) << right();
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator<(U&& t1, const V& t2)
{
    return std::forward<U>(t1) < t2();
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator>(U&& t1, const V& t2)
{
    return std::forward<U>(t1) > t2();
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator<=(U&& t1, const V& t2)
{
    return std::forward<U>(t1) <= t2();
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator>=(U&& t1, const V& t2)
{
    return std::forward<U>(t1) >= t2();
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator==(U&& t1, const V& t2)
{
    return std::forward<U>(t1) == t2();
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator!=(U&& t1, const V& t2)
{
    return std::forward<U>(t1) != t2();
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator&(U&& t1, const V& t2)
{
    return std::forward<U>(t1) & t2();
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator^(U&& t1, const V& t2)
{
    return std::forward<U>(t1) ^ t2();
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator|(U&& t1, const V& t2)
{
    return std::forward<U>(t1) | t2();
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator&&(U&& t1, const V& t2)
{
    return std::forward<U>(t1) && t2();
}
template <typename U, typename V, std::enable_if_t<!isPropertyIgnRefV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator||(U&& t1, const V& t2)
{
    return std::forward<U>(t1) || t2();
}
#pragma endregion
#pragma region global operators(property vs.property)
template <class U, class V, std::enable_if_t<isPropertyV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator*(const U& t1, const V& t2)
{
    return t1() * t2();
}
template <class U, class V, std::enable_if_t<isPropertyV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator/(const U& t1, const V& t2)
{
    return t1() / t2();
}
template <class U, class V, std::enable_if_t<isPropertyV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator%(const U& t1, const V& t2)
{
    return t1() % t2();
}
template <class U, class V, std::enable_if_t<isPropertyV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator+(const U& t1, const V& t2)
{
    return t1() + t2();
}
template <class U, class V, std::enable_if_t<isPropertyV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator-(const U& t1, const V& t2)
{
    return t1() - t2();
}
template <class U, class V, std::enable_if_t<isPropertyV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator<<(const U& t1, const V& t2)
{
    return t1() << t2();
}
template <class U, class V, std::enable_if_t<isPropertyV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator>>(const U& t1, const V& t2)
{
    return t1() >> t2();
}
template <class U, class V, std::enable_if_t<isPropertyV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator<(const U& t1, const V& t2)
{
    return t1() < t2();
}
template <class U, class V, std::enable_if_t<isPropertyV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator>(const U& t1, const V& t2)
{
    return t1() > t2();
}
template <class U, class V, std::enable_if_t<isPropertyV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator<=(const U& t1, const V& t2)
{
    return t1() <= t2();
}
template <class U, class V, std::enable_if_t<isPropertyV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator>=(const U& t1, const V& t2)
{
    return t1() >= t2();
}
template <class U, class V, std::enable_if_t<isPropertyV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator==(const U& t1, const V& t2)
{
    return t1() == t2();
}
template <class U, class V, std::enable_if_t<isPropertyV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator!=(const U& t1, const V& t2)
{
    return t1() != t2();
}
template <class U, class V, std::enable_if_t<isPropertyV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator&(const U& t1, const V& t2)
{
    return t1() & t2();
}
template <class U, class V, std::enable_if_t<isPropertyV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator^(const U& t1, const V& t2)
{
    return t1() ^ t2();
}
template <class U, class V, std::enable_if_t<isPropertyV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator|(const U& t1, const V& t2)
{
    return t1() | t2();
}
template <class U, class V, std::enable_if_t<isPropertyV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator&&(const U& t1, const V& t2)
{
    return t1() && t2();
}
template <class U, class V, std::enable_if_t<isPropertyV<U> && isPropertyV<V>>* = nullptr>
decltype(auto) operator||(const U& t1, const V& t2)
{
    return t1() || t2();
}
#pragma endregion

/**
 * @brief   Property class
 *
 * @tparam  T   Return type
 * @tparam  PropertyMode    Default/Get-only/Set-only
 */
template <class T, PropertyMode Mode = PropertyMode::Default>
class Property : public PropertyBase<Property<T, Mode>>
{
    using Base = PropertyBase<Property<T, Mode>>;
    friend Base;

public:
    using ValueType = typename Base::ValueType;
    using ReturnType = typename Base::ReturnType;

    template <class G, class S, PropertyMode m = Mode, std::enable_if_t<m == PropertyMode::Default>* = nullptr>
    Property(G get_f, S set_f) : getter_(get_f), setter_(set_f)
    {
        static_assert(std::is_same_v<ReturnType, decltype(get_f())>, "Not satisfied: get_f() -> ReturnType");
        static_assert(std::is_same_v<void, decltype(set_f(std::declval<const ValueType&>()))>,
                      "Not satisfied: set_f(const ValueType&) -> void");
    }
    template <class G, PropertyMode m = Mode, std::enable_if_t<m == PropertyMode::GetOnly>* = nullptr>
    explicit Property(G get_f) : getter_(get_f)
    {
        static_assert(std::is_same_v<ReturnType, decltype(get_f())>, "Not satisfied: get_f() -> ReturnType");
    }
    template <class S, PropertyMode m = Mode, std::enable_if_t<m == PropertyMode::SetOnly>* = nullptr>
    explicit Property(S set_f) : setter_(set_f)
    {
        static_assert(std::is_same_v<void, decltype(set_f(std::declval<const ValueType&>()))>,
                      "Not satisfied: set_f(const ValueType&) -> void");
    }

    // copy constructor (delete)
    Property(const Property&) = delete;
    Property(Property&&) = delete;

    // copy assign operator
    decltype(auto) operator=(const Property& right) const { return Base::operator=(right()); }
    decltype(auto) operator=(Property&& right) const { return Base::operator=(right()); }

    // equal operator
    template <class V, std::enable_if_t<std::is_convertible_v<V, ValueType>>* = nullptr>
    decltype(auto) operator=(V&& value) const
    {
        return Base::operator=(std::forward<V>(value));
    };

private:
    const std::function<ReturnType()> getter_ = std::function<ReturnType()>();
    const std::function<void(const ValueType&)> setter_ = std::function<void(const ValueType&)>();

    ReturnType Get() const
    {
        Base::CheckGetAccess();
        return getter_();
    }
    void Set(const ValueType& value) const
    {
        Base::CheckSetAccess();
        setter_(value);
    }
};

/**
 * @brief   Auto-property
 *
 * @tparam  T    Return type
 * @tparam  PropertyMode    Default/Get-only/Set-only
 */
template <class T, PropertyMode Mode = PropertyMode::Default>
class AutoProperty : public PropertyBase<AutoProperty<T, Mode>>
{
private:
    using Base = PropertyBase<AutoProperty<T, Mode>>;
    friend Base;

public:
    using ValueType = typename Base::ValueType;
    using ReturnType = std::remove_reference_t<typename Base::ReturnType>&;
    using ReturnTypeR = std::remove_reference_t<ReturnType>;

private:
    mutable ValueType entity_;

public:
    // constructor
    AutoProperty() = default;
    template <typename V, std::enable_if_t<std::is_convertible_v<V, ValueType>>* = nullptr>
    AutoProperty(V&& initial)
    {
        if constexpr (Mode == PropertyMode::SetOnly)
        {
            static_assert(Base::template false_v<T>, "Set-only property cannot have the initial value");
        }

        entity_ = std::forward<V>(initial);
    }

    // copy constructor (delete)
    AutoProperty(const AutoProperty&) = delete;
    AutoProperty(AutoProperty&&) = delete;

    // implicit cast (override)
    operator ReturnType() const& { return Get(); }
    operator ReturnTypeR() && { return std::move(*this).Get(); }

    // explicit cast (override)
    ReturnType operator()() const& { return Get(); }
    ReturnTypeR operator()() && { return std::move(*this).Get(); }

    // copy assign operator
    decltype(auto) operator=(const AutoProperty& right) const { return Base::operator=(right()); }
    decltype(auto) operator=(AutoProperty&& right) const { return Base::operator=(std::move(right)()); }

    // equal operator
    template <class V, std::enable_if_t<std::is_convertible_v<V, ValueType>>* = nullptr>
    decltype(auto) operator=(V&& value) const
    {
        return Base::operator=(std::forward<V>(value));
    };

private:
    ReturnType Get() const&
    {
        Base::CheckGetAccess();
        return entity_;
    }
    ReturnTypeR Get() &&
    {
        Base::CheckGetAccess();
        return std::move(entity_);
    }
    void Set(const ValueType& value) const
    {
        Base::CheckSetAccess();
        entity_ = value;
    }
};
