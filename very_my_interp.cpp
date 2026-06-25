#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <map>
#include <set>
#include <stdexcept>
#include <variant>
#include <stack>

enum class word_t {
	// разделители
	COLON, SEMICOLON, L_PAREN, R_PAREN, L_BRACE, R_BRACE, COMMA, POINT,
	// операции
	EQ, NOT_EQ, LESS, LE, GREATER, GE, PLUS, MINUS, MUL, DIV, MOD, ASSIGN, AND, OR, NOT,
	// служебные слова
	IF, ELSE, WHILE, FOR, BREAK, GOTO, STRUCT, PROGRAM, READ, WRITE,
	// типы
	INT_T, BOOLEAN, REAL_T, STRING_T, TRUE_, FALSE_, INT, REAL, STRING,
	// пользовательские слова
	IDENTIFIER, 
	// EOF
	E_O_F, EMPTY
};

struct word {
	word_t type;
	std::string data;
	word (word_t type = word_t::EMPTY, std::string data = ""): type(type), data(data) {}
};

// ---------- LEXER ----------

class Lex_Analyze {
	std::string program;
	int current;
	
	char cur() {return program[current];}
	
	char prev() {return program[current - 1];}
	
	bool E_O_F() {return current >= program.length();}
	
	bool check(char expect) {return program[current] == expect;}
	
	char get_c() {
		if (!E_O_F()) return program[current++];
		else return '\0';
	}
	
	std::string get_number(bool& Point_F) {
		std::string number = "";
		number += prev();
		while ((isdigit(cur()) || cur() == '.') && !E_O_F()) {
			number += get_c();
			Point_F = (cur() == '.');
		}
		return number;
	}
	
	std::string string(){
		std::string string = "";
		while ((cur() != '"') && (!E_O_F())) {
			string += get_c();
		}
		get_c();
		if (E_O_F()) throw std::runtime_error ("Незавершенная строка.");
		return string;
	}
	
	std::string name(word_t& type){
		std::string name = "";
		name += prev();
		while ((isalnum(cur()) || (cur() == '_')) && (!E_O_F())) {
			name += get_c();
		}
		std::map <std::string, word_t> serv_words{
			{"if", word_t::IF},
			{"else", word_t::ELSE},
			{"while", word_t::WHILE},
			{"for", word_t::FOR},
			{"break", word_t::BREAK},
			{"read", word_t::READ},
			{"write", word_t::WRITE},
			{"goto", word_t::GOTO},
			{"and", word_t::AND},
			{"or", word_t::OR},
			{"true", word_t::TRUE_},
			{"false", word_t::FALSE_},
			{"struct", word_t::STRUCT},
			{"int", word_t::INT_T},
			{"real", word_t::REAL_T},
			{"bool", word_t::BOOLEAN},
			{"string", word_t::STRING_T},
			{"program", word_t::PROGRAM}
		};
		auto pos = serv_words.find(name);
		if (pos != serv_words.end()) {
			type = pos -> second;
		} else {
			type = word_t::IDENTIFIER;
		}
		return name;
	}
	
	void skip() {
		while (!E_O_F()) {
			if ((get_c() == '*') && (cur() == '/')) {
				get_c();
				break;
			}
		}
		if (E_O_F()) throw std::runtime_error ("Незавершенный комментарий.");
	}
	
	void add_to_words(std::vector <word>& words, word_t type, const std::string& data) {
		words.emplace_back(type, data);
	}
	
	public:
	
	Lex_Analyze(std::string program): program(program), current(0) {}
	
	std::vector <word> Lex_Analyzer() {
		std::vector <word> parse_program; 
		char curr;
		while (!E_O_F()) {
			curr = get_c();
			switch(curr) {
				case ' ': 
				case '\n':
				case '\t': break;
				case '+': add_to_words(parse_program, word_t::PLUS, "+"); break;
				case '-': add_to_words(parse_program, word_t::MINUS, "-"); break;
				case '*': add_to_words(parse_program, word_t::MUL, "*"); break;
				case '%': add_to_words(parse_program, word_t::MOD, "%"); break;
				case '/': 
					if (cur() == '*') {
						get_c(); skip(); 
					} else {
						add_to_words(parse_program, word_t::DIV, "/"); 
					}
					break;
				case '=': 
					if (cur() == '=') {
						add_to_words(parse_program, word_t::EQ, "=="); get_c(); 
					} else {
						add_to_words(parse_program, word_t::ASSIGN, "="); 
					}
					break;
				case '!':
					if (cur() == '=') {
						add_to_words(parse_program, word_t::NOT_EQ, "!="); get_c(); 
					} else {
						add_to_words(parse_program, word_t::NOT, "!"); 
					}
					break;
				case '(': add_to_words(parse_program, word_t::L_PAREN, "("); break;
				case ')': add_to_words(parse_program, word_t::R_PAREN, ")"); break;
				case '{': add_to_words(parse_program, word_t::L_BRACE, "{"); break;
				case '}': add_to_words(parse_program, word_t::R_BRACE, "}"); break;
				case ':': add_to_words(parse_program, word_t::COLON, ":"); break;
				case ';': add_to_words(parse_program, word_t::SEMICOLON, ";"); break;
				case ',': add_to_words(parse_program, word_t::COMMA, ","); break;
                case '.': add_to_words(parse_program, word_t::POINT, "."); break;
                case '<':
                    if (cur() == '=') {
                        add_to_words(parse_program, word_t::LE, "<=");
                    } else {
                        add_to_words(parse_program, word_t::LESS, "<");
                    }
                    break;
                case '>':
                    if (cur() == '=') {
                        add_to_words(parse_program, word_t::GE, ">=");
                    } else {
                        add_to_words(parse_program, word_t::GREATER, ">");
                    }
                    break;
                case '"': add_to_words(parse_program, word_t::STRING, string()); break;
                default:
                	if (isdigit(curr)) {
                		bool PF = false;
                		std::string number = get_number(PF);
                		if (PF) {
                			add_to_words(parse_program, word_t::REAL, number);
                		} else {
                			add_to_words(parse_program, word_t::INT, number);
                		}
                	} else if (isalpha(curr) || curr == '_') {
                		word_t type;
                		std::string ident = name(type);
                		add_to_words(parse_program, type, ident);
                	} else {
                		throw std::runtime_error ("Неизвестный символ.");
                	}
			}
		}
		add_to_words(parse_program, word_t::E_O_F, "");
		return parse_program;
	}
};

// ---------- SYNTAX ----------

enum class return_t {
	INT, REAL, BOOLEAN, STRING, VOID, STRUCT
};

class expr {
	public:
	virtual ~expr() {}
};

class binary_expr: public expr {
	public:
	expr* left;
	expr* right;
	word op;
	binary_expr (expr* left, expr* right, word op): left(std::move(left)), right(std::move(right)), op(op) {}
	~binary_expr () {delete left; delete right;}
};

class unary_expr: public expr {
	public:
	expr* operand;
	word op;
	unary_expr (expr* operand, word op): operand(std::move(operand)), op(op) {}
	~unary_expr () {delete operand;}
};

class const_expr: public expr {
	public:
	word value;
	const_expr (word value): value(value) {}
};

class variable_expr: public expr {
	public:
	word name;
	variable_expr (word name): name(name) {}
};

class struct_expr: public expr {
	public:
	word struct_name;
	word field_name;
	struct_expr (word struct_name, word field_name): struct_name(struct_name), field_name(field_name) {}
};

class stat {
	public:
	virtual ~stat() {}
};

class if_stat: public stat {
	public:
	expr* cond;
	stat* if_part;
	stat* else_part;
	if_stat (expr* cond, stat* if_part, stat* else_part): cond(std::move(cond)), if_part(std::move(if_part)), else_part(std::move(else_part)) {}
	~if_stat() {delete cond; delete if_part; delete else_part;}
};

class while_stat: public stat {
	public:
	expr* cond;
	stat* body;
	while_stat (expr* cond, stat* body): cond(std::move(cond)), body(std::move(body)) {}
	~while_stat() {delete cond; delete body;}
};

class for_stat: public stat {
	public:
	expr* ident;
	expr* cond;
	expr* incr;
	stat* body;
	for_stat (expr* ident, expr* cond, expr* incr, stat* body): ident(std::move(ident)), cond(std::move(cond)), incr(std::move(incr)), body(std::move(body)) {}
	~for_stat() {delete ident; delete cond; delete incr; delete body;}
};

class block_stat: public stat {
	public:
	std::vector <stat*> statements;
	~block_stat() {
		for (auto& stat : statements) delete stat;
	}
};

class goto_stat: public stat {
	public:
	word label;
	goto_stat (word label): label(label) {}
};

class label_stat: public stat {
	public:
	word name;
	stat* statement;
	label_stat (word name, stat* statement): name(name), statement(std::move(statement)) {}
	~label_stat () {delete statement;}
};

class read_stat: public stat {
	public:
	word variable;
	read_stat (word variable): variable(variable) {}
};

class write_stat: public stat {
	public:
	std::vector <expr*> expressions;
	~write_stat () {
		for (auto& expr : expressions) {
			delete expr;
		}
	}
};

class var_stat: public stat {
	public:
	word name;
	return_t type;
	expr* init;
	var_stat (word name, return_t type, expr* init): name(name), type(type), init(std::move(init)) {}
	~var_stat () {delete init;}
};

class struct_stat: public stat {
	public:
	word struct_name;
	std::vector <var_stat*> fields;
	~struct_stat () {
		for (auto& field : fields) {
			delete field;
		}
	}
};

class break_stat: public stat {
	public:
	word break_word;
	break_stat (word break_word): break_word(break_word) {}
};

class expr_stat: public stat {
	public:
	expr* expression;
	expr_stat (expr* expression): expression(std::move(expression)) {}
	~expr_stat () {delete expression;}
};

class poliz_prep {
	public:
	std::vector <stat*> declarations;
	std::vector <stat*> statements;
	~poliz_prep () {
		for (stat* stat : declarations) {
			delete stat;
		}
		for (stat* stat : statements) {
			delete stat;
		}
	}
};

class Syntax_Analyze {
	const std::vector<word>& words;
	int pos;
	public:
	
	Syntax_Analyze (const std::vector<word>& words): words(words), pos(0) {}
	~Syntax_Analyze () {}
	
	poliz_prep* syn_analyzer () {
		poliz_prep* program = new poliz_prep();
		eat(word_t::PROGRAM, "Отсутствует заголовок.");
		eat(word_t::L_BRACE, "Ожидалось '{'");
		while (!E_O_F() && !check(word_t::R_BRACE)) {
			if (cur().type == word_t::STRUCT) {
				program -> declarations.emplace_back(struct_declaration());
				eat(word_t::SEMICOLON, "Ожидалось ';'");
			} else if (if_type()) {
				return_t type = get_return_t();
				advance();
				do {
					program -> declarations.emplace_back(var_declaration(type));
				} while (check_prev(word_t::COMMA));
				eat(word_t::SEMICOLON, "Ожидалось ';'");
			} else {
				program -> statements.emplace_back(statement());
			}
		}
		eat(word_t::R_BRACE, "Ожидалось '}'");
		return program;
	}
	
	private:
	word_t get_type() {
		return cur().type;
	}
	
	bool if_type() {
		return (get_type() == word_t::INT_T || get_type() == word_t::REAL_T || get_type() == word_t::BOOLEAN || get_type() == word_t::STRING_T);
	}
	
	word cur() {
		return words[pos];
	}
	
	word advance() {
		return words[pos++];
	}
	
	void back() {
		pos--;
	}
	
	word prev() {
		return words[pos - 1];
	}
	
	bool check(word_t type) {
		return (get_type() == type);
	}
	
	bool check_prev(word_t type) {
		return (prev().type == type);
	}
	
	bool check_and_forw (word_t type) {
		if (get_type() == type) {pos++; return true;}
		return false;
	}
	
	word eat(word_t type, std::string err) {
		if (get_type() == type) return advance();
		throw std::runtime_error(err);
	}
	
	bool E_O_F () {
		return get_type() == word_t::E_O_F;
	}
	
	return_t get_return_t () {
		if (get_type() == word_t::INT_T) return return_t::INT;
		if (get_type() == word_t::REAL_T) return return_t::REAL;
		if (get_type() == word_t::BOOLEAN) return return_t::BOOLEAN;
		if (get_type() == word_t::STRING_T) return return_t::STRING;
		throw std::runtime_error("Ожидался тип.");
	}
	
	stat* var_declaration (return_t type) {
		word name = eat(word_t::IDENTIFIER, "Ожидалось имя переменной.");
		expr* init = nullptr;
		if (get_type() == word_t::ASSIGN) {
			advance();
			init = expression();
		}
		if (get_type() == word_t::IDENTIFIER) {
			throw std::runtime_error("Ожидалась запятая.");
		}
		if (get_type() == word_t::COMMA) {
			advance();
		}
		return new var_stat(name, type, init);
	}
	
	stat* struct_declaration () {
		advance();
		word name = eat(word_t::IDENTIFIER, "Ожидалось имя структуры.");
		return_t type;
		eat(word_t::L_BRACE, "Ожидалось '{'");
		struct_stat* structure = new struct_stat;
		structure -> struct_name = name;
		while (get_type() != word_t::R_BRACE) {
			type = get_return_t();
			advance();
			do {
				structure -> fields.emplace_back((var_stat*)var_declaration(type));
			} while (check_prev(word_t::COMMA)); 
			eat(word_t::SEMICOLON, "Ожидалось ';'");
		}
		eat(word_t::R_BRACE, "Ожидалось '}'");
		return structure;
	}
	
	stat* statement() {
		if (check_and_forw(word_t::IF)) return if_statement();
		if (check_and_forw(word_t::WHILE)) return while_statement();
		if (check_and_forw(word_t::FOR)) return for_statement();
		if (check_and_forw(word_t::L_BRACE)) return block_statement();
		if (check_and_forw(word_t::READ)) return read_statement();
		if (check_and_forw(word_t::WRITE)) return write_statement();
		if (check_and_forw(word_t::GOTO)) return goto_statement();
		if (check_and_forw(word_t::BREAK)) return break_statement();
		if (check_and_forw(word_t::IDENTIFIER) && check(word_t::COLON)) return label_statement();
		else if (prev().type == word_t::IDENTIFIER) {back(); return expr_statement();}
		return expr_statement();
	}
	
	stat* if_statement() {
		eat(word_t::L_PAREN, "Ожидалось '('");
		expr* cond = expression();
		eat(word_t::R_PAREN, "Ожидалось ')'");
		stat* if_part = statement();
		stat* else_part = nullptr;
		if (check_and_forw(word_t::ELSE)) {
			else_part = statement();
		}
		return new if_stat(cond, if_part, else_part);
	}
	
	stat* while_statement() {
		eat(word_t::L_PAREN, "Ожидалось '('");
		expr* cond = expression();
		eat(word_t::R_PAREN, "Ожидалось ')'");
		stat* body = statement();
		return new while_stat(cond, body);
	}
	
	stat* for_statement() {
		eat(word_t::L_PAREN, "Ожидалось '('");
		expr* ident = nullptr;
		if (!check_and_forw(word_t::SEMICOLON)) {
			ident = expression();
			eat(word_t::SEMICOLON, "Ожидалось ';'");
		}
		expr* cond = nullptr;		
		if (!check_and_forw(word_t::SEMICOLON)) {
			cond = expression();
			eat(word_t::SEMICOLON, "Ожидалось ';'");
		}
		expr* incr = nullptr;
		if (!check(word_t::R_PAREN)) {
			incr = expression();
		}
		eat(word_t::R_PAREN, "Ожидалось ')'");
		stat* body = statement();
		return new for_stat(ident, cond, incr, body);
	}
	
	stat* block_statement() {
		block_stat* block = new block_stat();
		while (!check(word_t::R_BRACE)) {
			block -> statements.emplace_back(statement());
		}
		advance();
		return block;
	}
	
	stat* read_statement() {
		eat(word_t::L_PAREN, "Ожидалось '('");
		word name = eat(word_t::IDENTIFIER, "Ожидалось имя переменной.");
		eat(word_t::R_PAREN, "Ожидалось ')'");
		eat(word_t::SEMICOLON, "Ожидалось ';'");
		return new read_stat(name);
	}
	
	stat* write_statement() {
		eat(word_t::L_PAREN, "Ожидалось '('");
		write_stat* writestat = new write_stat();
		while (!check(word_t::R_PAREN)) {
			writestat -> expressions.emplace_back(expression());
			if (!check(word_t::R_PAREN)) eat(word_t::COMMA, "Ожидалось ','");
		}
		eat(word_t::R_PAREN, "Ожидалось ')'");
		eat(word_t::SEMICOLON, "Ожидалось ';'");
		return writestat;
	}
	
	stat* goto_statement() {
		word label = eat(word_t::IDENTIFIER, "Ожидалось имя метки.");
		eat(word_t::SEMICOLON, "Ожидалось ';'");
		return new goto_stat(label);
	}
	
	stat* break_statement() {
		word key = prev();
		eat(word_t::SEMICOLON, "Ожидалось ';'");
		return new break_stat(key);
	}
	
	stat* label_statement() {
		word label = prev();
		advance();
		stat* some_stat = statement();
		return new label_stat(label, some_stat);
	}
	
	stat* expr_statement() {
		expr* some_expr = expression();
		eat(word_t::SEMICOLON, "Ожидалось ';'");
		return new expr_stat(some_expr);
	}
	
	expr* expression() {
		return assign_expr();
	}
	
	expr* assign_expr() {
		expr* left = or_expr();
		if (check_and_forw(word_t::ASSIGN)) {
			word op = prev();
			expr* right = assign_expr();
			if (dynamic_cast<variable_expr*>(left) || dynamic_cast<struct_expr*>(left)) {
				return new binary_expr(left, right, op);
			}
			delete left; delete right;
			throw std::runtime_error("Неправильный операнд операции присваивания.");
		}
		return left;
	}
	
	expr* or_expr() {
		expr* left = and_expr();
		if (check_and_forw(word_t::OR)) {
			word op = prev();
			expr* right = or_expr();
			return new binary_expr(left, right, op);
		}
		return left;
	}
	
	expr* and_expr() {
		expr* left = eq_expr();
		if (check_and_forw(word_t::AND)) {
			word op = prev();
			expr* right = and_expr();
			return new binary_expr(left, right, op);
		}
		return left;
	}
	
	expr* eq_expr() {
		expr* left = comp_expr();
		if (check_and_forw(word_t::EQ) || check_and_forw(word_t::NOT_EQ)) {
			word op = prev();
			expr* right = eq_expr();
			return new binary_expr(left, right, op);
		}
		return left;
	}
	
	expr* comp_expr() {
		expr* left = term_expr();
		if (check_and_forw(word_t::LESS) || check_and_forw(word_t::LE) || check_and_forw(word_t::GREATER) || check_and_forw(word_t::GE)) {
			word op = prev();
			expr* right = comp_expr();
			return new binary_expr(left, right, op);
		}
		return left;
	}
	
	expr* term_expr() {
		expr* left = fact_expr();
		if (check_and_forw(word_t::PLUS) || check_and_forw(word_t::MINUS)) {
			word op = prev();
			expr* right = term_expr();
			return new binary_expr(left, right, op);
		}
		return left;
	}
	
	expr* fact_expr() {
		expr* left = un_expr();
		if (check_and_forw(word_t::MUL) || check_and_forw(word_t::DIV) || check_and_forw(word_t::MOD)) {
			word op = prev();
			expr* right = fact_expr();
			return new binary_expr(left, right, op);
		}
		return left;
	}
	
	expr* un_expr() {
		if (check_and_forw(word_t::NOT) || check_and_forw(word_t::MINUS)){
			word op = prev();
			expr* operand = un_expr();
			return new unary_expr(operand, op);
		}
		return prim_expr();
	}
	
	expr* prim_expr() {
		if (check_and_forw(word_t::STRING)) return new const_expr(prev());
		if (check_and_forw(word_t::INT)) return new const_expr(prev());
		if (check_and_forw(word_t::REAL)) return new const_expr(prev());
		if (check_and_forw(word_t::TRUE_)) return new const_expr(word(word_t::BOOLEAN, "true"));
		if (check_and_forw(word_t::FALSE_)) return new const_expr(word(word_t::BOOLEAN, "false"));
		if (check_and_forw(word_t::L_PAREN)) {
			expr* some_expr = expression();
			eat(word_t::R_PAREN, "Ожидалось ')'");
			return some_expr;
		}
		if (check_and_forw(word_t::IDENTIFIER)) {
			word name = prev();
			if (check_and_forw(word_t::POINT)) {
				word field = eat(word_t::IDENTIFIER, "Ожидалось имя поля после '.'");
				return new struct_expr(name, field);
			}
			return new variable_expr(name);
		}
		throw std::runtime_error("Ожидалось выражение.");
	}
	
};

// ---------- SEMANTIC ----------

class struct_for_sem {
	public:
	std::string name;
	std::map<std::string, return_t> fields;
};

class Semantic_Analyze {

	std::map<std::string, return_t> variables;
	std::map<std::string, struct_for_sem> structures;
	std::set<std::string> labels;
	std::set<std::string> gotos;
	bool loop_flag = false;
	
	public: 
	
	void semantic (poliz_prep& program) {
		
		for (auto& declare : program.declarations) {
			if (auto struct_declare = dynamic_cast<struct_stat*>(declare)) {
				struct_for_sem structure;
				structure.name = struct_declare -> struct_name.data;
				for (auto& field : struct_declare -> fields) {
					structure.fields[field -> name.data] = field -> type;
					if (field -> init) {
						return_t init_t = check_expr(*field -> init);
						if ((field -> type) != init_t) {
							throw std::runtime_error("Несоответствие типов при инициализации.");
						}
					}
				}
				structures[structure.name] = structure;
			}
			if (auto var_declare = dynamic_cast<var_stat*>(declare)) {
				if (variables.find(var_declare -> name.data) != variables.end()) {
					throw std::runtime_error("Переменная " + var_declare -> name.data + " уже описана.");
				}
				variables[var_declare -> name.data] = var_declare -> type;
				if (var_declare -> init) {
					return_t init_t = check_expr(*var_declare -> init);
					if ((var_declare -> type) != init_t) {
						throw std::runtime_error("Несоответствие типов при инициализации.");
					}
				}
			}
		}
		
		for (auto& statement : program.statements) {
			check_stat(*statement);
		}
		
		for (std::string label_name : gotos) {
			if (labels.find(label_name) == labels.end()) {
				throw std::runtime_error("Переход по неизвестной метке.");
			}
		}
	}
	
	private:
	
	void check_stat (stat& statement) {
		if (auto ifstat = dynamic_cast<if_stat*>(&statement)) {
			return_t cond = check_expr(*ifstat -> cond);
			if (cond != return_t::BOOLEAN && cond != return_t::INT) {
				throw std::runtime_error("Условие if должно быть int или boоl.");
			}
			check_stat(*ifstat -> if_part);
			if (ifstat -> else_part) {
				check_stat(*ifstat -> else_part);
			}
		} else if (auto whilestat = dynamic_cast<while_stat*>(&statement)) {
			return_t cond = check_expr(*whilestat -> cond);
			if (cond != return_t::BOOLEAN && cond != return_t::INT) {
				throw std::runtime_error("Условие while должно быть int или boоl.");
			}
			bool flag = loop_flag;
			loop_flag = true;
			check_stat(*whilestat -> body);
			loop_flag = flag;
		} else if (auto forstat = dynamic_cast<for_stat*>(&statement)) {
			if (forstat -> ident) {
				check_expr(*forstat -> ident);
			}
			if (forstat -> cond) {
				return_t cond = check_expr(*forstat -> cond);
				if (cond != return_t::BOOLEAN && cond != return_t::INT) {
					throw std::runtime_error("Условие for должно быть int или boоl.");
				}
			}
			if (forstat -> incr) {
				check_expr(*forstat -> incr);
			}
			bool flag = loop_flag;
			loop_flag = true;
			check_stat(*forstat -> body);
			loop_flag = flag;
		} else if (auto block = dynamic_cast<block_stat*>(&statement)) {
			for (auto& some_stat : block -> statements) {
				check_stat(*some_stat);
			}	
		} else if (auto breakstat = dynamic_cast<break_stat*>(&statement)) {
			if (!loop_flag) {
				throw std::runtime_error("Break вне цикла.");
			}
		} else if (auto labelstat = dynamic_cast<label_stat*>(&statement)) {
			if (labels.find(labelstat -> name.data) != labels.end()) {
				throw std::runtime_error("Метка " + labelstat -> name.data + " встречается дважды.");
			}
			labels.insert(labelstat -> name.data);
			check_stat(*labelstat -> statement);
		} else if (auto gotostat = dynamic_cast<goto_stat*>(&statement)) {
			gotos.insert(gotostat -> label.data);
		} else if (auto readstat = dynamic_cast<read_stat*>(&statement)) {
			if (variables.find(readstat -> variable.data) == variables.end()) {
				throw std::runtime_error("Переменная " + readstat -> variable.data + " не описана.");
			}
		} else if (auto writestat = dynamic_cast<write_stat*>(&statement)) {
			for (auto& some_expr : writestat -> expressions) {
				check_expr(*some_expr);
			}
		} else if (auto exprstat = dynamic_cast<expr_stat*>(&statement)) {
			check_expr(*exprstat -> expression);
		}
	}
	
	return_t check_expr(expr& expression) {
		if (auto binexpr = dynamic_cast<binary_expr*>(&expression)) {
			return_t left_t = check_expr(*binexpr -> left);
			return_t right_t = check_expr(*binexpr -> right);
			switch (binexpr -> op.type) {
				case word_t::PLUS:
					if (left_t == return_t::STRING && right_t == return_t::STRING) {
						return return_t::STRING;
					}
					if ((left_t == return_t::INT || left_t == return_t::REAL) && (right_t == return_t::INT || right_t == return_t::REAL)) {
						return (left_t == return_t::REAL || right_t == return_t::REAL) ? return_t::REAL : return_t::INT;
					}
					break;
				case word_t::MINUS:
				case word_t::MUL:
				case word_t::DIV:
					if ((left_t == return_t::INT || left_t == return_t::REAL) && (right_t == return_t::INT || right_t == return_t::REAL)) {
						return (left_t == return_t::REAL || right_t == return_t::REAL) ? return_t::REAL : return_t::INT;
					}
					break;
				case word_t::MOD:
					if (left_t == return_t::INT && right_t == return_t::INT) {
						return return_t::INT;
					}
					break;
				case word_t::AND:
				case word_t::OR:
					if (left_t == return_t::BOOLEAN && right_t == return_t::BOOLEAN) {
						return return_t::BOOLEAN;
					}
					break;
				case word_t::LESS:
				case word_t::LE:
				case word_t::GREATER:
				case word_t::GE:
				case word_t::EQ:
				case word_t::NOT_EQ:
					if (left_t == return_t::STRING && right_t == return_t::STRING) {
						return return_t::BOOLEAN;
					}
					if ((left_t == return_t::INT || left_t == return_t::REAL) && (right_t == return_t::INT || right_t == return_t::REAL)) {
						return return_t::BOOLEAN;
					}
					break;
				case word_t::ASSIGN:
					return left_t;
				default:
					break;
			}
			throw std::runtime_error("Несоответствие типов в бинарной операции.");
		} else if (auto unexpr = dynamic_cast<unary_expr*>(&expression)) {
			return_t operand_t = check_expr(*unexpr -> operand);
			switch (unexpr -> op.type) {
				case word_t::NOT:
					if (operand_t == return_t::BOOLEAN) {
						return return_t::BOOLEAN;
					}
					throw std::runtime_error("'!' требует bool.");
				case word_t::MINUS:
					if (operand_t == return_t::INT || operand_t == return_t::REAL) {
						return operand_t == return_t::INT ? return_t::INT : return_t::REAL;
					}
					throw std::runtime_error("Унарный '-' требует int или real.");
			}
		} else if (auto constant = dynamic_cast<const_expr*>(&expression)) {
			switch (constant -> value.type) {
				case word_t::INT: return return_t::INT;
				case word_t::REAL: return return_t::REAL;
				case word_t::BOOLEAN: return return_t::BOOLEAN;
				case word_t::STRING: return return_t::STRING;
				default: break;
			}
		} else if (auto varexpr = dynamic_cast<variable_expr*>(&expression)) {
			auto obj = variables.find(varexpr -> name.data);
			if (obj != variables.end()) {
				return obj -> second;
			}
			throw std::runtime_error("Неописанная переменная " + varexpr -> name.data);
		} else if (auto structexpr = dynamic_cast<struct_expr*>(&expression)) {
			auto structure = structures.find(structexpr -> struct_name.data);
			if (structure != structures.end()) {
				auto obj = structure -> second.fields.find(structexpr -> field_name.data);
				if (obj != structure -> second.fields.end()) {
					return obj -> second;
				}
				throw std::runtime_error("Неизвестное поле " + structexpr -> field_name.data + " структуры " + structexpr -> struct_name.data);
			}
			throw std::runtime_error("Неизвестная структура " + structexpr -> struct_name.data);
		}
		throw std::runtime_error("Ожидалось выражение.");
	}
	
};

// ---------- POLIZ ----------

enum class poliz_t {
    // Операции
    ADD, SUB, MUL, DIV, MOD,
    NEG, NOT,
    AND, OR,
    LESS, GRT, LE, GE, EQ, NE, 
    // Управление
    JMP, JMP_FALSE, LABEL,
    // Ввод-вывод
    READ, WRITE,
    // Работа с переменными
    LOAD, STORE, STORE_FIELD, STRUCT_NAME, LOAD_FIELD,
    // Константы
    PUSH_INT, PUSH_REAL, PUSH_STRING, PUSH_BOOL,
    // Другое
    BREAK, NOP
};

struct poliz_obj {
    poliz_t op;
    std::variant<int, double, std::string, bool> operand;
    
    poliz_obj(poliz_t op) : op(op) {}
    poliz_obj(poliz_t op, int value) : op(op), operand(value) {}
    poliz_obj(poliz_t op, double value) : op(op), operand(value) {}
    poliz_obj(poliz_t op, const std::string& value) : op(op), operand(value) {} 
    poliz_obj(poliz_t op, bool value) : op(op), operand(value) {}
};

class Poliz_Generate {
public:
	std::vector<poliz_obj> generate(poliz_prep& program) {
		code.clear(); 
		labels.clear();
		breakStack.clear(); 
		
		for (auto& decl : program.declarations) {
			if (auto structstat = dynamic_cast<struct_stat*>(decl)) {
		        for (auto& field : structstat -> fields) {
		            if (field -> init) {
			            expression_poliz(*field -> init);
			    	} else {
			        	switch (field -> type) {
			        		case return_t::INT: code.emplace_back(poliz_t::PUSH_INT, 0); break;
			        		case return_t::REAL: code.emplace_back(poliz_t::PUSH_REAL, 0); break;
			        		case return_t::BOOLEAN: code.emplace_back(poliz_t::PUSH_BOOL, 0); break;
			        		case return_t::STRING: code.emplace_back(poliz_t::PUSH_STRING, ""); break;
			        	}
			        }
			        code.emplace_back(poliz_t::STRUCT_NAME, structstat -> struct_name.data);
			        code.emplace_back(poliz_t::STORE_FIELD, field -> name.data);
		        }
		    }
		    if (auto varstat = dynamic_cast<var_stat*>(decl)) {
		        if (varstat -> init) {
		            expression_poliz(*varstat -> init);
		        } else {
		        	switch (varstat -> type) {
		        		case return_t::INT: code.emplace_back(poliz_t::PUSH_INT, 0); break;
		        		case return_t::REAL: code.emplace_back(poliz_t::PUSH_REAL, 0); break;
		        		case return_t::BOOLEAN: code.emplace_back(poliz_t::PUSH_BOOL, 0); break;
		        		case return_t::STRING: code.emplace_back(poliz_t::PUSH_STRING, ""); break;
		        	}
		        }
		        code.emplace_back(poliz_t::STORE, varstat -> name.data);
		    }
		}
		
		for (auto& statement : program.statements) {
		    statement_poliz(*statement);
		}
		
		goto_jmp_resolve();
		return code;
	}

private:
    std::vector<poliz_obj> code;
    std::map<std::string, int> labels;
    std::vector<std::vector<int>> breakStack;
    
    void statement_poliz(stat& statement) {
        if (auto block = dynamic_cast<block_stat*>(&statement)) {
            for (auto& some_stat : block -> statements) {
                statement_poliz(*some_stat);
            }
        }
        else if (auto ifstat = dynamic_cast<if_stat*>(&statement)) {
            expression_poliz(*ifstat -> cond);
            int jump_false = (int)code.size();
            code.emplace_back(poliz_t::JMP_FALSE);
            statement_poliz(*ifstat -> if_part);
            if (ifstat -> else_part) {
                int jump_end = (int)code.size();
                code.emplace_back(poliz_t::JMP);
                code[jump_false].operand = (int)code.size();
                statement_poliz(*ifstat -> else_part);
                code[jump_end].operand = (int)code.size();
            } else {
                code[jump_false].operand = (int)code.size();
            }
        }
        else if (auto whilestat = dynamic_cast<while_stat*>(&statement)) {
            int begin = (int)code.size();
            expression_poliz(*whilestat -> cond);
            int jump_end = (int)code.size();
            code.emplace_back(poliz_t::JMP_FALSE); 
            breakStack.emplace_back(); 
            statement_poliz(*whilestat -> body);
            code.emplace_back(poliz_t::JMP, (int)begin);
            code[jump_end].operand = (int)code.size();
            for (int pos : breakStack.back()) {
                code[pos].operand = (int)code.size();
            }
            breakStack.pop_back();
        }
        else if (auto forstat = dynamic_cast<for_stat*>(&statement)) {
            if (forstat -> ident) {
                expression_poliz(*forstat -> ident);
            }
            int begin = (int)code.size();
            if (forstat -> cond) {
                expression_poliz(*forstat -> cond);
                int jump_end = (int)code.size();
                code.emplace_back(poliz_t::JMP_FALSE);
                int jump_body = (int)code.size();
                code.emplace_back(poliz_t::JMP);
                int incr_pos = (int)code.size();
                if (forstat -> incr) {
                    expression_poliz(*forstat -> incr);
                    code.emplace_back(poliz_t::NOP); 
                }
                code.emplace_back(poliz_t::JMP, (int)begin);
                code[jump_body].operand = (int)code.size();
                breakStack.emplace_back();
                statement_poliz(*forstat -> body);
                code.emplace_back(poliz_t::JMP, (int)incr_pos);
                code[jump_end].operand = (int)code.size();
                
            } else {
                breakStack.emplace_back();
                statement_poliz(*forstat -> body);
                if (forstat -> incr) {
                    expression_poliz(*forstat -> incr);
                    code.emplace_back(poliz_t::NOP); 
                }
                code.emplace_back(poliz_t::JMP, (int)begin);
            }
            for (int pos : breakStack.back()) {
                code[pos].operand = (int)code.size();
            }
            breakStack.pop_back();
        }
        else if (auto breakstat = dynamic_cast<break_stat*>(&statement)) {
            int break_pos = (int)code.size();
            code.emplace_back(poliz_t::JMP); 
            breakStack.back().push_back(break_pos);
        }
        else if (auto gotostat = dynamic_cast<goto_stat*>(&statement)) {
            code.emplace_back(poliz_t::JMP, gotostat -> label.data);
        }
        else if (auto labelstat = dynamic_cast<label_stat*>(&statement)) {
            int label_pos = (int)code.size();
            code.emplace_back(poliz_t::LABEL, labelstat -> name.data);
            labels[labelstat -> name.data] = label_pos;
            statement_poliz(*labelstat -> statement);
        }
        else if (auto readstat = dynamic_cast<read_stat*>(&statement)) {
            code.emplace_back(poliz_t::READ, readstat -> variable.data);
        }
        else if (auto writestat = dynamic_cast<write_stat*>(&statement)) {
            for (auto& expr : writestat -> expressions) {
                expression_poliz(*expr);
                code.emplace_back(poliz_t::WRITE);
            }
        }
        else if (auto exprstat = dynamic_cast<expr_stat*>(&statement)) { 
            expression_poliz(*exprstat -> expression);
            code.emplace_back(poliz_t::NOP);
        }
    }
    
    void expression_poliz(expr& expr) { 
        if (auto binary = dynamic_cast<binary_expr*>(&expr)) {
            if (binary -> op.type == word_t::ASSIGN) {
                expression_poliz(*binary -> right);
                if (auto var = dynamic_cast<variable_expr*>(binary -> left)) {
                    code.emplace_back(poliz_t::STORE, var -> name.data);
                }
                else if (auto structexpr = dynamic_cast<struct_expr*>(binary -> left)) {
                	code.emplace_back(poliz_t::STRUCT_NAME, structexpr -> struct_name.data);
                    code.emplace_back(poliz_t::STORE_FIELD, structexpr -> field_name.data);
                }
            } else {
                expression_poliz(*binary -> left);
                expression_poliz(*binary -> right);
                
                switch (binary -> op.type) {
                    case word_t::PLUS: code.emplace_back(poliz_t::ADD); break;
                    case word_t::MINUS: code.emplace_back(poliz_t::SUB); break;
                    case word_t::MUL: code.emplace_back(poliz_t::MUL); break;
                    case word_t::DIV: code.emplace_back(poliz_t::DIV); break;
                    case word_t::MOD: code.emplace_back(poliz_t::MOD); break;
                    case word_t::AND: code.emplace_back(poliz_t::AND); break;
                    case word_t::OR: code.emplace_back(poliz_t::OR); break;
                    case word_t::LESS: code.emplace_back(poliz_t::LESS); break;
                    case word_t::GREATER: code.emplace_back(poliz_t::GRT); break;
                    case word_t::LE: code.emplace_back(poliz_t::LE); break;
                    case word_t::GE: code.emplace_back(poliz_t::GE); break;
                    case word_t::EQ: code.emplace_back(poliz_t::EQ); break;
                    case word_t::NOT_EQ: code.emplace_back(poliz_t::NE); break;
                    default: break;
                }
            }
        }
        else if (auto unary = dynamic_cast<unary_expr*>(&expr)) {
            expression_poliz(*unary -> operand);
            
            switch (unary -> op.type) {
                case word_t::NOT: code.emplace_back(poliz_t::NOT); break;
                case word_t::MINUS: code.emplace_back(poliz_t::NEG); break;
                default: break;
            }
        }
        else if (auto constant = dynamic_cast<const_expr*>(&expr)) {
            switch (constant -> value.type) {
                case word_t::INT:
                    code.emplace_back(poliz_t::PUSH_INT, std::stoi(constant -> value.data));
                    break;
                case word_t::REAL:
                    code.emplace_back(poliz_t::PUSH_REAL, std::stod(constant -> value.data));
                    break;
                case word_t::STRING:
                    code.emplace_back(poliz_t::PUSH_STRING, constant -> value.data);
                    break;
                case word_t::BOOLEAN:
                    code.emplace_back(poliz_t::PUSH_BOOL, constant -> value.data == "true");
                    break;
                default:
                    break;
            }
        }
        else if (auto structexpr = dynamic_cast<struct_expr*>(&expr)) {
            code.emplace_back(poliz_t::STRUCT_NAME, structexpr -> struct_name.data);
            code.emplace_back(poliz_t::LOAD_FIELD, structexpr -> field_name.data);
        }
        else if (auto varexpr = dynamic_cast<variable_expr*>(&expr)) {
            code.emplace_back(poliz_t::LOAD, varexpr -> name.data);
        }
    }
    
    void goto_jmp_resolve() {
        for (auto& polizobj : code) {
            if (polizobj.op == poliz_t::JMP) {
            	if (std::holds_alternative<std::string>(polizobj.operand)) {
		            auto it = labels.find(std::get<std::string>(polizobj.operand));
		            polizobj.operand = (int)(it -> second);
                }
            }
        }
    }
};

// Интерпретатор

class Interpreter {
public:
    void execute(const std::vector<poliz_obj>& poliz) {
        while (!stack.empty()){
        	stack.pop();
        }
        variables.clear();
        pc = 0; // позиция на которую смотрим в ПОЛИЗе
        
        while (pc < poliz.size()) {
            const auto& item = poliz[pc++];
            
            switch (item.op) {
                case poliz_t::ADD: {
                    auto b = pop_value();
                    auto a = pop_value();
                    push_value(add(a, b));
                    break;
                }
                case poliz_t::SUB: {
                    auto b = pop_value();
                    auto a = pop_value();
                    push_value(sub(a, b));
                    break;
                }
                case poliz_t::MUL: {
                    auto b = pop_value();
                    auto a = pop_value();
                    push_value(mul(a, b));
                    break;
                }
                case poliz_t::DIV: {
                    auto b = pop_value();
                    auto a = pop_value();
                    push_value(div(a, b));
                    break;
                }
                case poliz_t::MOD: {
                    int b = std::get<int>(pop_value());
                    int a = std::get<int>(pop_value());
                    push_value(a % b);
                    break;
                }
                case poliz_t::NEG: {
                    auto val = pop_value();
                    if (std::holds_alternative<int>(val)) {
                        push_value(-std::get<int>(val));
                    } else {
                        push_value(-std::get<double>(val));
                    }
                    break;
                }
                case poliz_t::NOT: {
                    bool val = make_bool(pop_value());
                    push_value(!val);
                    break;
                }
                case poliz_t::AND: {
                    bool b = make_bool(pop_value());
                    bool a = make_bool(pop_value());
                    push_value(a && b);
                    break;
                }
                case poliz_t::OR: {
                    bool b = make_bool(pop_value());
                    bool a = make_bool(pop_value());
                    push_value(a || b);
                    break;
                }
                case poliz_t::LESS: {
                    auto b = pop_value();
                    auto a = pop_value();
                    push_value(less(a, b));
                    break;
                }
                case poliz_t::GRT: {
                    auto b = pop_value();
                    auto a = pop_value();
                    push_value(greater(a, b));
                    break;
                }
                case poliz_t::LE: {
                    auto b = pop_value();
                    auto a = pop_value();
                    push_value(L_E(a, b));
                    break;
                }
                case poliz_t::GE: {
                    auto b = pop_value();
                    auto a = pop_value();
                    push_value(G_E(a, b));
                    break;
                }
                case poliz_t::EQ: {
                    auto b = pop_value();
                    auto a = pop_value();
                    push_value(equal(a, b));
                    break;
                }
                case poliz_t::NE: {
                    auto b = pop_value();
                    auto a = pop_value();
                    push_value(not_equal(a, b));
                    break;
                }
                case poliz_t::JMP: {
                    pc = std::get<int>(item.operand);
                    break;
                }
                case poliz_t::JMP_FALSE: {
                    bool cond = make_bool(pop_value());
                    if (!cond) {
                        pc = std::get<int>(item.operand);
                    }
                    break;
                }
                case poliz_t::LABEL: { // обработали в ПОЛИЗе
                    break;
                }
                case poliz_t::READ: {
                    std::cout << std::endl;
                    std::string var_name = std::get<std::string>(item.operand); // имя переменной
                    std::string input;
                    std::cin >> input;
                    if (std::holds_alternative<int>(variables[var_name])) {
                        variables[var_name] = std::stoi(input);
                    } else if (std::holds_alternative<double>(variables[var_name])) {
                        variables[var_name] = std::stod(input);
                    } else if (std::holds_alternative<bool>(variables[var_name])) {
                        variables[var_name] = (input == "true");
                    } else {
                        variables[var_name] = input;
                    }
                    break;
                }
                case poliz_t::WRITE: {
                    auto val = pop_value();
                    if (std::holds_alternative<int>(val)) {
                        std::cout << std::get<int>(val) << " ";
                    } else if (std::holds_alternative<double>(val)) {
                        std::cout << std::get<double>(val) << " ";
                    } else if (std::holds_alternative<bool>(val)) {
                        std::cout << (std::get<bool>(val) ? "true" : "false") << " ";
                    } else {
                        std::cout << std::get<std::string>(val) << " ";
                    }
                    break;
                }
                case poliz_t::LOAD: {
                    std::string var_name = std::get<std::string>(item.operand);
                    push_value(variables[var_name]);
                    break;
                }
                case poliz_t::LOAD_FIELD: {
                	std::string field_name = std::get<std::string>(item.operand);
                	std::string struct_name = std::get<std::string>(pop_value());
                	push_value(struct_fields[struct_name][field_name]);
                	break;
                }
                case poliz_t::STORE: {
                    std::string var_name = std::get<std::string>(item.operand);
                    variables[var_name] = pop_value();
                    break;
                }
                case poliz_t::STORE_FIELD: {
                    std::string field_name = std::get<std::string>(item.operand);
                    std::string struct_name = std::get<std::string>(pop_value());
                    struct_fields[struct_name][field_name] = pop_value();
                    break;
                }
                case poliz_t::PUSH_INT:
                case poliz_t::PUSH_REAL:
                case poliz_t::PUSH_STRING:
                case poliz_t::PUSH_BOOL:
                    push_value(item.operand);
                    break;
                case poliz_t::STRUCT_NAME: {
                	push_value(std::get<std::string>(item.operand));
                	break;
                }
                case poliz_t::BREAK:
                case poliz_t::NOP:
                    break;
            }
        }
    }

private:
    std::stack<std::variant<int, double, std::string, bool>> stack;
    std::map<std::string, std::variant<int, double, std::string, bool>> variables;
    std::map<std::string, std::map<std::string, std::variant<int, double, std::string, bool>>> struct_fields;
    int pc; 
    
    void push_value(const std::variant<int, double, std::string, bool>& value) {
        stack.push(value);
    }
    
    std::variant<int, double, std::string, bool> pop_value() {
        if (stack.empty()) {
            throw std::runtime_error("Стек пуст");
        }
        auto val = stack.top();
        stack.pop();
        return val;
    }
    
    bool make_bool(const std::variant<int, double, std::string, bool>& value) {
        if (std::holds_alternative<bool>(value)) {
            return std::get<bool>(value);
        }
        if (std::holds_alternative<int>(value)) {
            return std::get<int>(value) != 0;
        }
        if (std::holds_alternative<double>(value)) {
            return std::get<double>(value) != 0;
        }
        return true; 
    }
    
    std::variant<int, double, std::string, bool> add(const std::variant<int, double, std::string, bool>& a, const std::variant<int, double, std::string, bool>& b) {
        
        if (std::holds_alternative<std::string>(a) && std::holds_alternative<std::string>(b)) {
            return std::get<std::string>(a) + std::get<std::string>(b);
        }
        if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b)) {
            return std::get<int>(a) + std::get<int>(b);
        }
        if (std::holds_alternative<double>(a) || std::holds_alternative<double>(b)) {
            double da = std::holds_alternative<int>(a) ? std::get<int>(a) : std::get<double>(a);
            double db = std::holds_alternative<int>(b) ? std::get<int>(b) : std::get<double>(b);
            return da + db;
        }
        throw std::runtime_error("Недопустимые операнды для '+'");
    }
    
    
	std::variant<int, double, std::string, bool> sub( const std::variant<int, double, std::string, bool>& a, const std::variant<int, double, std::string, bool>& b) {
		
		if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b)) {
		    return std::get<int>(a) - std::get<int>(b);
		}
		if (std::holds_alternative<double>(a) || std::holds_alternative<double>(b)) {
		    double da = std::holds_alternative<int>(a) ? std::get<int>(a) : std::get<double>(a);
		    double db = std::holds_alternative<int>(b) ? std::get<int>(b) : std::get<double>(b);
		    return da - db;
		}
		throw std::runtime_error("Недопустимые операнды для '-'");
	}


	std::variant<int, double, std::string, bool> mul( const std::variant<int, double, std::string, bool>& a, const std::variant<int, double, std::string, bool>& b) {
		
		if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b)) {
		    return std::get<int>(a) * std::get<int>(b);
		}
		if (std::holds_alternative<double>(a) || std::holds_alternative<double>(b)) {
		    double da = std::holds_alternative<int>(a) ? std::get<int>(a) : std::get<double>(a);
		    double db = std::holds_alternative<int>(b) ? std::get<int>(b) : std::get<double>(b);
		    return da * db;
		}
		throw std::runtime_error("Недопустимые операнды для '*'");
	}

	std::variant<int, double, std::string, bool> div( const std::variant<int, double, std::string, bool>& a, const std::variant<int, double, std::string, bool>& b) {
		
		if (std::holds_alternative<int>(b) && std::get<int>(b) == 0) {
		    throw std::runtime_error("Деление на 0");
		}
		if (std::holds_alternative<double>(b) && std::get<double>(b) == 0.0) {
		    throw std::runtime_error("Деление на 0");
		}

		if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b)) {
		    return std::get<int>(a) / std::get<int>(b);
		}
		if (std::holds_alternative<double>(a) || std::holds_alternative<double>(b)) {
		    double da = std::holds_alternative<int>(a) ? std::get<int>(a) : std::get<double>(a);
		    double db = std::holds_alternative<int>(b) ? std::get<int>(b) : std::get<double>(b);
		    return da / db;
		}
		throw std::runtime_error("Недопустимые операнды для '/'");
	}


	bool less( const std::variant<int, double, std::string, bool>& a, const std::variant<int, double, std::string, bool>& b) {
		
		if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b)) {
		    return std::get<int>(a) < std::get<int>(b);
		}
		if (std::holds_alternative<double>(a) || std::holds_alternative<double>(b)) {
		    double da = std::holds_alternative<int>(a) ? std::get<int>(a) : std::get<double>(a);
		    double db = std::holds_alternative<int>(b) ? std::get<int>(b) : std::get<double>(b);
		    return da < db;
		}
		if (std::holds_alternative<std::string>(a) && std::holds_alternative<std::string>(b)){
		    return std::get<std::string>(a) < std::get<std::string>(b);
		}
		return false;
	}


	bool greater( const std::variant<int, double, std::string, bool>& a, const std::variant<int, double, std::string, bool>& b) {
		
		if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b)) {
		    return std::get<int>(a) > std::get<int>(b);
		}
		if (std::holds_alternative<double>(a) || std::holds_alternative<double>(b)) {
		    double da = std::holds_alternative<int>(a) ? std::get<int>(a) : std::get<double>(a);
		    double db = std::holds_alternative<int>(b) ? std::get<int>(b) : std::get<double>(b);
		    return da > db;
		}
		if (std::holds_alternative<std::string>(a) && std::holds_alternative<std::string>(b)) {
		    return std::get<std::string>(a) > std::get<std::string>(b);
		}
		return false;
	}

	bool L_E( const std::variant<int, double, std::string, bool>& a, const std::variant<int, double, std::string, bool>& b) { 
		if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b)) {
		    return std::get<int>(a) <= std::get<int>(b);
		}
		if (std::holds_alternative<double>(a) || std::holds_alternative<double>(b)) {
		    double da = std::holds_alternative<int>(a) ? std::get<int>(a) : std::get<double>(a);
		    double db = std::holds_alternative<int>(b) ? std::get<int>(b) : std::get<double>(b);
		    return da <= db;
		}
		if (std::holds_alternative<std::string>(a) && std::holds_alternative<std::string>(b)) {
		    return std::get<std::string>(a) <= std::get<std::string>(b);
		}
		return false;
	}

	bool G_E( const std::variant<int, double, std::string, bool>& a, const std::variant<int, double, std::string, bool>& b) {
		
		if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b)) {
		    return std::get<int>(a) >= std::get<int>(b);
		}
		if (std::holds_alternative<double>(a) || std::holds_alternative<double>(b)) {
		    double da = std::holds_alternative<int>(a) ? std::get<int>(a) : std::get<double>(a);
		    double db = std::holds_alternative<int>(b) ? std::get<int>(b) : std::get<double>(b);
		    return da >= db;
		}
		if (std::holds_alternative<std::string>(a) && std::holds_alternative<std::string>(b)) {
		    return std::get<std::string>(a) >= std::get<std::string>(b);
		}
		return false;
	}

	bool equal( const std::variant<int, double, std::string, bool>& a, const std::variant<int, double, std::string, bool>& b) {
		if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b)) {
		    return std::get<int>(a) == std::get<int>(b);
		}
		if (std::holds_alternative<double>(a) || std::holds_alternative<double>(b)) {
		    double da = std::holds_alternative<int>(a) ? std::get<int>(a) : std::get<double>(a);
		    double db = std::holds_alternative<int>(b) ? std::get<int>(b) : std::get<double>(b);
		    return da == db;
		}
		if (std::holds_alternative<std::string>(a) && std::holds_alternative<std::string>(b)) {
		    return std::get<std::string>(a) == std::get<std::string>(b);
		}
		if (std::holds_alternative<bool>(a) && std::holds_alternative<bool>(b)) {
		    return std::get<bool>(a) == std::get<bool>(b);
		}
		return false;
	}

	bool not_equal( const std::variant<int, double, std::string, bool>& a, const std::variant<int, double, std::string, bool>& b) {
		return !equal(a, b);
	}
};



int main () {
	std::string program = "";
    
    std::string line;
 
    std::ifstream in("hello.txt"); 
    if (in.is_open())
    {
        while (std::getline(in, line))
        {
        	program = program + line + "\n";
            std::cout << line << std::endl;
        }
    }
    in.close();
    
    try {
		// Лексический анализ
        Lex_Analyze tester(program);
        auto parse_program = tester.Lex_Analyzer();
        
        // Синтаксический анализ
        Syntax_Analyze analyzer(parse_program);
        auto program = analyzer.syn_analyzer();
        
         // Семантический анализ
        Semantic_Analyze checker;
        checker.semantic(*program);
        
        // Генерация ПОЛИЗа
        Poliz_Generate generator;
        auto poliz = generator.generate(*program);
        
        // Интерпретор
        Interpreter interpreter;
        interpreter.execute(poliz);
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
