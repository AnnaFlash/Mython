#pragma once

#include <iosfwd>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>

namespace parse {

    namespace token_type {
        struct Number {  // ������� ������
            int value;   // �����
        };

        struct Id {             // ������� ��������������
            std::string value;  // ��� ��������������
        };

        struct Char {    // ������� �������
            char value;  // ��� �������
        };

        struct String {  // ������� ���������� ���������
            std::string value;
        };

        struct Class {};    // ������� �class�
        struct Return {};   // ������� �return�
        struct If {};       // ������� �if�
        struct Else {};     // ������� �else�
        struct Def {};      // ������� �def�
        struct Newline {};  // ������� ������ ������
        struct Print {};    // ������� �print�
        struct Indent {};  // ������� ����������� �������, ������������� ���� ��������
        struct Dedent {};  // ������� ����������� �������
        struct Eof {};     // ������� ������ �����
        struct And {};     // ������� �and�
        struct Or {};      // ������� �or�
        struct Not {};     // ������� �not�
        struct Eq {};      // ������� �==�
        struct NotEq {};   // ������� �!=�
        struct LessOrEq {};     // ������� �<=�
        struct GreaterOrEq {};  // ������� �>=�
        struct None {};         // ������� �None�
        struct True {};         // ������� �True�
        struct False {};        // ������� �False�
    }  // namespace token_type

    using TokenBase
        = std::variant<token_type::Number, token_type::Id, token_type::Char, token_type::String,
        token_type::Class, token_type::Return, token_type::If, token_type::Else,
        token_type::Def, token_type::Newline, token_type::Print, token_type::Indent,
        token_type::Dedent, token_type::And, token_type::Or, token_type::Not,
        token_type::Eq, token_type::NotEq, token_type::LessOrEq, token_type::GreaterOrEq,
        token_type::None, token_type::True, token_type::False, token_type::Eof>;

    struct Token : TokenBase {
        using TokenBase::TokenBase;

        template <typename T>
        [[nodiscard]] bool Is() const {
            return std::holds_alternative<T>(*this);
        }

        template <typename T>
        [[nodiscard]] const T& As() const {
            return std::get<T>(*this);
        }

        template <typename T>
        [[nodiscard]] const T* TryAs() const {
            return std::get_if<T>(this);
        }
    };

    bool operator==(const Token& lhs, const Token& rhs);
    bool operator!=(const Token& lhs, const Token& rhs);

    std::ostream& operator<<(std::ostream& os, const Token& rhs);

    class LexerError : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };

    class Lexer {
    public:
        explicit Lexer(std::istream& input);

        // ���������� ������ �� ������� ����� ��� token_type::Eof, ���� ����� ������� ����������
        [[nodiscard]] const Token& CurrentToken() const;

        // ���������� ��������� �����, ���� token_type::Eof, ���� ����� ������� ����������
        Token NextToken();

        // ���� ������� ����� ����� ��� T, ����� ���������� ������ �� ����.
        // � ��������� ������ ����� ����������� ���������� LexerError
        template <typename T>
        const T& Expect() const {
            using namespace std::literals;
            if (token_.Is<T>()) {
                return token_.As<T>();
            }
            else {
                throw LexerError("Not implemented"s);
            }
        }

        // ����� ���������, ��� ������� ����� ����� ��� T, � ��� ����� �������� �������� value.
        // � ��������� ������ ����� ����������� ���������� LexerError
        template <typename T, typename U>
        void Expect(const U& value) const {
            using namespace std::literals;
            if (token_.Is<T>()&&token_.As<T>().value == value) {
            }
            else {
                throw LexerError("Not implemented"s);
            }
        }

        // ���� ��������� ����� ����� ��� T, ����� ���������� ������ �� ����.
        // � ��������� ������ ����� ����������� ���������� LexerError
        template <typename T>
        const T& ExpectNext() {
            char c;
            input_.get(c);
            if (c == ' ' && TimeToCountInDedents == false) {
                while (c == ' ') {
                    input_.get(c);
                }
            }
            input_.putback(c);
            SetToken();
            using namespace std::literals;
            if (token_.Is<T>()) {
                return token_.As<T>();
            }
            else {
                throw LexerError("Not implemented"s);
            }
        }

        // ����� ���������, ��� ��������� ����� ����� ��� T, � ��� ����� �������� �������� value.
        // � ��������� ������ ����� ����������� ���������� LexerError
        template <typename T, typename U>
        void ExpectNext(const U& value) {
            using namespace std::literals;
            char c;
            input_.get(c);
            if (c == ' ' && TimeToCountInDedents == false) {
                while (c == ' ') {
                    input_.get(c);
                }
            }
            input_.putback(c);
            SetToken();
            if (token_.Is<T>()&& token_.As<T>().value == value) {
            }
            else {
                throw LexerError("Not implemented"s);
            }
        }

    private:
        // ���������� ��������� ����� ��������������
        std::istream& input_;
        Token token_;
        void SetToken();
        void ParseIds();
       // void ParseNumbers();
        void ParseString();
        void ParseCharLogicOPerations();
        bool CheckIds(std::string s) const;
        size_t Indent = 0;
        size_t Dedent = 0;
        bool TimeToCountInDedents=true;
        bool FirstConstruct = true;
        bool CommentFlag = false;
    };

}  // namespace parse