#include "runtime.h"

#include <cassert>
#include <optional>
#include <sstream>

using namespace std;

namespace runtime {

    ObjectHolder::ObjectHolder(std::shared_ptr<Object> data)
        : data_(std::move(data)) {
    }

    void ObjectHolder::AssertIsValid() const {
        assert(data_ != nullptr);
    }

    ObjectHolder ObjectHolder::Share(Object& object) {
        // Возвращаем невладеющий shared_ptr (его deleter ничего не делает)
        return ObjectHolder(std::shared_ptr<Object>(&object, [](auto* /*p*/) {  /*do nothing*/  }));
    }

    ObjectHolder ObjectHolder::None() {
        return ObjectHolder();
    }

    Object& ObjectHolder::operator*() const {
        AssertIsValid();
        return *Get();
    }

    Object* ObjectHolder::operator->() const {
        AssertIsValid();
        return Get();
    }

    Object* ObjectHolder::Get() const {
        return data_.get();
    }

    ObjectHolder::operator bool() const {
        return Get() != nullptr;
    }

    bool IsTrue(const ObjectHolder& object) {
        if (const auto* obj  = object.TryAs<Bool>()) {
            if (obj->GetValue()) { return true; }
        }
        if (const auto* obj = object.TryAs<String>()) {
            if (!(obj->GetValue().empty())) { return true; }
        }
        if (const auto* obj = object.TryAs<Number>()) {
            if (obj->GetValue() != 0) { return true; }
        }
        return false;
    }
    /*
 * Если у объекта есть метод __str__, выводит в os результат, возвращённый этим методом.
 * В противном случае в os выводится адрес объекта.
 */
    void ClassInstance::Print(std::ostream& os, Context& context) {
        if (HasMethod("__str__", 0)) {
            cls_.GetMethod("__str__")->body.get()->Execute(closure_, context).Get()->Print(os, context);
        }
        else {
            os << this;
        }
    }
    // Возвращает true, если объект имеет метод method, принимающий argument_count параметров

    bool ClassInstance::HasMethod(const std::string& method, size_t argument_count) const {
        // Заглушка, реализуйте метод самостоятельно
        if (const auto* met = cls_.GetMethod(method)) {
            if (met->formal_params.size() == argument_count) {
                return true;
            }
        }
        return false;
    }
    // Возвращает ссылку на Closure, содержащий поля объекта

    Closure& ClassInstance::Fields() {
        return closure_;
    }
    // Возвращает константную ссылку на Closure, содержащую поля объекта

    const Closure& ClassInstance::Fields() const {
        return closure_;
    }

    ClassInstance::ClassInstance(const Class& cls) : cls_(cls) {
    }
    /*
 * Вызывает у объекта метод method, передавая ему actual_args параметров.
 * Параметр context задаёт контекст для выполнения метода.
 * Если ни сам класс, ни его родители не содержат метод method, метод выбрасывает исключение
 * runtime_error
 */
    ObjectHolder ClassInstance::Call(const std::string& method,
        const std::vector<ObjectHolder>& actual_args,
        Context& context) {
        Closure closure;

        if (HasMethod(method, actual_args.size())) {
            closure["self"] = ObjectHolder::Share(*this);
            for (size_t i = 0; i < actual_args.size(); i++) {
                closure[cls_.GetMethod(method)->formal_params[i]] = actual_args[i];
            }
            return cls_.GetMethod(method)->body->Execute(closure, context);
        }
        throw runtime_error("");
    }

    Class::Class(std::string name, std::vector<Method> methods, const Class* parent) : name_(name), methods_(std::move(methods)), parent_(parent) {
    }

    // Возвращает указатель на метод name или nullptr, если метод с таким именем отсутствует
    const Method* Class::GetMethod(const std::string& name) const {
        auto it = find_if(methods_.begin(), methods_.end(), [&name](const Method& met) {return met.name == name; });
        if (it != methods_.end()) {
            return &methods_[it - methods_.begin()];
        }
        if (parent_ != nullptr) {
            return parent_->GetMethod(name);
        }
        return nullptr;
    }

    // Возвращает имя класса
    [[nodiscard]] const std::string& Class::GetName() const {
        return name_;
    }

    // Выводит в os строку "Class <имя класса>", например "Class cat"
    void Class::Print(ostream& os, Context& context) {
        os << "Class " << name_;
    }

    void Bool::Print(std::ostream& os, [[maybe_unused]] Context& context) {
        os << (GetValue() ? "True"sv : "False"sv);
    }

    bool Equal(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
        // Заглушка. Реализуйте функцию самостоятельно
        if (lhs.TryAs<Bool>() && rhs.TryAs<Bool>()) {
            return lhs.TryAs<Bool>()->GetValue() == rhs.TryAs<Bool>()->GetValue();
        }
        if (lhs.TryAs<String>() && rhs.TryAs<String>()) {
            return lhs.TryAs<String>()->GetValue() == rhs.TryAs<String>()->GetValue();
        }
        if (lhs.TryAs<Number>() && rhs.TryAs<Number>()) {
            return lhs.TryAs<Number>()->GetValue() == rhs.TryAs<Number>()->GetValue();
        }
        if (lhs.TryAs<ClassInstance>()) {
            if (lhs.TryAs<ClassInstance>()->HasMethod("__eq__", 1)) {
                return IsTrue(lhs.TryAs<ClassInstance>()->Call("__eq__"s, { rhs }, context));
            }
        }
        if (lhs.Get() == nullptr && rhs.Get() == nullptr) {
            return true;
        }
        throw runtime_error("");
    }

    bool Less(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
        // Заглушка. Реализуйте функцию самостоятельно
        if (lhs.TryAs<Bool>() && rhs.TryAs<Bool>()) {
            return lhs.TryAs<Bool>()->GetValue() < rhs.TryAs<Bool>()->GetValue();
        }
        if (lhs.TryAs<String>() && rhs.TryAs<String>()) {
            return lhs.TryAs<String>()->GetValue() < rhs.TryAs<String>()->GetValue();
        }
        if (lhs.TryAs<Number>() && rhs.TryAs<Number>()) {
            return lhs.TryAs<Number>()->GetValue() < rhs.TryAs<Number>()->GetValue();
        }
        if (lhs.TryAs<ClassInstance>()) {
            if (lhs.TryAs<ClassInstance>()->HasMethod("__lt__", 1)) {
                return IsTrue(lhs.TryAs<ClassInstance>()->Call("__lt__"s, { rhs }, context));
            }
        }
        throw runtime_error("");
    }

    bool NotEqual(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
        return !Equal(lhs, rhs, context);
    }

    bool Greater(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
        return !Less(lhs, rhs, context) && !Equal(lhs, rhs, context);
    }

    bool LessOrEqual(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
        return (Less(lhs, rhs, context) || Equal(lhs, rhs, context));
    }

    bool GreaterOrEqual(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
        return !Less(lhs, rhs, context);
    }

}  // namespace runtime