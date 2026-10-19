#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#define MAXN 100000
#define MX_VAR_LEN 7

#define TODO() do { \
    fprintf(stderr, "TODO: %s:%d\n", __FILE__, __LINE__); \
    abort(); \
} while (0)



typedef enum {
	Expr_Var,
	Expr_Fun,
	Expr_App
} Expr_Type;

typedef struct {
	char ident[MX_VAR_LEN+1];
	int id;
} Symbol;

int global_symbol_counter = 0;
Symbol symbol(const char* ident) {
	if (strlen(ident) > MX_VAR_LEN) {
		printf("Identifier too long for symbol: %s", ident);
		assert(false);
	}
	Symbol symbol;
	strcpy(symbol.ident, ident);
	symbol.id = global_symbol_counter++;
	return symbol;
}

typedef struct {
	Expr_Type expr_type;
	union {
		Symbol var;
		struct {
			Symbol param;
			int body;
		} fun;
		struct {
			int lhs;
			int rhs;
		} app;
	} as;
} Expr;

Expr blob[MAXN];
int blob_index = 1;
int x_cnt = 0;

int new_var(Symbol symbol) {
	assert(blob_index < MAXN);
	blob[blob_index].expr_type = Expr_Var;
	blob[blob_index].as.var = symbol;
	return blob_index++;
}

int new_fun(Symbol param, int body) {
	assert(blob_index < MAXN);
	blob[blob_index].expr_type = Expr_Fun;
	blob[blob_index].as.fun.param = param;
	blob[blob_index].as.fun.body = body;
	return blob_index++;
}

int new_app(int lhs, int rhs) {
	assert(blob_index < MAXN);
	blob[blob_index].expr_type = Expr_App;
	blob[blob_index].as.app.lhs = lhs;
	blob[blob_index].as.app.rhs = rhs;
	return blob_index++;
}

bool symbol_eq(Symbol a, Symbol b) {
	return strcmp(a.ident, b.ident) == 0 && a.id == b.id;
}

void change_symbol_id(Symbol symbol, int new_id, int expr) {
	switch(blob[expr].expr_type) {
		case Expr_Var:
			if (symbol_eq(blob[expr].as.var, symbol)) {
				blob[expr].as.var.id = new_id;
			}
			return;
		case Expr_App:
			change_symbol_id(symbol, new_id, blob[expr].as.app.lhs);
			change_symbol_id(symbol, new_id, blob[expr].as.app.rhs);
			return;
		case Expr_Fun:
			assert(!symbol_eq(blob[expr].as.fun.param, symbol));
			change_symbol_id(symbol, new_id, blob[expr].as.fun.body);
	}
}

int create_copy(int expr) {
	switch (blob[expr].expr_type) {
		case Expr_Var:
			return new_var(blob[expr].as.var);
		case Expr_App:
			int new_lhs = create_copy(blob[expr].as.app.lhs);
			int new_rhs = create_copy(blob[expr].as.app.rhs);
			return new_app(new_lhs, new_rhs);
		case Expr_Fun:
			int new_body = create_copy(blob[expr].as.fun.body);
			Symbol symbol = blob[expr].as.fun.param;
			int new_id = global_symbol_counter++;
			change_symbol_id(symbol, new_id, new_body);
			symbol.id = new_id;
			return new_fun(symbol, new_body);
	}
	assert(false);
}



int substitute(Symbol symbol, int expr, int new_expr) {
	switch (blob[expr].expr_type) {
		case Expr_Var:
			if (symbol_eq(blob[expr].as.var, symbol)) {
				return create_copy(new_expr);
			} else {
				return expr;
			}
		case Expr_Fun:
			assert(!symbol_eq(blob[expr].as.fun.param, symbol));
			int new_body = substitute(symbol, blob[expr].as.fun.body, new_expr);
			return new_fun(blob[expr].as.fun.param, new_body);
		case Expr_App:
			int new_lhs = substitute(symbol, blob[expr].as.app.lhs, new_expr);
			int new_rhs = substitute(symbol, blob[expr].as.app.rhs, new_expr);
			return new_app(new_lhs, new_rhs);
	}
	assert(false);
}

// Returns -1 if no reductions
int beta_reduce(int expr) {
	switch (blob[expr].expr_type) {
		case Expr_Var:
			return -1;
		case Expr_Fun:
			int new_body = beta_reduce(blob[expr].as.fun.body);
			if (new_body == -1) {
				return -1;
			}
			return new_fun(blob[expr].as.fun.param, new_body);
		case Expr_App:
			int lhs = blob[expr].as.app.lhs;
			int rhs = blob[expr].as.app.rhs;
			if (blob[lhs].expr_type == Expr_Fun) {
				return substitute(blob[lhs].as.fun.param, blob[lhs].as.fun.body, rhs);
			}
			int new_lhs = beta_reduce(blob[expr].as.app.lhs);
			if (new_lhs != -1) {
				return new_app(new_lhs, rhs);
			}
			int new_rhs = beta_reduce(blob[expr].as.app.rhs);
			if (new_rhs != -1) {
				return new_app(lhs, new_rhs);
			}
			return -1;
	}
	assert(false);
}

void print_expr(int expr) {
	switch (blob[expr].expr_type) {
		case Expr_Var:
			printf("%s_%d", blob[expr].as.var.ident, blob[expr].as.var.id);
			return;
		case Expr_App:
			print_expr(blob[expr].as.app.lhs);
			printf(" ");
			print_expr(blob[expr].as.app.rhs);
			return;
		case Expr_Fun:
			printf("(\\%s_%d.", blob[expr].as.fun.param.ident, blob[expr].as.fun.param.id);
			print_expr(blob[expr].as.fun.body);
			printf(")");
	}
}

int main() {
	printf("Hello, world\n");
	Symbol x = symbol("x");
	Symbol y = symbol("y");
	int expr = new_app(new_fun(x, new_app(new_app(new_var(x), new_var(x)),new_var(x))), new_fun(y, new_app(new_app(new_var(y), new_var(y)), new_var(y))));
	for (int i = 0; expr != -1 && i < 100; i++) {
		print_expr(expr);
		printf("\n");
		expr = beta_reduce(expr);
	}

	return 0;
}
