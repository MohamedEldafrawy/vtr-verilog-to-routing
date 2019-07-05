/*
Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef AST_ELABORATE_H
#define AST_ELABORATE_H

void remove_generate(ast_node_t *node);
int simplify_ast_module(ast_node_t **ast_module);
void reduce_assignment_expression();
void reduce_assignment_expression(ast_node_t *ast_module);
void find_assign_node(ast_node_t *node, std::vector<ast_node_t *> list, char *module_name);
ast_node_t *find_top_module();

struct enode
{
	struct
	{
		short operation;
		int data;
		std::string variable;
	}type;

	int id;
	int flag;
	int priority;
	struct enode *next;
	struct enode *pre;

};

void record_tree_info(ast_node_t *node);
void store_exp_list(ast_node_t *node);
void create_enode(ast_node_t *node);
bool simplify_expression();
bool adjoin_constant();
enode *replace_enode(int data, enode *t, int mark);
bool combine_constant();
bool delete_continuous_multiply();
void construct_new_tree(enode *tail, ast_node_t *node, int line_num, int file_num);
void reduce_enode_list();
enode *find_tail(enode *node);
int check_exp_list(enode *tail);
void create_ast_node(enode *temp, ast_node_t *node, int line_num, int file_num);
void create_op_node(ast_node_t *node, enode *temp, int line_num, int file_num);
void free_exp_list();
void change_exp_list(enode *beign, enode *end, enode *s, int flag);
enode *copy_enode_list(enode *new_head, enode *begin, enode *end);
void copy_enode(enode *node, enode *new_node);
bool check_tree_operation(ast_node_t *node);
void check_operation(enode *begin, enode *end);
bool check_mult_bracket(std::vector<int> list);

#endif