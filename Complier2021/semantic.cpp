#include "semantic.h"
#include <algorithm>
#include <numeric>

static void ext_def_list(Node* root);
static void ext_def(Node* root);
static void ext_dec_list(Node* root, Type spec);
static void fun_dec(Node* root, Type spec, bool def);
static void var_list(Node* root, FieldList& fieldList);
static void param_dec(Node* root, FieldList& fieldList);
static void comp_st(Node* root, Type spec);
static void stmt_list(Node* root, Type spec);
static void stmt(Node* root, Type spec);
static Type specifier(Node* root);
static Type struct_specifier(Node* root);
static void def_list(Node* root, FieldList* fieldList);
static void def(Node* root, FieldList* fieldList);
static void dec_list(Node* root, FieldList* fieldList, Type spec);
static void dec(Node* root, FieldList* fieldList, Type spec);
static Type var_dec(Node* root, FieldList* fieldList, Type spec, bool param = false);
static Type exp(Node* root, bool* rv = nullptr, int arrDepth = 0, int struDepth = 0);
static void args(Node* root, FieldList& fieldList);

SymTable symTable;
list<InterCode> intercodes;

static void report_error(int type, int lineno, const char* info) {
	printf("Error type %d at Line %d: %s.\n", type, lineno, info);
}

int Oprand::label_state = 0, Oprand::tempvar_state = 0;
static int unamed_type_state = 0;
static string unamed_type() { return to_string(unamed_type_state++); }

static bool type_match(Type a, Type b) {
	// skip checking if NULL (since NULL is returned on definition)
	if (!(a && b))	return true;
	// return true if either of TypeError suppress type checking
	if (b->get_kind() == ERROR && b->skip_check())
		return true;
	switch (a->get_kind()) {
	case BASIC:
		return b->get_kind() == BASIC && a->get_basic_type() == b->get_basic_type();
	case ARRAY:
		return b->get_kind() == ARRAY && a->get_array_shape().size() == b->get_array_shape().size() && type_match(a->get_subtype(), b->get_subtype());
	case STRUCTURE:
		// call Field::operator== recursively
		return b->get_kind() == STRUCTURE && a->get_field() == b->get_field();
	case FUNCTION:
		return b->get_kind() == FUNCTION && a->get_field() == b->get_field() && type_match(a->get_subtype(), b->get_subtype());
	case ERROR:
		return a->skip_check();
	default:
		throw "type error";
	}
}

static int size_of(Type t) {
	int size;
	switch (t->get_kind()) {
	case BASIC:
		return 4;
	case ARRAY:
		size = 1;
		for (int len : t->get_array_shape())
			size *= len;
		return size * size_of(t->get_subtype());
	case STRUCTURE:
		size = 0;
		for (const Field& f : t->get_field())
			size += size_of(f.type);
		return size;
	default:
		throw "type error";
	}
}

static void ext_def_list(Node* root) {
	if (!root)	return;
	root = root->child;
	ext_def(root);
	ext_def_list(root->next);
}

static void ext_def(Node* root) {
	root = root->child;
	Type spec = specifier(root);
	if (root->next->next) {
		root = root->next;
		if TYPE_IS(ExtDecList)
			ext_dec_list(root, spec);
		else if (root->next->info.type == string("SEMI")) {
			// function declaration
			symTable.new_scope(FUNC);
			fun_dec(root, spec, false);
			symTable.pop_scope();
		}
		else { // function definition
			symTable.new_scope(FUNC);
			fun_dec(root, spec, true);
			comp_st(root->next, spec);
			symTable.pop_scope();
		}
	}
}

static void fun_dec(Node* root, Type spec, bool def) {
	root = root->child;
	string id = root->info.val.sval;
	auto res = symTable.find_previous(id);
	FieldList fieldList;
	if (root->next->next->next) // VarList
		var_list(root->next->next, fieldList);
	Type current = new TypeFunc(spec, fieldList);
	if (res.first == CURRENT) {
		if (res.second->second.type->get_kind() != FUNCTION || res.second->second.defined && def)
			report_error(4, root->lineNo, "Redefinition of identifier");
		// declared but not defined: type match checking
		else if (!type_match(current, res.second->second.type))
			report_error(19, root->lineNo, "Inconsistant function declaration/definition");
		else // set defined to true if in definition
			res.second->second.defined |= def;
			// ignore definition here in translation (since there shouldn't be any declaration)
	}
	else { // no entry in symtable
		symTable.insert_previous({ id, SymEntry{current, root->lineNo, def} });
		intercodes.push_back({ {Oprand(id, ID)}, FUN });
		for (const Field& f : fieldList)
			intercodes.push_back({ {Oprand(f.name, ID)}, PARAM });
	}
}

static void var_list(Node* root, FieldList& fieldList) {
	root = root->child;
	param_dec(root, fieldList);
	if (root->next)
		var_list(root->next->next, fieldList);
}

static void param_dec(Node* root, FieldList& fieldList) {
	root = root->child;
	Type spec = specifier(root);
	var_dec(root->next, &fieldList, spec, true);
}

static void comp_st(Node* root, Type spec) {
	root = root->child->next;
	if TYPE_IS(DefList) {
		def_list(root, nullptr);
		root = root->next;
	}
	if TYPE_IS(StmtList)
		stmt_list(root, spec);
	// else // function emtpy; void return type disallowed
		// report_error(8, root->lineNo, "Return type mismatch");
}

static void stmt_list(Node* root, Type spec) {
	if (root) {
		root = root->child; // Def
		stmt(root, spec);
		stmt_list(root->next, spec);
	}
}

static void cond(Node* root, const Oprand& ltrue, const Oprand& lfalse) {
	Node* current = root;
	root = root->child;
	if TYPE_IS(NOT)
		return cond(root->next, lfalse, ltrue);
	Node* first = root;
	root = root->next;
	Node* second = root->next;
	if TYPE_IS(RELOP) {
		exp(first);
		Oprand firstVal = Oprand::temp_var(false);
		exp(second);
		intercodes.push_back({ {firstVal, Oprand(root->info.val.sval, OPERATOR), Oprand::temp_var(false), ltrue}, IF_GOTO });
		intercodes.push_back({ {lfalse}, GOTO });
	}
	else if TYPE_IS(AND) {
		Oprand l1 = Oprand::label();
		cond(first, l1, lfalse);
		intercodes.push_back({ {l1}, LABEL });
		cond(second, ltrue, lfalse);
	}
	else if TYPE_IS(OR) {
		Oprand l1 = Oprand::label();
		cond(first, ltrue, l1);
		intercodes.push_back({ {l1}, LABEL });
		cond(second, ltrue, lfalse);
	}
	else {
		exp(current);
		intercodes.push_back({ {Oprand::temp_var(false), Oprand("!=", OPERATOR), Oprand("0", IMMEDIATE), ltrue}, IF_GOTO });
		intercodes.push_back({ {lfalse}, GOTO });
	}
}

static void stmt(Node* root, Type spec) {
	root = root->child;
	if (!root->next) { // CompSt
		symTable.new_scope(FUNC);
		comp_st(root, spec);
		symTable.pop_scope();
		return;
	}
	if TYPE_IS(Exp) {
		exp(root);
		return;
	}
	if TYPE_IS(RETURN) {
		Type retType = exp(root->next);
		if (!type_match(retType, spec))
			report_error(8, root->lineNo, "Return type mismatch");
		// get last temp variable from exp
		intercodes.push_back({ {Oprand::temp_var(false)}, RETURN });
		return;
	}
	// translate flow control
	Node* ptr = root->next->next;
	if TYPE_IS(WHILE) {
		Oprand l1 = Oprand::label(), l2 = Oprand::label(), l3 = Oprand::label();
		intercodes.push_back({ {l1}, LABEL });
		cond(ptr, l2, l3);
		intercodes.push_back({ {l2}, LABEL });
		stmt(ptr->next->next, spec);
		intercodes.push_back({ {l1}, GOTO });
		intercodes.push_back({ {l3}, LABEL });
	}
	else if (ptr->next->next->next) { // if-else
		Oprand l1 = Oprand::label(), l2 = Oprand::label(), l3 = Oprand::label();
		cond(ptr, l1, l2);
		intercodes.push_back({ {l1}, LABEL });
		stmt(ptr->next->next, spec);
		intercodes.push_back({ {l3}, GOTO });
		intercodes.push_back({ {l2}, LABEL });
		stmt(ptr->next->next->next->next, spec);
		intercodes.push_back({ {l3}, LABEL });
	}
	else { // if
		Oprand l1 = Oprand::label(), l2 = Oprand::label();
		cond(ptr, l1, l2);
		intercodes.push_back({ {l1}, LABEL });
		stmt(ptr->next->next, spec);
		intercodes.push_back({ {l2}, LABEL });
	}
	/*
	Type logical = exp(ptr);
	if (!(logical->get_kind() == ERROR && logical->skip_check() || logical->get_kind() == BASIC && logical->get_basic_type() == INT))
		report_error(20, ptr->lineNo, "Type incorrect in flow control");
	ptr = ptr->next->next;
	stmt(ptr, spec);
	if (ptr->next)
		stmt(ptr->next->next, spec);
	*/
}

static void ext_dec_list(Node* root, Type spec) {
	root = root->child;
	var_dec(root, nullptr, spec);
	if (root->next)
		ext_dec_list(root->next->next, spec);
}

static Type specifier(Node* root) {
	root = root->child;
	if TYPE_IS(TYPE)
		return new TypeBasic(root->info.val.sval == string("int") ? INT : FLOAT);
	else // structSpecifier
		return struct_specifier(root);
}

static Type struct_specifier(Node* root) {
	root = root->child->next;
	if TYPE_IS(Tag) { // in reference
		string idName = root->child->info.val.sval;
		auto pos = symTable.find(idName);
		if (pos.first == NONEXIST) {
			report_error(17, root->lineNo, "Undefined identifier");
			return new TypeError(true);
		}
		return pos.second->second.type;
	}
	// in definition
	string idName;
	if TYPE_IS(LC) // OptTag is NULL
		idName = unamed_type();
	else { // OptTag
		idName = root->child->info.val.sval;
		root = root->next;
	} // LC <- root
	root = root->next; // DefList | RC <- root
	auto pos = symTable.find(idName);
	if (pos.first != CURRENT) {
		Type current = new TypeStruct(idName);
		symTable.insert({ idName, SymEntry{current, root->lineNo} });
		if TYPE_IS(DefList) {
			symTable.new_scope(STRUCT);
			def_list(root, &current->get_field());
			symTable.pop_scope();
		}
		return current;
	}
	report_error(16, root->lineNo, "Redefinition of identifier");
	return new TypeError(true);

}

static void def_list(Node* root, FieldList* fieldList) {
	// insert all fields into current & modify symtable
	if (root) {
		root = root->child; // Def
		def(root, fieldList);
		def_list(root->next, fieldList);
	}
}

static void def(Node* root, FieldList* fieldList) {
	Type spec = specifier(root->child);
	dec_list(root->child->next, fieldList, spec);
}

static void dec_list(Node* root, FieldList* fieldList, Type spec) {
	// if spec is not null, a new entry in symTable is being created
	if (root) {
		root = root->child; // Dec
		dec(root, fieldList, spec);
		if (root->next)
			dec_list(root->next->next, fieldList, spec);
	}
}

static void dec(Node* root, FieldList* fieldList, Type spec) {
	root = root->child; // VarDec
	// XXX modify in lab2
	var_dec(root, fieldList, spec);
	if (root->next) { // assignop
		if (symTable.current_scope() == STRUCT)
			report_error(15, root->lineNo, "assignment in struct");
		Type right = exp(root->next->next);
		if (!type_match(spec, right))
			report_error(5, root->lineNo, "Type mismatch in assignment");
		// only int can be assigned a value in definition
		assert(spec->get_kind() == BASIC && spec->get_basic_type() == INT);
		// VarDec <- root
		intercodes.push_back({ {Oprand(root->child->info.val.sval, ID), Oprand::temp_var(false)}, ASSIGN });
	}
}

static string var_dec_rec(Node* root, Shape& shape) {
	// return id name and fill in array shape
	if (root->next) {
		string name = var_dec_rec(root->child, shape);
		shape.push_back(root->next->next->info.val.ival);
		return name;
	}
	return root->info.val.sval;
}

static Type var_dec(Node* root, FieldList* fieldList, Type spec, bool param) {
	root = root->child; // ID | VarDec
	Shape array_shape;
	string name = var_dec_rec(root, array_shape);
	auto pos = symTable.find(name);
	if (spec) { // new table entry
		if (pos.first == CURRENT)
			report_error(symTable.current_scope() == STRUCT ? 15 : 3, root->lineNo, "Redefinition of identifier");
		else {
			if (array_shape.size()) { // array
				Type current = new TypeArray(spec, array_shape);
				if (fieldList)
					fieldList->push_back({ name, current });
				if (!param && symTable.current_scope() != STRUCT) {
					// only alloc space if in function scope and not in function params
					intercodes.push_back({ {Oprand::temp_var(), Oprand(to_string(size_of(current)), SIZE)}, DEC });
					Oprand op = Oprand::temp_var(false);
					op.address = true;
					// variable after DEC, i.e. array and structure, should be represented in address
					intercodes.push_back({ {Oprand(name, ID), op} , ASSIGN });
				}
				symTable.insert({ name, SymEntry{current, root->lineNo} });
			}
			else { // basic or struct
				if (fieldList)
					fieldList->push_back({ name, spec });
				if (spec->get_kind() == STRUCTURE && !param && symTable.current_scope() != STRUCT) {
					intercodes.push_back({ {Oprand::temp_var(), Oprand(to_string(size_of(spec)), SIZE)}, DEC });
					Oprand op = Oprand::temp_var(false);
					op.address = true;
					intercodes.push_back({ {Oprand(name, ID), op} , ASSIGN });
				}
				symTable.insert({ name, SymEntry{spec, root->lineNo} });
			}
		}
		return nullptr;
	}
	// in exp
	if (pos.first == NONEXIST) {
		report_error(1, root->lineNo, "Undefined identifier");
		return new TypeError(true);
	}
	if (array_shape.size() && pos.second->second.type->get_kind() != ARRAY) {
		// subscription but not array
		report_error(10, root->lineNo, "Identifier not an array");
		return new TypeError(true);
	}
	return pos.second->second.type;
}

static Type exp_id(vector<Node*>& terms, bool* rv, int arrDepth, int struDepth) {
	auto pos = symTable.find(terms[0]->info.val.sval);
	if (pos.first == NONEXIST) {
		report_error(1, terms[0]->lineNo, "Undefined identifier");
		return new TypeError(true);
	}
	// if leftval and not array ref or structure (because they two are already represented in address), take address
	// intercode of a = b: t = &a; *t = b. Leave it to optimization.
	intercodes.push_back({ {Oprand::temp_var(), Oprand(terms[0]->info.val.sval, ID, rv && !arrDepth && !struDepth)}, ASSIGN });
	return pos.second->second.type;
}

static Type exp_basic(vector<Node*>& terms, bool* rv) {
	if (rv)	*rv = true;
	string terms0 = terms[0]->info.type;
	if (terms0 == "INT") {
		intercodes.push_back({ {Oprand::temp_var(), Oprand(to_string(terms[0]->info.val.ival), IMMEDIATE)}, ASSIGN });
		return new TypeBasic(INT);
	}
	return new TypeBasic(FLOAT);
}

static Type exp_unary(vector<Node*>& terms, bool* rv) {
	if (rv)	*rv = true;
	Type oprand = exp(terms[1]);
	string terms0 = terms[0]->info.type;
	if (oprand->get_kind() != BASIC || terms0 == "NOT" && oprand->get_basic_type() != INT) {
		report_error(7, terms[1]->lineNo, "Type mismatch in experssion");
		return new TypeError(true);
	}
	// use 0 - x to translate -x; NOT should not appear here in translation
	assert(terms0 == "MINUS");
	Oprand op = Oprand::temp_var(false);
	intercodes.push_back({ {Oprand::temp_var(), Oprand("0", IMMEDIATE), Oprand("-", OPERATOR), op}, ARITH });
	return oprand;
}

static Type exp_assign(vector<Node*>& terms, bool* rv) {
	if (rv)	*rv = true;
	bool is_right_value = false;
	Type left = exp(terms[0], &is_right_value);
	Oprand leftVal = Oprand::temp_var(false);
	Type right = exp(terms[2]);
	if (is_right_value) {
		report_error(6, terms[1]->lineNo, "Assigning to right value");
		return new TypeError(true);
	}
	if (left->get_kind() != FUNCTION && type_match(left, right)) {
		// get value; leftVal is an address
		leftVal.value = true;
		Oprand rightVal = Oprand::temp_var(false);
		/*
		if (right->get_kind() == ARRAY || right->get_kind() == STRUCTURE)
			// get value; they two only calculate address
			rightVal.value = true;*/
		intercodes.push_back({ {leftVal, rightVal}, ASSIGN });
		return left;
	}
	report_error(5, terms[1]->lineNo, "Type mismatch in assignment");
	return new TypeError(true);
}

static Type exp_binary(vector<Node*>& terms, bool* rv) {
	if (rv)	*rv = true;
	Type left = exp(terms[0]);
	Oprand leftVal = Oprand::temp_var(false);
	Type right = exp(terms[2]);
	string terms1 = terms[1]->info.type;
	if (left->get_kind() == ERROR && left->skip_check() || left->get_kind() == BASIC && !(left->get_basic_type() == FLOAT && (terms1 == "AND" || terms1 == "OR")) && type_match(left, right)) {
		if (terms1 == "RELOP") {
			return new TypeBasic(INT);
		}
		Oprand op = Oprand::temp_var(false);
		intercodes.push_back({ {Oprand::temp_var(), leftVal, Oprand(terms[1]->info.val.sval, OPERATOR), op}, ARITH });
		return left;
	}
	report_error(7, terms[1]->lineNo, "Type mismatch in experssion");
	return new TypeError(true);
}

static Type exp_func(vector<Node*>& terms, bool* rv) {
	if (rv)	*rv = true;
	string id = terms[0]->info.val.sval;
	if (id == "read")
		intercodes.push_back({ {Oprand::temp_var()}, READ });
	else if (id == "write") {
		exp(terms[2]->child);
		intercodes.push_back({ {Oprand::temp_var(false)}, WRITE });
	}
	else {
		auto pos = symTable.find(terms[0]->info.val.sval);
		if (pos.first == NONEXIST) {
			report_error(2, terms[0]->lineNo, "Undefined identifier");
			return new TypeError(true);
		}
		if (pos.second->second.type->get_kind() != FUNCTION) {
			report_error(11, terms[0]->lineNo, "Call on non-function type");
			return new TypeError(true);
		}
		FieldList fieldList;
		if (terms[2]->info.type == string("Args"))
			args(terms[2], fieldList);
		if (fieldList != pos.second->second.type->get_field())
			// we still return the correct return type although args are wrong
			report_error(9, terms[0]->lineNo, "Args mismatch in function call");
		intercodes.push_back({ {Oprand::temp_var(), Oprand(id, ID)}, CALL });
		return pos.second->second.type->get_subtype();
	}
	return new TypeBasic(INT);
}

static Type exp_array(vector<Node*>& terms, bool* rv, int arrDepth) {
	// array reference, generate intercode to calculate address
	Type base;
	base = exp(terms[0], rv, arrDepth + 1);
	if (base->get_kind() != ARRAY) {
		report_error(10, terms[0]->lineNo, "Subscription on non-array type");
		return new TypeError(true);
	}
	Oprand baseValue = Oprand::temp_var(false);
	Type offset = exp(terms[2]);
	if (!(offset->get_kind() == BASIC && offset->get_basic_type() == INT))
		report_error(12, terms[2]->lineNo, "Non-integer array subscription");
	Shape shape = base->get_array_shape();
	int size = 1;
	for (int i = 1; i < shape.size(); ++i)
		size *= shape[i];
	size *= size_of(base->get_subtype());
	Oprand rightVal = Oprand::temp_var(false);
	intercodes.push_back({ {Oprand::temp_var(), rightVal, Oprand("*", OPERATOR), Oprand(to_string(size), IMMEDIATE)}, ARITH });
	rightVal = Oprand::temp_var(false);
	intercodes.push_back({ {Oprand::temp_var(), baseValue, Oprand("+", OPERATOR), rightVal}, ARITH });
	shape = base->get_array_shape();
	shape.erase(shape.begin());
	Type elem = base->get_subtype();
	if (shape.size())
		return new TypeArray(elem, shape);
	// take value in rvalue exp if array ref address is fully calculated
	// and element type is basic (otherwise wrong if conbined with struct)
	if (!rv && elem->get_kind() == BASIC) {
		Oprand addr = Oprand::temp_var(false);
		addr.value = true;
		intercodes.push_back({ {Oprand::temp_var(), addr}, ASSIGN });
	}
	return elem;
}

static Type exp_struct(vector<Node*>& terms, bool* rv, int struDepth) {
	// structure reference, generate intercode to calculate address
	Type base = exp(terms[0], rv, 0, struDepth + 1);
	if (base->get_kind() != STRUCTURE) {
		if (base->get_kind() != ERROR || !base->skip_check())
			report_error(13, terms[0]->lineNo, "\'.\' operator on non-structure type");
		return new TypeError(true);
	}
	const FieldList& fieldList = base->get_field();
	auto p = fieldList.begin();
	int offset = 0;
	for (; p != fieldList.end() && p->name != terms[2]->info.val.sval; ++p)
		offset += size_of(p->type);
	if (p == fieldList.end()) {
		report_error(14, terms[2]->lineNo, "Identifier not a member of structure");
		return new TypeError(true);
	}
	Oprand op = Oprand::temp_var(false);
	intercodes.push_back({ {Oprand::temp_var(), op, Oprand("+", OPERATOR), Oprand(to_string(offset), IMMEDIATE)}, ARITH });
	if (!rv && !struDepth && p->type->get_kind() == BASIC) {
		Oprand addr = Oprand::temp_var(false);
		addr.value = true;
		intercodes.push_back({ {Oprand::temp_var(), addr}, ASSIGN });
	}
	return p->type;
}

static Type exp(Node* root, bool* rv, int arrDepth, int struDepth) {
	// rv: whether the current exp return a right value
	// set to null if not in the left of assignop (default)
	// arrDepth: the depth of current array. x[1]|[2]                   x[1][2]
	// depth of inner most is 0, e.g.        ~~~~       depth = 0, and  ~~~~~~~ depth = 1
	// baseType: 
	root = root->child;
	vector<Node*> terms;
	while (root) {
		terms.push_back(root);
		root = root->next;
	}
	assert(terms.size());
	string terms0 = terms[0]->info.type;
	if (terms.size() == 1) {
		if (terms0 == "ID")
			return exp_id(terms, rv, arrDepth, struDepth);
		return exp_basic(terms, rv);
	}
	if (terms0 == "MINUS" || terms0 == "NOT")
		return exp_unary(terms, rv);
	string terms1 = terms[1]->info.type;
	if (terms1 == "ASSIGNOP")
		return exp_assign(terms, rv);
	if (terms1 == "AND" || terms1 == "OR" || terms1 == "RELOP" || terms1 == "PLUS" || terms1 == "MINUS" || terms1 == "STAR" || terms1 == "DIV")
		return exp_binary(terms, rv);
	if (terms0 == "LP")
	// XXX modify in lab2
		return exp(terms[1], rv);
	if (terms0 == "ID")
		return exp_func(terms, rv);
	if (terms1 == "LB")
		return exp_array(terms, rv, arrDepth);
	assert(terms1 == "DOT");
	return exp_struct(terms, rv, struDepth);
}

static void args(Node* root, FieldList& fieldList) {
	root = root->child;
	Type current = exp(root);
	Oprand expVal = Oprand::temp_var(false);
	/*
	if (current->get_basic_type() != BASIC) {
		expVal.address = true;
		intercodes.push_back({ {Oprand::temp_var(), expVal}, ASSIGN });
		expVal = Oprand::temp_var(false);
	}*/
	fieldList.push_back({ "", current });
	if (root->next)
		args(root->next->next, fieldList);
	intercodes.push_back({ {expVal}, ARG });
}

extern "C" void traverse_gtree(Node * root) {
	assert(root);
	ext_def_list(root->child);
	symTable.check_undefined();
}

extern "C" void print_intercodes() {
	for (auto& code : intercodes)
		code.print();
}
