/*
	calculator08buggy.cpp | From: Programming -- Principles and Practice Using C++, by Bjarne Stroustrup
	We have inserted 3 bugs that the compiler will catch and 3 that it won't.
*/

#include "std_lib_facilities.h"

struct Token {
	char kind;
	double value;
	string name;
	Token(char ch) :kind(ch), value(0) { }
	Token(char ch, string s): kind(ch), name(s){} //Line was missing in original 
	Token(char ch, double val) :kind(ch), value(val) { }
};

class Token_stream {
	bool full;
	Token buffer;
public:
	Token_stream() :full(0), buffer(0) { }

	Token get();
	void unget(Token t) { buffer = t; full = true; }

	void ignore(char);
};

const char let = 'L';
const char quit = 'Q';
const char print = ';';
const char number = '8';
const char name = 'a';
const char squarert = 's';
const char power = 'P';
const char constant = 'C';

Token Token_stream::get()
{
	if (full) { full = false; return buffer; }
	char ch;
	cin >> ch;
	switch (ch) {
	case '(':
	case ')':
	case '+':
	case '-':
	case '*':
	case '/':
	case '%':
	case ';':
	case '=':
	case ',':
		return Token(ch);
	case '#':
	{
		return Token(let);
	}
	case '.':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	{	cin.unget();
	double val;
	cin >> val;
	return Token(number, val);
	}
	default:
		if (isalpha(ch) || ch == '_') {
			string s;
			s += ch;
			while (cin.get(ch) && (isalpha(ch) || isdigit(ch) || ch == '_')) s += ch;
			cin.unget();
			if (s == "let") return Token(let);
			if (s == "exit" || s == "quit") return Token(quit); //Error: returned Token(name)
			if (s == "sqrt") return Token(squarert);
			if (s == "pow") return Token(power);
			if (s == "const") return Token(constant); 
			return Token(name, s);
		}
		error("Bad token");
	}
}

void Token_stream::ignore(char c)
{
	if (full && c == buffer.kind) {
		full = false;
		return;
	}
	full = false;

	char ch;
	while (cin >> ch)
		if (ch == c) return;
}

struct Variable {
	string name;
	double value;
	bool constant;
	Variable(string n, double v, bool c = true) :name(n), value(v), constant(c) { }
};

vector<Variable> names;

double get_value(string s)
{
	for (int i = 0; i < names.size(); ++i)
		if (names[i].name == s) return names[i].value;
	error("get: undefined name ", s);
}

void set_value(string s, double d)
{
	for (int i = 0; i <= names.size(); ++i)
		if (names[i].name == s && names[i].constant == false) error(s, " is a constant");
		else if (names[i].name == s && names[i].constant == true) {
			names[i].value = d;
			return;
		}
	error("set: undefined name ", s);
}

bool is_declared(string s)
{
	for (int i = 0; i < names.size(); ++i)
	{
  		if (names[i].name == s) return true;
 	}
	return false;
}

Token_stream ts;

double define_name(string s, double val, bool var=true)
{
    if (is_declared(s)) error(s," declared twice");
    names.push_back(Variable(s,val,var));
    return val;
}

double expression();

double primary()
{
	Token t = ts.get();
	switch (t.kind) {
	case '(':
	{	double d = expression();
	t = ts.get();
	if (t.kind != ')') error("')' expected");
	return d;
	}
	case '-':
		return -primary();
	case '+':
		return primary();
	case number:
		return t.value;
	case squarert:
	{
		t = ts.get();
		if(t.kind != '(') error("'(' expected");
		ts.unget(t);
		double d = primary();
		if (d < 0) error("You cannot have a square root of a negative number!");
		return sqrt(d);
	}
	case power:
	{
		t = ts.get();
		if(t.kind != '(') error("'(' expected");
		double x = expression();
		t = ts.get();
		if(t.kind != ',') error("',' expected");
		int i = expression();
		t = ts.get();
		if (t.kind != ')') error("')' expected");
		return pow(x, i);
	}
	case name:
		{
			Token next = ts.get();
			if (next.kind == '=') {	// handle name = expression
				double d = expression();
				set_value(t.name,d);
				return d;
			}
			else {
				ts.unget(next);		// not an assignment: return the value
				return get_value(t.name); // return the variable's value
			}
		}
	default:
		error("primary expected");
	}
}

double term()
{
	double left = primary();
	while (true) {
		Token t = ts.get();
		switch (t.kind) {
		case '*':
			left *= primary();
			break;
		case '/':
		{	double d = primary();
			if (d == 0) error("divide by zero");
			left /= d;
			break;
		}
		case '%':
   		{
    		double d = primary();
    		if (d == 0) error("%: divide by zero");
    		left = fmod(left, d);
    		break;
   		}
		default:
			ts.unget(t);
			return left;
		}
	}
}

double expression()
{
	double left = term();
	Token t = ts.get();
	while (true) {
		switch (t.kind) {
		case '+':
			left += term();
			break;
		case '-':
			left -= term();
			break;
		default:
			ts.unget(t);
			return left;
		}
	}
}

double declaration(Token k)
{
	Token t = ts.get();
    if (t.kind != name) error ("name expected in declaration");
    string var_name = t.name;

    Token t2 = ts.get();
    if (t2.kind != '=') error("= missing in declaration of ", var_name);

    double d = expression();
    define_name(var_name ,d, k.kind == let);
    return d;
}

double statement()
{
	Token t = ts.get();
	switch (t.kind) {
	case let:
	case constant: 
		return declaration(t.kind);
	default:
		ts.unget(t);
		return expression();
	}
}

void clean_up_mess()
{
	ts.ignore(print);
}

const string prompt = "> ";
const string result = "= ";

void calculate()
{
	while (true) try {
		cout << prompt;
		Token t = ts.get();
		while (t.kind == print) t = ts.get();
		if (t.kind == quit) return;
		ts.unget(t);
		cout << result << statement() << endl;
	}
	catch (runtime_error& e) {
		cerr << e.what() << endl;
		clean_up_mess();
	}
}

int main()

try {
	define_name("pi", 3.14159265359, false);
	define_name("k", 1000, false);
	define_name("e", 2.71828182846, false);

	calculate();
	return 0;
}
catch (exception& e) {
	cerr << "exception: " << e.what() << endl;
	char c;
	while (cin >> c && c != ';');
	return 1;
}
catch (...) {
	cerr << "exception\n";
	char c;
	while (cin >> c && c != ';');
	return 2;
}