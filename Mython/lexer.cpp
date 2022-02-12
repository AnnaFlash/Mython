#include "lexer.h"

#include <algorithm>
#include <charconv>
#include <unordered_map>

using namespace std;

namespace parse {

    bool operator==(const Token& lhs, const Token& rhs) {
        using namespace token_type;

        if (lhs.index() != rhs.index()) {
            return false;
        }
        if (lhs.Is<Char>()) {
            return lhs.As<Char>().value == rhs.As<Char>().value;
        }
        if (lhs.Is<Number>()) {
            return lhs.As<Number>().value == rhs.As<Number>().value;
        }
        if (lhs.Is<String>()) {
            return lhs.As<String>().value == rhs.As<String>().value;
        }
        if (lhs.Is<Id>()) {
            return lhs.As<Id>().value == rhs.As<Id>().value;
        }
        return true;
    }

    bool operator!=(const Token& lhs, const Token& rhs) {
        return !(lhs == rhs);
    }

    std::ostream& operator<<(std::ostream& os, const Token& rhs) {
        using namespace token_type;

#define VALUED_OUTPUT(type) \
    if (auto p = rhs.TryAs<type>()) return os << #type << '{' << p->value << '}';

        VALUED_OUTPUT(Number);
        VALUED_OUTPUT(Id);
        VALUED_OUTPUT(String);
        VALUED_OUTPUT(Char);

#undef VALUED_OUTPUT

#define UNVALUED_OUTPUT(type) \
    if (rhs.Is<type>()) return os << #type;

        UNVALUED_OUTPUT(Class);
        UNVALUED_OUTPUT(Return);
        UNVALUED_OUTPUT(If);
        UNVALUED_OUTPUT(Else);
        UNVALUED_OUTPUT(Def);
        UNVALUED_OUTPUT(Newline);
        UNVALUED_OUTPUT(Print);
        UNVALUED_OUTPUT(Indent);
        UNVALUED_OUTPUT(Dedent);
        UNVALUED_OUTPUT(And);
        UNVALUED_OUTPUT(Or);
        UNVALUED_OUTPUT(Not);
        UNVALUED_OUTPUT(Eq);
        UNVALUED_OUTPUT(NotEq);
        UNVALUED_OUTPUT(LessOrEq);
        UNVALUED_OUTPUT(GreaterOrEq);
        UNVALUED_OUTPUT(None);
        UNVALUED_OUTPUT(True);
        UNVALUED_OUTPUT(False);
        UNVALUED_OUTPUT(Eof);

#undef UNVALUED_OUTPUT

        return os << "Unknown token :("sv;
    }

    void ParseClassDefinition(Lexer& lexer) {
        lexer.Expect<token_type::Class>();
        auto name = lexer.ExpectNext<token_type::Id>().value;
        lexer.ExpectNext<token_type::Char>(':');
        //        ...
    }

    bool Lexer::CheckIds(std::string s) const {
        bool check = true;
        for (size_t i = 0; i < s.size(); i++) {
            if (!(    (s[i] <= 'z' && s[i] >= 'a') || (s[i] <= 'Z' && s[i] >= 'A') || (s[i] == '_') || (s[i] <= '9' && s[i] >= '0')   )) {
                check = false;
            }
        }
        if (check) {
            if (s[0] <= '9' && s[0] >= '0') {
                check = false;
            }
        }
        return check;
    }
    bool is_number(const std::string& s)
    {
        return !s.empty() && std::find_if(s.begin(),
            s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
    }
    void Lexer::ParseIds() {
        std::string buf;
        char c;
        while (true) {
            input_.get(c);
            if (c == ' ' || c == '=' || c == '\n' ||  c == ':' || c == '*' || c == '-' || c == '/' 
                || c == '+' || c == '!' || c == '#' || c == '(' || c == ')' || c==',' || c=='.') {
                input_.putback(c);
                break;
            }
            if (input_.eof()) {
                break;
            }
            buf.push_back(c);
        }
        if (buf == "class") {
            token_ = token_type::Class{};
        }
        else if (buf == "return") {
            token_ = token_type::Return{};
        }
        else if (buf == "if") {
            token_ = token_type::If{};
        }
        else if (buf == "else") {
            token_ = token_type::Else{};
        }
        else if (buf == "def") {
            token_ = token_type::Def{};
        }
        else if (buf == "print") {
            token_ = token_type::Print{};
        }
        else if (buf == "and") {
            token_ = token_type::And{};
        }
        else if (buf == "or") {
            token_ = token_type::Or{};
        }
        else if (buf == "not") {
            token_ = token_type::Not{};
        }
        else if (buf == "None") {
            token_ = token_type::None{};
        }
        else if (buf == "True") {
            token_ = token_type::True{};
        }
        else if (buf == "False") {
            token_ = token_type::False{};
        }
        else if (is_number(buf)) {
            token_ = token_type::Number{ stoi(buf) };
        }
        else {
            if (!CheckIds(buf)) {
                throw ("Bad Id");
            }
            token_ = token_type::Id{ buf };
        }
    }
    //void Lexer::ParseNumbers() {
    //    int buf;
    //    input_ >> buf;
    //    token_ = token_type::Number{ buf };
    //}

    void Lexer::ParseString() {
        //char c;
        //input_.get(c);
        //if (c == '\'') {
        //    std::string buf;
        //    std::getline(input_, buf, '\'');
        //    token_ = token_type::String{ buf };
        //}
        //else {
        //    std::string buf;
        //    std::getline(input_, buf, '\"');
        //    token_ = token_type::String{ buf };
        //}
        char c;
        input_.get(c);
        auto it = std::istreambuf_iterator<char>(input_);
        auto end = std::istreambuf_iterator<char>();
        std::string s;
        while (true) {
            if (it == end && s.back() != '"') {
                // throw ParsingError("String parsing error");
            }
            if (it == end) {
                break;
            }
            const char ch = *it;
            if ((ch == '"' && c == '"') || ch == '\'' && c == '\'')
            {
                ++it;
                break;
            }
            else if (ch == '\\') {
                ++it;
                if (it == end) {
                    //throw ParsingError("String parsing error");
                }
                const char escaped_char = *(it);
                switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                case '\'':
                    s.push_back('\'');
                    break;
                default:
                    throw LexerError("Not implemented"s);
                }
            }
            else if (ch == '\n' || ch == '\r') {
                //        throw ParsingError("Unexpected end of line");
            }
            else {
                s.push_back(ch);
            }
            ++it;
        }
        token_ = token_type::String{ s };
    }

    void Lexer::ParseCharLogicOPerations() {
        char c;
        input_.get(c);
        char b;
        input_.get(b);
        if (!input_.eof()) {
            if (b == '=') {
                switch (c)
                {
                case '=':
                    token_ = token_type::Eq{};
                    return;
                case '!':
                    token_ = token_type::NotEq{};
                    return;
                case '<':
                    token_ = token_type::LessOrEq{};
                    return;
                case '>':
                    token_ = token_type::GreaterOrEq{};
                    return;
                default:
                    input_.putback(b);
                    break;
                }
            }
            else { input_.putback(b); }
        }
        token_ = token_type::Char{ c };
    }
    const Token& Lexer::CurrentToken() const {  // x = 42
    // Заглушка. Реализуйте метод самостоятельно
        return token_;
    }
    Lexer::Lexer(std::istream& input) : input_(input) {
        SetToken();
        FirstConstruct = false;
    }

    void Lexer::SetToken() {
        char c;
        input_.get(c);
        if (c == '#') {
            CommentFlag = true;
        }
        if (c == '\n') {
            CommentFlag = false;
        }
        if (!CommentFlag) {
            if (input_.eof()) {
                Dedent != 0 ? Dedent = Dedent - 2, token_ = token_type::Dedent{} : token_ = token_type::Eof{};
            }
            else if (c == '\n') {
                if (token_.Is<token_type::Newline>() || FirstConstruct) {
                    SetToken();
                }
                else {
                    TimeToCountInDedents = true;
                    token_ = token_type::Newline{};
                }
            }
            else if ((c == '\'') || (c == '\"')) {
                TimeToCountInDedents = false;
                input_.putback(c);
                ParseString();
            }
            else if (c == '-' || c == '*' || c == '/' || c == '+' || c == '!' || c == '<'
                || c == '>' || c == '=' || c == ':' || c == '(' || c == ')' || c == ',' || c == '.') {
                TimeToCountInDedents = false;
                input_.putback(c);
                ParseCharLogicOPerations();
            }
            else if (TimeToCountInDedents == true && c == ' ') {
                while (c == ' ') {
                    input_.get(c);
                    Indent++;
                }
                input_.putback(c);
                if (Indent == Dedent) {
                    Indent = 0;
                    SetToken();
                }
                else {
                    Indent % 2 != 0 ? throw LexerError("Bad indent") : Indent;
                    if (Indent < Dedent) {
                        size_t CountDedetns = Indent;
                        token_ = token_type::Dedent{};
                        Dedent = Dedent - 2;
                        Indent = 0;
                        if (Dedent != CountDedetns) {
                            for (size_t i = 0; i < CountDedetns; i++) {
                                input_.putback(' ');
                            }
                            TimeToCountInDedents = true;
                        }
                        else {
                            TimeToCountInDedents = false;
                        }
                    }
                    else {
                        Dedent = Indent;
                        Indent = 0;
                        token_ = token_type::Indent{};
                        TimeToCountInDedents = false;
                    }
                }
            }
            else {
                TimeToCountInDedents = false;
                input_.putback(c);
                ParseIds();
            }
        }
        else {
            SetToken();
        }
    }

/* 
    if (self.ok):
      self.a = a
      self.b = b
      self.c = c

  def __str__():
  */
    Token Lexer::NextToken() {
        // Заглушка. Реализуйте метод самостоятельно
        char c;
        input_.get(c);
        if (c == '#') {
            CommentFlag = true;
        }
        if (c == '\n' || input_.eof()) {
            CommentFlag = false;
        }
        if (!CommentFlag) {
            if (input_.eof()) {
                if (Dedent != 0 && TimeToCountInDedents == true) {
                    token_ = token_type::Dedent{};
                    Dedent = Dedent - 2;
                }
                else if (token_.Is<token_type::Dedent>() || token_.Is<token_type::Indent>()) {
                    token_ = token_type::Eof{};
                }
                else if (!token_.Is<token_type::Newline>() && !token_.Is<token_type::Eof>()) {
                    token_ = token_type::Newline{};
                }
                else {
                    token_ = token_type::Eof{};
                }
                return CurrentToken();
            }
            if (c == '\n') {
                if (token_.Is<token_type::Newline>()) {
                    return NextToken();
                }
                else {
                    TimeToCountInDedents = true;
                    token_ = token_type::Newline{};
                    return CurrentToken();
                }
            }
            else if (c == ' ' && TimeToCountInDedents == false) {
                while (c == ' ') {
                    input_.get(c);
                }
            }
            else if (Dedent != 0 && c != ' ' && TimeToCountInDedents == true) {
                input_.putback(c);
                token_ = token_type::Dedent{};
                Dedent = Dedent - 2;
                return CurrentToken();
            }
            //else if (token_.Is<token_type::Dedent>() && Dedent != 0) {
            //    input_.putback(c);
            //    token_ = token_type::Dedent{};
            //    Dedent = Dedent - 2;
            //    return CurrentToken();
            //}
            input_.putback(c);
            SetToken();
            return CurrentToken();
        }
        else {
            return NextToken();
        }
    }

}  // namespace parse