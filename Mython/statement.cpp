#include "statement.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace ast {

    using runtime::Closure;
    using runtime::Context;
    using runtime::ObjectHolder;

    namespace {
        const string ADD_METHOD = "__add__"s;
        const string INIT_METHOD = "__init__"s;
    }  // namespace

    ObjectHolder Assignment::Execute(Closure& closure, Context& context) {
        closure[var_] = rv_.get()->Execute(closure, context);
        return rv_.get()->Execute(closure, context);
    }

    Assignment::Assignment(std::string var, std::unique_ptr<Statement> rv) : var_(var), rv_(std::move(rv)) {
    }

    VariableValue::VariableValue(const std::string& var_name) : var_name_(var_name) {
    }

    VariableValue::VariableValue(std::vector<std::string> dotted_ids) : dotted_ids_(dotted_ids) {
    }

    ObjectHolder VariableValue::Execute(Closure& closure, Context& context) {
        if (!var_name_.empty()) {
            if (closure.count(var_name_)) {
                return closure[var_name_];
            }
            else {
                throw runtime_error("");
            }
        }
        if (!dotted_ids_.empty()) {
            ObjectHolder chain;
            chain = closure[dotted_ids_[0]];
            for (size_t i = 1; i < dotted_ids_.size(); i++) {
                if (chain.Get()) {
                    if(chain.TryAs<runtime::ClassInstance>()->Fields().count(dotted_ids_[i])) {
                        chain = chain.TryAs<runtime::ClassInstance>()->Fields()[dotted_ids_[i]];
                    }
                }
                else {
                    chain = closure[dotted_ids_[i]];
                }
            }
            return chain;
        }
        throw runtime_error("");
    }

    unique_ptr<Print> Print::Variable(const std::string& name) {
        auto ptr = std::unique_ptr<Print >{ new Print{std::unique_ptr<Statement>{new VariableValue(name)}} }; 
        return ptr; 
    }

    Print::Print(unique_ptr<Statement> argument) : argument_(std::move(argument)){
    }

    Print::Print(vector<unique_ptr<Statement>> args) : args_(std::move(args)){
    }

    ObjectHolder Print::Execute(Closure& closure, Context& context) {
        // Заглушка. Реализуйте метод самостоятельно
        if (argument_ != nullptr) {
            argument_.get()->Execute(closure, context).Get()->Print(context.GetOutputStream(), context);
            context.GetOutputStream() << "\n";
            return argument_.get()->Execute(closure, context);
        }
        if (!args_.empty()) {
            for (size_t i = 0; i < args_.size();i++) {
                if (auto a = args_[i].get()->Execute(closure, context)) {
                    if (a.operator bool()) {
                        if (i == 0) {
                           a.Get()->Print(context.GetOutputStream(), context);
                        }
                        else {
                            context.GetOutputStream() << " ";

                            a.Get()->Print(context.GetOutputStream(), context);
                        }
                    }
                }
                else {
                    i==0 ? context.GetOutputStream() << "None" : context.GetOutputStream() << " None";
                }
            }
            context.GetOutputStream() << "\n";
            return {};
        }
        context.GetOutputStream() << "\n";
        return {};
    }

    MethodCall::MethodCall(std::unique_ptr<Statement> object, std::string method,
        std::vector<std::unique_ptr<Statement>> args) : object_(std::move(object)), method_(method), args_(std::move(args)) {
    }


    // Вызывает метод object.method со списком параметров args
    ObjectHolder MethodCall::Execute(Closure& closure, Context& context ) {
        std::vector<runtime::ObjectHolder> args;
        for (const auto& arg : args_) {
            args.push_back(arg.get()->Execute(closure, context));
        }
        return object_.get()->Execute(closure, context).TryAs<runtime::ClassInstance>()->Call(method_, args, context);
    }

    ObjectHolder Stringify::Execute(Closure& closure, Context& context) {
        // Заглушка. Реализуйте метод самостоятельно
        ObjectHolder holder;
        std::ostringstream stream;
        if (argument_.get()->Execute(closure, context).Get()) {
            argument_.get()->Execute(closure, context).Get()->Print(stream, context);
            runtime::String s(stream.str());
            holder = holder.Own(std::move(s));
        }
        else {
            runtime::String s("None");
            holder = holder.Own(std::move(s));
        }
        return holder;
    }
    
    ObjectHolder Add::Execute(Closure& closure, Context& context) {
        auto lhs = lhs_.get()->Execute(closure, context);
        auto rhs = rhs_.get()->Execute(closure, context);
        if (auto ptr_l = lhs.TryAs<runtime::Number>()) {
            if (auto ptr_r = rhs.TryAs<runtime::Number>()) {
                ObjectHolder holder;
                auto l = ptr_l->GetValue();
                auto r = ptr_r->GetValue();
                runtime::Number n(l + r);
                holder = holder.Own(std::move(n));
                return holder;
            }
        }
        else if (auto ptr_l = lhs.TryAs< runtime::String>()) {
            if (auto ptr_r = rhs.TryAs<runtime::String>()) {
                ObjectHolder holder;
                auto l = ptr_l->GetValue();
                auto r = ptr_r->GetValue();
                runtime::String s(l + r);
                holder = holder.Own(std::move(s));
                return holder;
            }
        }
        else if (auto ptr_l = lhs.TryAs<runtime::ClassInstance>()) {
            return ptr_l->Call(ADD_METHOD, { rhs_.get()->Execute(closure, context) }, context);
        }
        throw runtime_error("");
    }

    ObjectHolder Sub::Execute(Closure& closure, Context& context) {
        auto lhs = lhs_.get()->Execute(closure, context);
        auto rhs = rhs_.get()->Execute(closure, context);
        if (auto ptr_l = lhs.TryAs<runtime::Number>()) {
            if (auto ptr_r = rhs.TryAs<runtime::Number>()) {
                ObjectHolder holder;
                auto l = ptr_l->GetValue();
                auto r = ptr_r->GetValue();
                runtime::Number n(l - r);
                holder = holder.Own(std::move(n));
                return holder;
            }
            else {
                throw runtime_error("");
            }
        }
        else {
            throw runtime_error("");
        }
    }

    ObjectHolder Mult::Execute(Closure& closure, Context& context) {
        auto lhs = lhs_.get()->Execute(closure, context);
        auto rhs = rhs_.get()->Execute(closure, context);
        if (auto ptr_l = lhs.TryAs<runtime::Number>()) {
            if (auto ptr_r = rhs.TryAs<runtime::Number>()) {
                ObjectHolder holder;
                auto l = ptr_l->GetValue();
                auto r = ptr_r->GetValue();
                runtime::Number n(l * r);
                holder = holder.Own(std::move(n));
                return holder;
            }
            else {
                throw runtime_error("");
            }
        }
        else {
            throw runtime_error("");
        }
    }

    ObjectHolder Div::Execute(Closure& closure, Context& context) {
        auto lhs = lhs_.get()->Execute(closure, context);
        auto rhs = rhs_.get()->Execute(closure, context);
        if (auto ptr_l = lhs.TryAs<runtime::Number>()) {
            if (auto ptr_r = rhs.TryAs<runtime::Number>()) {
                ObjectHolder holder;
                auto l = ptr_l->GetValue();
                auto r = ptr_r->GetValue();
                runtime::Number n(l / r);
                holder = holder.Own(std::move(n));
                return holder;
            }
            else {
                throw runtime_error("");
            }
        }
        else {
            throw runtime_error("");
        }
    }

    ObjectHolder Compound::Execute(Closure& closure, Context& context) {
        for (size_t i = 0; i < compounds_.size();i++) {
            compounds_[i].get()->Execute(closure, context);
       }
        return {};
    }

    ObjectHolder Return::Execute(Closure& closure, Context& context) {
        ObjectHolder holder = statement_.get()->Execute(closure, context);
        if (holder.TryAs<runtime::String>()) {
            throw runtime_error(holder.TryAs<runtime::String>()->GetValue());
        }
        if (holder.TryAs<runtime::Number>()) {
            auto a = holder.TryAs<runtime::Number>()->GetValue();
            throw runtime_error(std::to_string(a));
        }
        return {};
    }

    ClassDefinition::ClassDefinition(ObjectHolder cls) : cls_(cls) {
    }

    ObjectHolder ClassDefinition::Execute(Closure& closure, Context& context) {
        closure[cls_.TryAs<runtime::Class>()->GetName()] = cls_;
        return closure[cls_.TryAs<runtime::Class>()->GetName()];
    }

    FieldAssignment::FieldAssignment(VariableValue object, std::string field_name,
        std::unique_ptr<Statement> rv) : object_(object), field_name_(field_name), rv_(std::move(rv)) {
    }

    ObjectHolder FieldAssignment::Execute(Closure& closure, Context& context) {
        return object_.Execute(closure, context).TryAs<runtime::ClassInstance>()->Fields()[field_name_] = rv_->Execute(closure, context);
    }

    IfElse::IfElse(std::unique_ptr<Statement> condition, std::unique_ptr<Statement> if_body,
        std::unique_ptr<Statement> else_body) : condition_(std::move(condition)),  if_body_(std::move(if_body)), else_body_(std::move(else_body)) {
    }

    ObjectHolder IfElse::Execute(Closure& closure, Context& context) {
        if (runtime::IsTrue(condition_.get()->Execute(closure, context))) {
            return if_body_.get()->Execute(closure, context);
        }
        else {
            if (else_body_.get() != nullptr) {
                return else_body_.get()->Execute(closure, context);
            }
        }
        return {};
    }

    ObjectHolder Or::Execute(Closure& closure, Context& context) {
        ObjectHolder holder;
        if (runtime::IsTrue(lhs_.get()->Execute(closure, context))) {
            holder = holder.Own(runtime::Bool(true));
            return holder;
        }
        else {
            if (runtime::IsTrue(rhs_.get()->Execute(closure, context))) {
                holder = holder.Own(runtime::Bool(true));
                return holder;
            }
            else {
                holder = holder.Own(runtime::Bool(false));
                return holder;
            }
        }
        holder = holder.Own(runtime::Bool(false));
        return holder;
    }

    ObjectHolder And::Execute(Closure& closure, Context& context) {
        ObjectHolder holder;
        if (runtime::IsTrue(lhs_.get()->Execute(closure, context))) {
            if (runtime::IsTrue(rhs_.get()->Execute(closure, context))) {
                holder = holder.Own(runtime::Bool(true));
                return holder;
            }
        }
        holder = holder.Own(runtime::Bool(false));
        return holder;
    }

    ObjectHolder Not::Execute(Closure& closure, Context& context) {
        ObjectHolder holder;
        if (runtime::IsTrue(argument_.get()->Execute(closure, context))) {
            holder = holder.Own(runtime::Bool(false));
            return holder;
        }
        holder = holder.Own(runtime::Bool(true));
        return holder;
    }

    Comparison::Comparison(Comparator cmp, unique_ptr<Statement> lhs, unique_ptr<Statement> rhs)
        : BinaryOperation(std::move(lhs), std::move(rhs)), cmp_(cmp) {
    }

    ObjectHolder Comparison::Execute(Closure& closure, Context& context) {
        bool cmp = cmp_(lhs_.get()->Execute(closure, context), rhs_.get()->Execute(closure, context), context);
        ObjectHolder holder;
        holder = holder.Own(runtime::Bool(cmp));
        return holder;
    }

    NewInstance::NewInstance(const runtime::Class& class_, std::vector<std::unique_ptr<Statement>> args) : new_instance_(class_), args_(std::move(args)) {
    }

    NewInstance::NewInstance(const runtime::Class& class_) : new_instance_(class_) {
    }

    ObjectHolder NewInstance::Execute(Closure& closure, Context& context) {
        if (new_instance_.HasMethod(INIT_METHOD, args_.size())) {
            std::vector<runtime::ObjectHolder> args;
            for (const auto& arg : args_) {
                args.push_back(arg.get()->Execute(closure, context));
            }
            new_instance_.Call(INIT_METHOD, args, context);
            return ObjectHolder::Share(new_instance_);
        }
        else {
            return ObjectHolder::Share(new_instance_);
        }
        return {};
    }

    MethodBody::MethodBody(std::unique_ptr<Statement>&& body) : body_(std::move(body)) {
    }

    ObjectHolder MethodBody::Execute(Closure& closure, Context& context) {
        try {
            body_.get()->Execute(closure, context);
        }
        catch (runtime_error const& e)
        {
            auto a = e.what();
            std::string s(a);
            ObjectHolder holder;
            holder = holder.Own((runtime::String(s)));
            return holder;
        }
        return {};
    }

}  // namespace ast