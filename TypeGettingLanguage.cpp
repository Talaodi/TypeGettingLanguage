#include <iostream>
#include <string>
#include <cstdio>
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
	int kind; // char or 256(Type), 257(Id)
	std::string value; // type or id.
};

std::ostream& operator<<(std::ostream&, Token); // to use cout

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
	int line = 1; // for error message
	Token lex(); // get the next token
private:
	std::map<std::string, Token> words; // keywords
	char lookahead = ' '; // char it reads now
	void next(); // get the next char
	void add(std::string); // add new keywords
};

Lexer::Lexer() {
	// keywords: int, bool, char
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
		// finish reading
		return Token(-1);
	}
	else {
		// whitespace char
		while (lookahead == ' ' || lookahead == '\n' || lookahead == '\t') {
			next();
			if (lookahead == '\n') {
				line++;
			}
		}

		// letter
		if ('a' <= lookahead && 'z' >= lookahead) {
			std::string value;
			do {
				value += lookahead;
				next();
			} while (('a' <= lookahead && 'z' >= lookahead) || ('0' <= lookahead && '9' >= lookahead)); // letter or digit

			auto find = words.find(value); // if value is a keyword
			if (find != words.end()) {
				return find->second;
			}
			else {
				return Token(Tag::Id, value);
			}
		}

		// others
		Token result(lookahead);
		lookahead = ' ';
		return result;
	}
}

// the symbol table
class Env {
public:
	Env() = default;
	std::string find(std::string); // name -> type
	void push(std::string, std::string); // add (type, name)
private:
	std::map<std::string, std::string> ids; // the symbol table
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
		return ""; // return empty string if name is not found
	}
}

/*
syntax:
code -> block
block -> {stmts}
stmts -> stmt stmts | empty
stmt -> id; | type id; | block | empty
*/

class Parser {
public:
	Parser() = default;
	Lexer lexer; // to get tokens
	void code(); // entrance
	void print_result(); // print result
private:
	std::string result; // output
	std::stack<Env> stack; // the symbol table
	int tabs = 0;
	Token now = lexer.lex(); // token it reads now
	Token match(int, std::string); // match a token, or throw an error
	void print_tabs(); // style
	// look at syntax
	void block();
	void stmts();
	void stmt(); 
	void error(std::string); // throw an error
};

void Parser::error(std::string message) {
	std::cerr << "SyntaxError: " << message << " on line " << lexer.line << ".\n";
	exit(0); // exit
}

Token Parser::match(int kind, std::string message) {
	if (now.kind == kind) { // match
		Token t = now;
		now = lexer.lex();
		return t; // return the token it matches
	}
	else {
		error(message);
	}
}

void Parser::print_tabs() {
	// add tabs * '\t'
	for (int i = 1; i <= tabs; i++) {
		result += '\t';
	}
}

void Parser::code() {
	// code -> block
	block();
}

void Parser::block() {
	// block -> {stmts}
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
		// stmts -> stmt stmts
		stmt();
		stmts();
	}
	// empty
}

void Parser::stmt() {
	if (now.kind == '{') {
		// stmt -> block
		block();
	}
	else if (now.kind == Tag::Type) { 
		// stmt -> type id;
		// match
		Token type = match(Tag::Type, "");
		Token name = match(Tag::Id, "Expect a id after a type");
		match(';', "Expect a ';'");

		std::string find = stack.top().find(name.value);
		if (find != "") {
			// id was defined
			error("Id was defined");
		}
		// new id
		stack.top().push(type.value, name.value);
	}
	else if (now.kind = Tag::Id) { 
		// stmt -> id;
		// match
		Token name = match(Tag::Id, "");
		match(';', "Expect a ';'");
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
				// can not found id
				error("Id not found");
			}
		}

		print_tabs();
		result += name.value + ": " + find + ";\n";
	}
	// stmt -> empty
}

void Parser::print_result() {
	std::cout << result;
}

int main(int argc, char** argv) {
	freopen(argv[1], "r", stdin);
	
	Parser parser;
	parser.code();
	parser.print_result();

	return 0;
}
