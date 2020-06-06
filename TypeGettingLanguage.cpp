#include <iostream>
#include <string>
#include <map>
#include <stack>

namespace Tag {
	enum {
		Type = 256, Id
	};
};

class Token {
public:
	Token() : kind(0) {}
	Token(int k) : kind(k) {}
	Token(int k, std::string val) : kind(k), value(val) {}
	int kind;
	std::string value; // type or id.
};

std::ostream& operator<<(std::ostream&, Token);

std::ostream& operator<<(std::ostream& out, Token token) {
	if (token.value != "") {
		out << "Token(" << token.kind << ", " << token.value << ")\n";
	}
	else if (token.kind != -1) {
		out << "Token('" << char(token.kind) << "')\n";
	}

	return out;
}

class Lexer {
public:
	Lexer();
	int line = 1;
	Token lex();
private:
	std::map<std::string, Token> words;
	char lookahead = ' ';
	void next();
	void add(std::string);
};

Lexer::Lexer() {
	add("int");
	add("bool");
	add("char");
}

void Lexer::add(std::string type) {
	words.insert(std::pair<std::string, Token>(type, Token(Tag::Type, type)));
}

void Lexer::next() {
	lookahead = getchar();
}

Token Lexer::lex() {
	if (lookahead == EOF) {
		return Token(-1);
	}
	else {
		while (lookahead == ' ' || lookahead == '\n' || lookahead == '\t') {
			next();
			if (lookahead == '\n') {
				line++;
			}
		}

		if ('a' <= lookahead && 'z' >= lookahead) {
			std::string value;
			do {
				value += lookahead;
				next();
			} while (('a' <= lookahead && 'z' >= lookahead) || ('0' <= lookahead && '9' >= lookahead));

			auto find = words.find(value);
			if (find != words.end()) {
				return find->second;
			}
			else {
				return Token(Tag::Id, value);
			}
		}

		Token result(lookahead);
		lookahead = ' ';
		return result;
	}
}

class Env {
public:
	Env() = default;
	std::string find(std::string);
	void push(std::string, std::string);
private:
	std::map<std::string, std::string> ids;
};

void Env::push(std::string type, std::string name) {
	ids.insert(std::pair<std::string, std::string>(name, type));
}

std::string Env::find(std::string name) {
	auto found = ids.find(name);
	if (found != ids.end()) {
		return found->second;
	}
	else {
		return "";
	}
}

/*
code -> block
block -> {stmts}
stmts -> stmt stmts | empty
stmt -> id; | type id; | block | empty
*/

class Parser {
public:
	Parser() = default;
	Lexer lexer;
	void code();
	void print_result();
private:
	std::string result;
	std::stack<Env> stack;
	int tabs = 0;
	Token now = lexer.lex();
	Token match(int, std::string);
	void print_tabs();
	void block();
	void stmts();
	void stmt();
	void error(std::string);
};

void Parser::error(std::string message) {
	std::cerr << "SyntaxError: " << message << " on line " << lexer.line << ".\n";
	exit(0);
}

Token Parser::match(int kind, std::string message) {
	if (now.kind == kind) {
		Token t = now;
		now = lexer.lex();
		return t;
	}
	else {
		error(message);
	}
}

void Parser::print_tabs() {
	for (int i = 1; i <= tabs; i++) {
		result += '\t';
	}
}

void Parser::code() {
	block();
}

void Parser::block() {
	match('{', "Expect a '{'");
	print_tabs();
	result += "{\n";
	tabs++;
	stack.push(Env());
	stmts();
	match('}', "Expect a '}'");
	tabs--;
	print_tabs();
	result += "}\n";
	stack.pop();
}

void Parser::stmts() {
	if (now.kind == '{' || now.kind == Tag::Type || now.kind == Tag::Id) {
		stmt();
		stmts();
	}
}

void Parser::stmt() {
	if (now.kind == '{') {
		block();
	}
	else if (now.kind == Tag::Type) { // type id;
		Token type = match(Tag::Type, "");
		Token name = match(Tag::Id, "Expect a id after a type");
		std::string find = stack.top().find(name.value);
		if (find != "") {
			error("Id was defined");
		}
		stack.top().push(type.value, name.value);
		match(';', "Expect a ';'");
	}
	else if (now.kind = Tag::Id) { // id;
		Token name = match(Tag::Id, "");
		std::string find = stack.top().find(name.value);
		if (find == "") {
			std::stack<Env> temp;
			while (!stack.empty() && find == "") {
				find = stack.top().find(name.value);
				temp.push(stack.top());
				stack.pop();
			}

			while (!temp.empty()) {
				stack.push(temp.top());
				temp.pop();
			}
			if (find == "") {
				error("Id not found");
			}
		}

		print_tabs();
		result += name.value + ": " + find + ";\n";
		match(';', "Expect a ';'");
	}
}

void Parser::print_result() {
	std::cout << result;
}

int main() {
	Parser parser;
	parser.code();
	parser.print_result();

	return 0;
}