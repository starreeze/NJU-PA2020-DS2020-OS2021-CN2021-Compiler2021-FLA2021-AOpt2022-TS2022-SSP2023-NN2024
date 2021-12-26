#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <cstdio>
#include <cassert>
using namespace std;

// gtree definition
union ValType {
	int ival;
	float fval;
	char* sval;
};
struct Info {
	char* type;
	ValType val;
};
struct Node {
	int lineNo;
	Info info;
	struct Node* child;
	struct Node* next;
};
#define TYPE_IS(typename) (root->info.type == string(#typename))

enum Kind { BASIC, ARRAY, STRUCTURE, FUNCTION, ERROR };
enum BasicType { INT, FLOAT };
// for searching symbol in a scope
enum Status { NONEXIST, CURRENT, PREVIOUS };
// unamed scope in function definition is considered as FUNC
enum Scope { GLOBAL, FUNC, STRUCT };
struct Field;
class TypeBase;
typedef TypeBase* Type;
typedef vector<Field> FieldList;
typedef vector<int> Shape;

class TypeBase {
	Kind kind;
public:
	TypeBase(Kind k) : kind(k) {}
	Kind get_kind() const { return kind; }
	virtual BasicType& get_basic_type() { throw "type error"; }
	virtual Shape& get_array_shape() { throw "type error"; }
	virtual string& get_struct_name() { throw "type error"; }
	virtual Type& get_subtype() { throw "type error"; }
	virtual FieldList& get_field() { throw "type error"; }
	virtual bool skip_check() const { throw "type error"; }
};

class TypeBasic : public TypeBase {
	BasicType basicType;
public:
	TypeBasic(BasicType basic) : TypeBase(BASIC), basicType(basic) {}
	BasicType& get_basic_type() { return basicType; }
};

class TypeArray : public TypeBase {
	Type elemType;
	Shape shape;
public:
	TypeArray(Type elem, Shape s) : TypeBase(ARRAY), elemType(elem), shape(s) {}
	Type& get_subtype() { return elemType; }
	Shape& get_array_shape() { return shape; }
};

class TypeStruct : public TypeBase {
	string name;
	FieldList members;
public:
	TypeStruct(const string& n) : TypeBase(STRUCTURE), name(n) {}
	string& get_struct_name() { return name; }
	FieldList& get_field() { return members; }
};

class TypeFunc : public TypeBase {
	Type returnType;
	FieldList args;
public:
	TypeFunc(Type rt, FieldList arg) : TypeBase(FUNCTION), returnType(rt), args(arg) {}
	Type& get_subtype() { return returnType; }
	FieldList& get_field() { return args; }
};

class TypeError :public TypeBase {
	bool skip_checking;
public:
	TypeError(bool skip) : TypeBase(ERROR), skip_checking(skip) {}
	bool skip_check() const { return skip_checking; }
};

static bool type_match(Type a, Type b);
static void report_error(int type, int lineno, const char* info);

struct Field {
	string name;
	Type type;
	bool operator==(const Field& f) const {
		return type_match(type, f.type);
	}
};

struct SymEntry {
	Type type;
	int lineno;
	bool defined;
	SymEntry(Type t, int line) : type(t), lineno(line), defined(true) {}
	SymEntry(Type t, int line, bool def) : type(t), lineno(line), defined(def) {}
};
typedef unordered_map<string, SymEntry>::iterator TableIt;

class SymTable {
	vector<unordered_map<string, SymEntry>> table{ {} };
	vector<Scope> scope{ GLOBAL };
	pair<Status, TableIt> find_first_n(const string& name, int n) {
		assert(n > 0);
		for (int i = n - 1; i >= 0; --i) {
			TableIt pos = table[i].find(name);
			if (pos != table[i].end())
				return { i == n - 1 ? CURRENT : PREVIOUS, pos };
		}
		return { NONEXIST, {} };
	}
public:
	pair<Status, TableIt> find(const string& name) {
		return find_first_n(name, table.size());
	}
	pair<Status, TableIt> find_previous(const string& name) {
		return find_first_n(name, table.size() - 1);
	}
	void insert(const pair<string, SymEntry>& item) {
		table.back().insert(item);
	}
	void insert_previous(const pair<string, SymEntry>& item) {
		assert(table.size() > 1);
		table[table.size() - 2].insert(item);
	}
	void new_scope(Scope s) { table.push_back({}); scope.push_back(s); }
	void pop_scope() { table.pop_back(); scope.pop_back(); }
	Scope current_scope() const { return scope.back(); }
	void check_undefined() const {
		for (auto p : table[0])
			if (!p.second.defined)
				report_error(18, p.second.lineno, "Undefined function");
	}
};

extern SymTable symTable;

// ************** Above: error detection; Below: translation *****************

enum OpType { ID, IMMEDIATE, OPERATOR, SIZE };
enum IrType {
	LABEL,
	FUN,
	ASSIGN,
	ARITH, // +-*/
	GOTO,
	IF_GOTO,
	RETURN,
	DEC,
	ARG,
	CALL,
	PARAM,
	READ,
	WRITE
};

struct Oprand { // also operator
	string info; // all represented as string
	OpType type;
	bool address = false, value = false;
	Oprand() {}
	Oprand(const string& content, OpType opType, bool addr = false, bool val = false) : info(content), type(opType), address(addr), value(val) {}
	string repr() const {
		assert(!(address && value));
		string res;
		if (address)	res.push_back('&');
		if (value)	res.push_back('*');
		if (type == IMMEDIATE)	res.push_back('#');
		res += info;
		return res;
	}
	bool operator==(const Oprand& op) const { // name equal
		return info == op.info;
	}
	bool congruent(const Oprand& op) const {  // all equal
		return info == op.info && address == op.address && value == op.value;
	}
	bool is_temp() const {
		return info.back() == '\'';
	}
	static int label_state;
	static Oprand label(bool createNew = true) {
		label_state += createNew;
		Oprand oprand;
		oprand.type = ID;
		oprand.info = "l" + to_string(label_state) + "'";
		return oprand;
	}
	static int tempvar_state;
	static Oprand temp_var(bool createNew = true) {
		tempvar_state += createNew;
		Oprand oprand;
		oprand.type = ID;
		oprand.info = "t" + to_string(tempvar_state) + "'";
		return oprand;
	}
};

struct OpHash {
	unsigned long operator()(const Oprand& op) const {
		return hash<string>()(op.info);
	}
};

struct InterCode {
	vector<Oprand> oprands;
	IrType type;
	void print() const {
		switch (type) {
		case LABEL:
			assert(oprands.size() == 1);
			printf("LABEL %s :\n", oprands[0].repr().c_str());
			break;
		case FUN:
			assert(oprands.size() == 1);
			printf("FUNCTION %s :\n", oprands[0].repr().c_str());
			break;
		case ASSIGN:
			assert(oprands.size() == 2);
			printf("%s := %s\n", oprands[0].repr().c_str(), oprands[1].repr().c_str());
			break;
		case ARITH:
			assert(oprands.size() == 4);
			printf("%s := %s %s %s\n", oprands[0].repr().c_str(), oprands[1].repr().c_str(), oprands[2].repr().c_str(), oprands[3].repr().c_str());
			break;
		case GOTO:
			assert(oprands.size() == 1);
			printf("GOTO %s\n", oprands[0].repr().c_str());
			break;
		case IF_GOTO:
			assert(oprands.size() == 4);
			printf("IF %s %s %s GOTO %s\n", oprands[0].repr().c_str(), oprands[1].repr().c_str(), oprands[2].repr().c_str(), oprands[3].repr().c_str());
			break;
		case RETURN:
			assert(oprands.size() == 1);
			printf("RETURN %s\n", oprands[0].repr().c_str());
			break;
		case DEC:
			assert(oprands.size() == 2);
			printf("DEC %s %s\n", oprands[0].repr().c_str(), oprands[1].repr().c_str());
			break;
		case ARG:
			assert(oprands.size() == 1);
			printf("ARG %s\n", oprands[0].repr().c_str());
			break;
		case CALL:
			assert(oprands.size() == 2);
			printf("%s := CALL %s\n", oprands[0].repr().c_str(), oprands[1].repr().c_str());
			break;
		case PARAM:
			assert(oprands.size() == 1);
			printf("PARAM %s\n", oprands[0].repr().c_str());
			break;
		case READ:
			assert(oprands.size() == 1);
			printf("READ %s\n", oprands[0].repr().c_str());
			break;
		case WRITE:
			assert(oprands.size() == 1);
			printf("WRITE %s\n", oprands[0].repr().c_str());
			break;
		}

	}
};

extern list<InterCode> intercodes;
typedef list<InterCode>::iterator It;

#endif
