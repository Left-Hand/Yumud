#include "src/testbench/tb.h"

#include <map>
#include <vector>
#include <string>
#include <bits/stl_pair.h>
#include <stack>
#include <algorithm>

#include "algo/random/random_generator.hpp"
#include "core/math/real.hpp"
#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"

#include "hal/bus/uart/uarthw.hpp"
// using namespace std;
using std::stack;
using std::string;
using std::map;
using std::vector;
using std::pair;
using std::make_pair;
using std::min;
using std::max;
#define endl "\r\n";

static void getline(OutputStream & logger, string & str){
    // String temp_str;
    // while(true){
    //     if(logger.available()){
    //         char chr;
    //         logger.read(chr);
    //         if(chr == '\n'){
    //             if(temp_str.length()){
    //                 str = temp_str.c_str();
    //                 return;
    //             }
    //             temp_str = "";
    //         }else{
    //             temp_str.concat(chr);
    //         }
    //     }
    //     else{
    //         delay(400);
    //         logger.print(' ');
    //     }
    // }
	str = "1";
}

class
	Interpreter{
        protected:
        OutputStream & logger;
        RandomGenerator rnd;
        private:
		struct
			SYMBOL{
				string
					sym,
					dat;
				int
					typ,
					pri;
                int
					pos;
				real_t
					val;
					
				//0 - const value
				//1 - operator
				//2 - point symbol
				//3 - function
				//4 - sys vars
				//5 - usr var (number type)
				//6 - string
				//7 - null
				//8 - END
				//9 - usr var (string type)
			};
		
		vector <SYMBOL>
			registered_symbols;
			
		vector <map<string,SYMBOL> >
			vars;
			
		map <string,vector <string> >
			usr_func;
			
		vector <pair<int,string> >
			err_msg;
			
		bool
			test_mode,
			err_strict;
			
		int
			max_solve_loop,
			max_integrate_loop;
		
		real_t
			prec,
			delt;
			
		public:
			
		SYMBOL
			make_symb(string sym,int typ,int pri, int pos = 0,real_t val = 0){
				SYMBOL
					res;
				if (typ == 6){
					res.sym = "$string";
					res.dat = sym;
				}
				else
					res.sym = sym;
				res.typ = typ;
				res.pri = pri;
				res.pos = pos;
				res.val = val;
				return
					res;	
			}
		
		void
			make_symb_and_push(string sym,int typ,int pri,int pos = 0,real_t val = 0){
				registered_symbols.push_back(make_symb(sym,typ,pri,pos,val));
			}
			
		void
			init_buildin_symbols(){
				
				make_symb_and_push("null",7,-1);
				
				make_symb_and_push("+",1,6);
				make_symb_and_push("-",1,6);
				make_symb_and_push("*",1,5);
				make_symb_and_push("/",1,5);
				make_symb_and_push("%",1,5);
				make_symb_and_push("^",1,4);
				make_symb_and_push("&&",1,7);
				make_symb_and_push("||",1,7);
				make_symb_and_push("!=",1,7);
				make_symb_and_push("==",1,7);
				make_symb_and_push(">=",1,7);
				make_symb_and_push("<=",1,7);
				make_symb_and_push(">",1,7);
				make_symb_and_push("<",1,7);
				make_symb_and_push("(",2,10);
				make_symb_and_push(")",2,10);
				make_symb_and_push(",",2,9);
				make_symb_and_push("=",1,8);
				make_symb_and_push(":=",1,8);
				
				make_symb_and_push("Sin",3,-1);
				make_symb_and_push("Cos",3,-1);
				make_symb_and_push("Tan",3,-1);
				make_symb_and_push("Csc",3,-1);
				make_symb_and_push("Sec",3,-1);
				make_symb_and_push("Cot",3,-1);
				make_symb_and_push("ASin",3,-1);
				make_symb_and_push("ACos",3,-1);
				make_symb_and_push("ATan",3,-1);
				
				make_symb_and_push("Sh",3,-1);
				make_symb_and_push("Ch",3,-1);
				make_symb_and_push("Th",3,-1);
				
				make_symb_and_push("Ln",3,-1);
				make_symb_and_push("Log",3,-1);
				make_symb_and_push("Exp",3,-1);
				make_symb_and_push("Sgn",3,-1);
				make_symb_and_push("Abs",3,-1);
				make_symb_and_push("Sqrt",3,-1);
				
				make_symb_and_push("Int",3,-1);
				make_symb_and_push("Floor",3,-1);
				make_symb_and_push("Frac",3,-1);
				make_symb_and_push("Ceiling",3,-1);
				make_symb_and_push("Zero",3,-1);
				
				make_symb_and_push("Reverse",3,-1);
				make_symb_and_push("StringReverse",3,-1);
				make_symb_and_push("StringSort",3,-1);
				make_symb_and_push("Length",3,-1);
				make_symb_and_push("Erase",3,-1);
				
				make_symb_and_push("Sort",3,-1);
				make_symb_and_push("Max",3,-1);
				make_symb_and_push("Min",3,-1);
				
				make_symb_and_push("Let",3,-1);
				make_symb_and_push("Pick",3,-1);
				make_symb_and_push("Throw",3,-1);
				make_symb_and_push("If",3,-1);
				make_symb_and_push("While",3,-1);
				make_symb_and_push("Do",3,-1);
				make_symb_and_push("Dec",3,-1);
				make_symb_and_push("Inc",3,-1);
				make_symb_and_push("First",3,-1);
				make_symb_and_push("Last",3,-1);
				make_symb_and_push("Delete",3,-1);
				make_symb_and_push("Clear",3,-1);
				make_symb_and_push("Type",3,-1);
				
				make_symb_and_push("Char",3,-1);
				make_symb_and_push("TakeChar",3,-1);
				make_symb_and_push("Pos",3,-1);
				make_symb_and_push("Reduce",3,-1);
				
				make_symb_and_push("Solve",3,-1);
				make_symb_and_push("Integrate",3,-1);
				make_symb_and_push("Diff",3,-1);
				make_symb_and_push("Sum",3,-1);
				make_symb_and_push("Quadrature",3,-1);
				
				make_symb_and_push("Print",3,-1);
				make_symb_and_push("Input",3,-1);
				
				make_symb_and_push("Pi",4,-1,0, real_t(3.1415926535897932384626433832795));
				make_symb_and_push("E",4,-1,0, real_t(2.7182818284590452353602874713527));
				
			}
	
		bool
			is_number(char c){
				return
					(c == '.') || (c >= '0' && c <= '9');
			}
	
		bool
			is_string(char c){
				return
					c == '\"';
			}
			
		bool
			is_integer(real_t x){
				return
					x == (int) x;
			}
			
		bool
			in_string(char c,string s){
				for(int i = 0;i < (int)s.size();i++)
					if (s[i] == c)
						return
							1;
				return
					0;
			}
			
		int
			token_t(char c){
				if ((c >= 'a' && c <= 'z')
				|| (c >= 'A' && c <= 'Z')
				|| (c >= '0' && c <= '9')
				|| c == '_')
					return 1;
				return
					0;
			}
	
		int
			next_token(int &x,string &expr){
				int
					res = expr.size(),
					cnt;
				string
					temp;
				
				if (in_string(expr[x],"+-*/=!%^&<>(),|")){
					for(;x < (int)expr.size();x++)
						for(int i = 0;i < (int)registered_symbols.size();i++){
							if (registered_symbols[i].sym.size() > expr.size() - x)
								continue;
							cnt = 0;
							for(int j = 0;j < (int)registered_symbols[i].sym.size();j++)
								if (registered_symbols[i].sym[j] == expr[j + x])
									cnt++;
								else
									break;
							if (cnt == (int)registered_symbols[i].sym.size())
								return
									i;
						}
					return
						res;
				}
				for(;x < (int)expr.size() && token_t(expr[x]);x++)
				 	temp.push_back(expr[x]);
				for(int i = 0;i < (int)registered_symbols.size();i++)
					if (registered_symbols[i].sym == temp){
							res = i;
							x -= temp.size();
							break;
						}
				return
					res;			
			}
	
		string
			get_string(string &expr,int &addr){
				string
					res;
				if (expr[addr] == '\"')
					addr++;
				for(;addr < (int)expr.size();addr++){
					if (expr[addr] == '\"'){
						addr++;
						break;
					}
					if (expr[addr] == '\\' && addr < (int)expr.size() - 1){
						if (expr[addr + 1] == '\"'){
							res.push_back('\"');
							addr++;
							continue;
						}
						if (expr[addr + 1] == 'n'){
							res.push_back('\n');
							addr++;
							continue;
						}
						if (expr[addr + 1] == 'r'){
							res.push_back('\r');
							addr++;
							continue;
						}
						if (expr[addr + 1] == 't'){
							res.push_back('\t');
							addr++;
							continue;
						}
						err_msg.push_back(make_pair(addr,"Error : Invaild char detected."));
						continue;
					}
					res.push_back(expr[addr]);
				}
				return
					res;
			}
	
		real_t
			get_number(string &expr,int &addr){
				real_t res,base = real_t(0.1);
				real_t tres = 0;
				bool
					point_flag = false;
				for(;addr < (int)expr.size();addr++){
					if (expr[addr] == '.'){
						if (point_flag)
							err_msg.push_back(make_pair(addr,"Error : Invaild number format."));
						point_flag = 1;
						continue;
					}
					if (expr[addr] >= '0' && expr[addr] <= '9')
						if (point_flag){
							tres = tres + (expr[addr] - '0') * base;
							base /= real_t(10.0);
						}
						else
							tres = tres * 10 + expr[addr] - '0';
					else
						break;
					// if (abs(tres) >= 1.7976931348623157E+308)
						// err_msg.push_back(make_pair(addr,"Warning : Value too large. Mistakes may appear."));
				}
				res = (real_t) tres;
				return
					res;
			}
	
		void
			print_list(vector <SYMBOL> t){
				if (!t.size()){
					logger << "NULL";
					return;
				}
				if (t.size() != 1)
					logger << '(';
				for(int i = 0;i < (int)t.size();i++){
					if (!t[i].typ || t[i].typ == 5 || t[i].typ == 4)
						logger << t[i].val;
					if (t[i].typ == 9 || t[i].typ == 6)
						logger << "\""+ t[i].dat +"\"";
					if (t.size() != 1 && i < (int)t.size() - 1)
						logger << ',';
				}
				if (t.size() != 1)
					logger << ')';
			}
			
		void
			print_err(){
				for(int i = 0;i < (int)err_msg.size();i++){
					logger << "Pos " << err_msg[i].first << ' ' << err_msg[i].second << endl; 
				}
			}
		
		void
			lexer(vector <SYMBOL> &tokens,string &expr){
				real_t
					res;
				int
					nxt_token,nxt_symb;
				string
					temp;
					
				tokens.clear();
				
				for(int i = 0;i < (int)expr.size();){
					
					for(;i < (int)expr.size() && expr[i] == ' ';i++);  //Reduce Space
					
					if (is_number(expr[i])){
						tokens.push_back(make_symb("$number",0,-1,i,get_number(expr,i)));
						continue;
					}
					
					if (is_string(expr[i])){
						tokens.push_back(make_symb(get_string(expr,i),6,-1,i,0));
						continue;
					}
					
					nxt_token = i;
					nxt_symb = next_token(nxt_token,expr);
					if (nxt_token == i){
						nxt_token += registered_symbols[nxt_symb].sym.size();
						tokens.push_back(registered_symbols[nxt_symb]);
						tokens[tokens.size() - 1].pos = i;
					}
					else{
						temp.clear();
						for(int j = i;j < nxt_token;j++)
							temp.push_back(expr[j]);
						tokens.push_back(make_symb(temp,5,-1,i,0));
					}
					i = nxt_token;
				}
				
				if (!tokens.empty() && 
					(tokens[0].sym == "-" || tokens[0].sym == "+"))
						tokens.insert(tokens.begin(),make_symb("$number",0,-1,-1,0));
						
				for(int i = 1;i < (int)tokens.size();i++){
					if ((tokens[i - 1].sym == "(" || tokens[i - 1].sym == "="
					  || tokens[i - 1].sym == "!=" || tokens[i - 1].sym == ">=" ||
					  tokens[i - 1].sym == "<=" || tokens[i - 1].sym == "==" ||
					  tokens[i - 1].sym == ">" || tokens[i - 1].sym == "<" ||
					  tokens[i - 1].sym == ",")
					&& (tokens[i].sym == "-" || tokens[i].sym == "+")){
					 	tokens.insert(tokens.begin() + i,make_symb("$number",0,-1,-1,0));
					 	i++;
					 	continue;
					 }
					if ((!tokens[i - 1].typ &&
					    (tokens[i].typ >= 3 && tokens[i].typ <= 5)) ||
						(!tokens[i].typ &&
					    (tokens[i - 1].typ >= 3 && tokens[i - 1].typ <= 5)) ||
						(tokens[i - 1].sym == ")" && !tokens[i].typ) ||
						(!tokens[i - 1].typ && tokens[i].sym == "(") ||
						(tokens[i].typ == 3 && tokens[i - 1].sym == ")")){
					   	tokens.insert(tokens.begin() + i,make_symb("*",1,5,-1,0));
					   	i++;
					   	continue;
					   }
				}
				
				tokens.push_back(make_symb("END",8,0x7f,-1,0));
				
				if (test_mode){	
					logger << "Lexer:\n";
					for(int i = 0;i < (int)tokens.size();i++)
						logger << '	' << tokens[i].sym << '	' << tokens[i].typ << '	' << tokens[i].pos << ' ' << tokens[i].val << endl;
					logger << "End of line\n";
				}
				
			}
			
		void
			executer(vector <SYMBOL> &tokens,vector <SYMBOL> &res){
				stack <vector<SYMBOL> > val_stack;
				stack <SYMBOL> opr_stack;	
				stack <int> loop_pos;
				
				stack <int> solve_loop;
				stack <int> solve_state;
				stack <real_t> solve_temp;
				
				//state = 1 - calc f(x)
				//state = 2 - calc f(x + dx) and storage res to solve_temp
				//state = 3 - compare
				
				stack <int> integrate_loop;
				stack <real_t> integrate_result;
				stack <real_t> integrate_last;
				
				stack <int> diff_state;
				stack <real_t> diff_result;
				
				//state = 1 - calc set x
				//state = 2 - calc f(x)
				//state = 3 - calc f(x + dx) and return
				
				stack <int> sum_state;
				stack <real_t> sum_result;
				
				stack <int> quadrature_state;
				stack <real_t> quadrature_result;
				
				vector <SYMBOL>	tmp;	
				SYMBOL temp,tsa;
				bool flag;
				vector<SYMBOL> a,b,c,d;
				real_t ta,tb,tc,td;
				string sa,sb,sc;
				vector <real_t> tv;
				vector <string> tsv;
				int ia,ib,ic;
				
				err_msg.clear();
				res.clear();
				
				if (err_msg.size() && err_strict){
					res.push_back(make_symb("NULL",7,-1,-1,0));
					return;
				}
					
				for(int i = 0;i < (int)tokens.size();i++){
					
					if (test_mode){
						logger << "Now token = " << i << endl;
						logger << "Now the top of val_stack is\n";
						if (!val_stack.empty())
							print_list(val_stack.top());
						else
							logger << "NULL" << endl;
						logger << endl;
						logger << "Now the top of opr_stack is\n";
						if (!opr_stack.empty()){
							logger << opr_stack.top().sym << ' ' << opr_stack.top().typ << endl;
                        }else{
							logger << "NULL" << endl;
                        }
						logger << endl;
					}
					
					tmp.clear();
					
					if (tokens[i].typ == 0 || tokens[i].typ == 6){
						tmp.push_back(tokens[i]);
						val_stack.push(tmp);
						continue;
					}//numbers and strings
					
					if (tokens[i].typ == 5 || tokens[i].typ == 4 || tokens[i].typ == 9){
						if (tokens[i].typ == 5){
							flag = 0;
							for(int j = vars.size() - 1;j >= 0;j--)
								if (vars[j].count(tokens[i].sym)){
									flag = 1;
									tmp.push_back(vars[j][tokens[i].sym]);
									break;
								}
						}
						else{
							flag = 1;
							tmp.push_back(tokens[i]);
						}
						if (!flag){
							tmp.push_back(tokens[i]);
							tmp[tmp.size() - 1].val = 0;
							vars[vars.size() - 1][tokens[i].sym] = make_symb(tokens[i].sym,5,-1,-1,0);
							//err_msg.push_back(make_pair(tokens[i].pos,"Warning : Undefined var \"" + tokens[i].sym +"\". Initial value is 0."));
						}
						val_stack.push(tmp);
						continue;
					}//custom values
					
					if (tokens[i].typ == 1 || tokens[i].typ == 2 || tokens[i].typ == 3 || tokens[i].typ == 8){
						
						if (tokens[i].sym == "While" || tokens[i].sym == "Do"
						 || tokens[i].sym == "Solve" || tokens[i].sym == "Integrate"
						 || tokens[i].sym == "Diff" || tokens[i].sym == "Sum"
						 || tokens[i].sym == "Quadrature"){
							loop_pos.push(i);
							if (tokens[i].sym == "Solve"){
								solve_loop.push(max_solve_loop);
								solve_state.push(1);
							}
							else if (tokens[i].sym == "Integrate"){
								integrate_loop.push(max_integrate_loop);
							}
							else if (tokens[i].sym == "Diff"){
								diff_state.push(1);
							}
							else if (tokens[i].sym == "Sum"){
								sum_state.push(0);
								sum_result.push(0);
							}
							else if (tokens[i].sym == "Quadrature"){
								quadrature_state.push(0);
								quadrature_result.push(1);
							}
						}
									
						if (tokens[i].sym == "("){
							opr_stack.push(tokens[i]);
							continue;
						}
						
						if (tokens[i].typ != 8 && (opr_stack.empty() || opr_stack.top().pri > tokens[i].pri)){
							opr_stack.push(tokens[i]); 
							continue;
						}
						else{
							while(!opr_stack.empty() &&
								 (tokens[i].sym == ")" ? 
								 	(opr_stack.top().sym != "(") : 
									(opr_stack.top().pri <= tokens[i].pri))){
								temp = opr_stack.top();
								opr_stack.pop();
								if (temp.typ == 1){
									if (!val_stack.empty()){
										b = val_stack.top();
										val_stack.pop();
									}
									else{
										err_msg.push_back(make_pair(temp.pos,"Error : Missing argument."));
										b.clear();
										b.push_back(make_symb("null",7,-1,-1,0));
									}
									if (!val_stack.empty()){
										a = val_stack.top();
										val_stack.pop();
									}
									else{
										err_msg.push_back(make_pair(temp.pos,"Error : Missing argument."));
										a.clear();
										a.push_back(make_symb("null",7,-1,-1,0));
									}
									c.clear();
									
									for(int j = 0;j < max((int)a.size(),(int)b.size());j++){
										if (j >= min((int)a.size(),(int)b.size())
										&& (a.size() != 1 && b.size() != 1))
											break;
										if (a.size() == 1)
											ia = 0;
										else
											ia = j;
										if (b.size() == 1)
											ib = 0;
										else
											ib = j;
										
										if (((a[ia].typ == 0) && (b[ib].typ == 0)) ||
											((a[ia].typ == 5) && (b[ib].typ == 5)) ||
											((a[ia].typ == 4) && (b[ib].typ == 4)) ||
											((a[ia].typ == 0) && (b[ib].typ == 5)) ||
											((a[ia].typ == 0) && (b[ib].typ == 4)) ||
											((a[ia].typ == 5) && (b[ib].typ == 0)) ||
											((a[ia].typ == 4) && (b[ib].typ == 0)) ||
											((a[ia].typ == 5) && (b[ib].typ == 4)) ||
											((a[ia].typ == 4) && (b[ib].typ == 5)) ||
											(((temp.sym == "=" && a[ia].typ == 9 && b[ib].typ == 4)) ||
											 ((temp.sym == "=" && a[ia].typ == 9 && b[ib].typ == 5)) ||
											 ((temp.sym == "=" && a[ia].typ == 9 && b[ib].typ == 0)) ||
											 ((temp.sym == ":=" && a[ia].typ == 9 && b[ib].typ == 4)) ||
											 ((temp.sym == ":=" && a[ia].typ == 9 && b[ib].typ == 5)) ||
											 ((temp.sym == ":=" && a[ia].typ == 9 && b[ib].typ == 0)))){
												flag = 0;
												ta = a[ia].val;
												tb = b[ib].val;
												if (temp.sym == "+"){
													tc = ta + tb;
												}
												else if (temp.sym == "-"){
													tc = ta - tb;
												}
												else if (temp.sym == "*"){
													tc = ta * tb;
												}
												else if (temp.sym == "/"){
													tc = ta / tb;
												}
												else if (temp.sym == "%"){
													tc = (int) ta % (int) tb;
												}
												else if (temp.sym == "^"){
													tc = pow(ta,tb);
												}
												else if (temp.sym == "=="){
													tc = ta == tb;
												}
												else if (temp.sym == "!="){
													tc = ta != tb;
												}
												else if (temp.sym == ">="){
													tc = ta >= tb;
												}
												else if (temp.sym == "<="){
													tc = ta <= tb;
												}
												else if (temp.sym == ">"){
													tc = ta > tb;
												}
												else if (temp.sym == "<"){
													tc = ta < tb;
												}
												else if (temp.sym == "=" || temp.sym == ":="){
													if (b[ib].typ == 0 || b[ib].typ == 4 || b[ib].typ == 5){
														a[ia].val = b[ib].val;
														a[ia].typ = 5;
														vars[vars.size() - 1][a[ia].sym] = a[ia];
														flag = 1;
													}
													else
														err_msg.push_back(make_pair(temp.pos,"Error : Mismatch argument type."));
												}
												else
													err_msg.push_back(make_pair(temp.pos,"Error : \"" + temp.sym + "\" is an unknow symbol."));
												if (flag)
													c.push_back(a[ia]);
												else
													c.push_back(make_symb("$number",0,-1,-1,tc));
											}else if (a[ia].typ == 6 && b[ib].typ == 6){
												flag = 0;
												sa = a[ia].dat;
												sb = b[ib].dat;
												if (temp.sym == "+")
													sc = sa + sb;
												else if (temp.sym == "=="){
													flag = 1;
													ta = sa == sb;
												}
												else if (temp.sym == "!="){
													flag = 1;
													ta = sa != sb;
												}
												else if (temp.sym == ">="){
													flag = 1;
													ta = sa >= sb;
												}
												else if (temp.sym == "<="){
													flag = 1;
													ta = sa <= sb;
												}
												else if (temp.sym == ">"){
													flag = 1;
													ta = sa > sb;
												}
												else if (temp.sym == "<"){
													flag = 1;
													ta = sa < sb;
												}
												else
													err_msg.push_back(make_pair(temp.pos,"Error : " + temp.sym + " : Unknow symbol."));
												if (flag)
													c.push_back(make_symb("$number",0,-1,-1,ta));
												else
													c.push_back(make_symb(sc,6,-1,-1,0));
											}
											else if ((a[ia].typ == 5 && b[ib].typ == 6) ||
													 (a[ia].typ == 9 && b[ib].typ == 6) ||
													 (a[ia].typ == 5 && b[ib].typ == 9) ||
													 (a[ia].typ == 9 && b[ib].typ == 9)){
												flag = 0;
												if (temp.sym == "="){
													a[ia].dat = b[ib].dat;
													a[ia].typ = 9;
													vars[vars.size() - 1][a[ia].sym] = a[ia];
													flag = 1;
												}
												else if (temp.sym == "+")
													sc = a[ia].dat + b[ib].dat;
												if (flag)
													c.push_back(a[ia]);
												else
													c.push_back(make_symb(sc,6,-1,-1,0));
											}
											else if ((a[ia].typ == 6 && b[ib].typ == 9) ||
													 (a[ia].typ == 6 && b[ib].typ == 6)){
												if (temp.sym == "+")
													sc = a[ia].dat + b[ib].dat;
												c.push_back(make_symb(sc,6,-1,-1,0));
											}
											else{
												c.push_back(make_symb("null",7,-1,-1,0));
												err_msg.push_back(make_pair(temp.pos,"Error : Mismatch object types."));
											}
									}
									val_stack.push(c);
								}
								
								if (temp.sym == ","){
									if (!val_stack.empty()){
										b = val_stack.top();
										val_stack.pop();
									}
									else{
										b.clear();
										b.push_back(make_symb("null",7,-1,-1,0));
									}
									if (!val_stack.empty()){
										a = val_stack.top();
										val_stack.pop();
									}
									else{
										a.clear();
										a.push_back(make_symb("null",7,-1,-1,0));
									}
									a.insert(a.end(),b.begin(),b.end());
									val_stack.push(a);
								}
								
								if (temp.typ == 3){
									if (val_stack.empty()){
										err_msg.push_back(make_pair(temp.pos,"Error : Missing function argument."));
										continue;
									}
									a = val_stack.top();
									val_stack.pop();
									
									if (temp.sym == "Sort"){
										flag = 0;
										tv.clear();
										for(int j = 0;j < (int)a.size();j++)
											if (a[j].typ == 0)
												tv.push_back(a[j].val);
											else{
												flag = 1;
												break;
											}
										a.clear();
										if (!flag){
											sort(tv.begin(),tv.end());
											for(int j = 0;j < (int)tv.size();j++)
												a.push_back(make_symb("$number",0,-1,-1,tv[j]));
										}else{
											err_msg.push_back(make_pair(temp.pos,"Error : Mismatch object types."));
											a.push_back(make_symb("null",7,-1,-1,0));
										}
									}
									else if (temp.sym == "StringSort"){
										flag = 0;
										tsv.clear();
										for(int j = 0;j < (int)a.size();j++)
											if (a[j].typ == 6)
												tsv.push_back(a[j].dat);
											else{
												flag = 1;
												break;
											}
										a.clear();
										if (!flag){
											sort(tsv.begin(),tsv.end());
											for(int j = 0;j < (int)tsv.size();j++)
												a.push_back(make_symb(tsv[j],6,-1,-1,0));
										}else{
											err_msg.push_back(make_pair(temp.pos,"Error : Mismatch object types."));
											a.push_back(make_symb("null",7,-1,-1,0));
										}
									}
									else if (temp.sym == "Max"){
										flag = 0;
										if (a[0].typ != 0)
											flag = 1;
										else
											ta = a[0].val;
										for(int j = 1;!flag && j < (int)a.size();j++)
											if (a[j].typ == 0){
												if (a[j].val > ta)
													ta = a[j].val;
											}else
												flag = 1;
										a.clear();
										if (!flag)
											a.push_back(make_symb("$number",0,-1,-1,ta));
										else{
											err_msg.push_back(make_pair(temp.pos,"Error : Mismatch object types."));
											a.push_back(make_symb("NULL",7,-1,-1,0));
										}
									}
									else if (temp.sym == "Min"){
										flag = 0;
										if (a[0].typ != 0)
											flag = 1;
										else
											ta = a[0].val;
										for(int j = 1;!flag && j < (int)a.size();j++)
											if (a[j].typ == 0){
												if (a[j].val < ta)
													ta = a[j].val;
											}else
												flag = 1;
										a.clear();
										if (!flag)
											a.push_back(make_symb("$number",0,-1,-1,ta));
										else{
											err_msg.push_back(make_pair(temp.pos,"Error : Mismatch object types."));
											a.push_back(make_symb("null",7,-1,-1,0));
										}
									}
									else if (temp.sym == "Reverse"){
										reverse(a.begin(),a.end());
									}
									else if (temp.sym == "Let"){
										flag = 0;
										if (a.size() != 2){
											err_msg.push_back(make_pair(temp.pos,"Error : Incorrect argument quantity."));
											a.push_back(make_symb("null",7,-1,-1,0));
											flag = 1;
										}
										if (flag || !(a[0].typ == 5 || a[0].typ == 9) ||
													 (a[1].typ != 4 && a[1].typ != 0 && a[1].typ != 5 && a[1].typ != 6)){
											err_msg.push_back(make_pair(temp.pos,"Error : Incorrect argument type."));
											a.push_back(make_symb("null",7,-1,-1,0));
											flag = 1;
										}
										if (!flag){
											if (a[1].typ == 6){
												a[1].typ = 9;
												a[1].sym = a[0].sym;
											}else{
												a[1].typ = 5;
												a[1].sym = a[0].sym;
											}
											vars[vars.size() - 1][a[0].sym] = a[1];
											a.erase(a.begin(),a.begin() + 1);
										}
									}
									else if (temp.sym == "Pick"){
										flag = 0;
										if (a.size() <= 1){
											err_msg.push_back(make_pair(temp.pos,"Error : Incorrect argument quantity."));
											a.push_back(make_symb("null",7,-1,-1,0));
											flag = 1;
										}
										if (flag || (a[0].typ != 5 && a[0].typ != 4 && a[0].typ != 0) || !is_integer(a[0].val)){
											err_msg.push_back(make_pair(temp.pos,"Error : Incorrect argument type."));
											a.push_back(make_symb("null",7,-1,-1,0));
											flag = 1;
										}
										if (flag || a[0].val > a.size() - 2){
											err_msg.push_back(make_pair(temp.pos,"Error : Out of range."));
											a.push_back(make_symb("null",7,-1,-1,0));
											flag = 1;
										}
										if (!flag){
											tsa = a[(int) a[0].val + 1];
											a.clear();
											a.push_back(tsa);
										}
									}
									else if (temp.sym == "Last"){
										flag = 0;
										if (a.size() < 1){
											err_msg.push_back(make_pair(temp.pos,"Error : Incorrect argument quantity."));
											a.push_back(make_symb("null",7,-1,-1,0));
											flag = 1;
										}
										if (!flag){
											tsa = a[a.size() - 1];
											a.clear();
											a.push_back(tsa);
										}
									}
									else if (temp.sym == "First"){
										flag = 0;
										if (a.size() < 1){
											err_msg.push_back(make_pair(temp.pos,"Error : Incorrect argument quantity."));
											a.push_back(make_symb("null",7,-1,-1,0));
											flag = 1;
										}
										if (!flag){
											tsa = a[0];
											a.clear();
											a.push_back(tsa);
										}
									}
									else if (temp.sym == "Throw"){
										a.clear();
										a.push_back(make_symb("null",7,-1,-1,0));
									}
									else if (temp.sym == "Zero"){
										a.clear();
										a.push_back(make_symb("$number",0,-1,-1,0));
									}
									else if (temp.sym == "Dec"){
										flag = 0;
										if (a.size() != 1){
											err_msg.push_back(make_pair(temp.pos,"Error : Incorrect argument quantity."));
											a.push_back(make_symb("null",7,-1,-1,0));
											flag = 1;
										}
										if (flag || a[0].typ != 5){
											err_msg.push_back(make_pair(temp.pos,"Error : Incorrect argument type."));
											a.push_back(make_symb("null",7,-1,-1,0));
											flag = 1;
										}
										if (!flag){
											ta = (vars[vars.size() - 1][a[0].sym].val-=1);
											a.clear();
											a.push_back(make_symb("$number",0,-1,-1,ta));
										}
										
									}
									else if (temp.sym == "Inc"){
										flag = 0;
										if (a.size() != 1){
											err_msg.push_back(make_pair(temp.pos,"Error : Incorrect argument quantity."));
											a.push_back(make_symb("null",7,-1,-1,0));
											flag = 1;
										}
										if (flag || a[0].typ != 5){
											err_msg.push_back(make_pair(temp.pos,"Error : Incorrect argument type."));
											a.push_back(make_symb("null",7,-1,-1,0));
											flag = 1;
										}
										if (!flag){
											ta = vars[vars.size() - 1][a[0].sym].val += 1;
											a.clear();
											a.push_back(make_symb("$number",0,-1,-1,ta));
										}
									}
									else if (temp.sym == "If"){
										flag = 0;
										if (a.size() != 3){
											err_msg.push_back(make_pair(temp.pos,"Error : Incorrect argument quantity."));
											a.push_back(make_symb("null",7,-1,-1,0));
											flag = 1;
										}
										if (flag || (a[0].typ != 5 && a[0].typ != 4 && a[0].typ != 0)){
											err_msg.push_back(make_pair(temp.pos,"Error : Incorrect argument type."));
											a.push_back(make_symb("null",7,-1,-1,0));
											flag = 1;
										}
										if (!flag){
											if (a[0].val)
												tsa = a[1];
											else
												tsa = a[2];
											a.clear();
											a.push_back(tsa);
										}
									}
									else if (temp.sym == "While"){
										flag = 0;
										if (a.size() < 2){
											err_msg.push_back(make_pair(temp.pos,"Error : Incorrect argument quantity."));
											a.push_back(make_symb("null",7,-1,-1,0));
											flag = 1;
										}
										if (flag || loop_pos.empty()){
											err_msg.push_back(make_pair(temp.pos,"Error : Wrong loop structure."));
											flag = 1;
										}
										if (!flag){
											if (a[0].val){
												i = loop_pos.top();
												continue;
											}else{
												loop_pos.pop();
												a.erase(a.begin(),a.begin() + 1);
											}
										}
									}
									else if (temp.sym == "Do"){
										flag = 0;
										if (a.size() < 2){
											err_msg.push_back(make_pair(temp.pos,"Error : Incorrect argument quantity."));
											a.push_back(make_symb("null",7,-1,-1,0));
											flag = 1;
										}
										if (flag || loop_pos.empty()){
											err_msg.push_back(make_pair(temp.pos,"Error : Wrong loop structure."));
											flag = 1;
										}
										if (!flag){
											if (a[a.size() - 1].val){
												i = loop_pos.top();
												continue;
											}else{
												loop_pos.pop();
												a.erase(a.end() - 1,a.end());
											}
										}
									}
									else if (temp.sym == "Solve"){
										flag = 0;
										if (a.size() != 2){
											err_msg.push_back(make_pair(temp.pos,"Error : Incorrect argument quantity."));
											a.push_back(make_symb("null",7,-1,-1,0));
											flag = 1;
										}
										if (flag || a[0].typ != 5){
											err_msg.push_back(make_pair(temp.pos,"Error : Incorrect argument type."));
											a.push_back(make_symb("null",7,-1,-1,0));
											flag = 1;
										}
										if (!flag){
											if (solve_loop.top() == max_solve_loop)
												vars[vars.size() - 1][a[0].sym].val = 0.01_r * (rnd.update() % 100);
											if (solve_state.top() == 1){
												ic = solve_loop.top();
												solve_loop.pop();
												ic--;
												solve_loop.push(ic);
												solve_temp.push(a[1].val);
												vars[vars.size() - 1][a[0].sym].val += delt;
												solve_state.pop();
												solve_state.push(2);
												i = loop_pos.top();
												continue;
											}
											if (solve_state.top() == 2){
												ta = solve_temp.top();
												solve_temp.pop();
												if (abs((a[1].val - ta) / delt) < 1e-4_r)
													vars[vars.size() - 1][a[0].sym].val = real_t(0.01) * (rnd.update() % 100);
												vars[vars.size() - 1][a[0].sym].val -= delt * ta / (a[1].val - ta);
												//f(x)/df(x)=dx*f(x)/(f(x+dx)-f(x))
												solve_state.pop();
												solve_state.push(3);
												i = loop_pos.top();
												continue;
											}
											if (solve_state.top() == 3){
												if (solve_loop.top() <= 0 || abs(a[1].val) < prec){
													loop_pos.pop();
													solve_state.pop();
													solve_loop.pop();
													a.erase(a.begin() + 1,a.end());
												}
												else{
													solve_state.pop();
													solve_state.push(1);
													i = loop_pos.top();
													continue;
												}
											}
										}
									}
									else if (temp.sym == "Delete"){
										flag = 1;
										for(int j = 0;j < (int)a.size();j++)
											if (a[j].typ == 5 || a[j].typ == 9){
												vars[vars.size() - 1].erase(a[j].sym);
											}else{
												err_msg.push_back(make_pair(temp.pos,"Error : Invalid object type"));
												flag = 0;
											}
										a.clear();
										a.push_back(make_symb("$number",0,-1,-1,flag));
									}
									else if (temp.sym == "Clear"){
										a.clear();
										a.push_back(make_symb("$number",0,-1,-1,vars[vars.size() - 1].size()));
										vars[vars.size() - 1].clear();
									}
									else if (temp.sym == "Char"){
										flag = 0;
										if (a.size() != 1){
											err_msg.push_back(make_pair(temp.pos,"Error : Incorrect argument quantity."));
											a.push_back(make_symb("null",7,-1,-1,0));
											flag = 1;
										}
										if (flag || !(a[0].typ == 5 || a[0].typ == 0) || !is_integer(a[0].val)){
											err_msg.push_back(make_pair(temp.pos,"Error : Incorrect argument type."));
											a.push_back(make_symb("null",7,-1,-1,0));
											flag = 1;
										}
										if (!flag){
											sa.clear();
											sa.push_back((int) a[0].val);
											a.clear();
											a.push_back(make_symb(sa,6,-1,-1,0));
										}
									}
									else if (temp.sym == "Pos"){
										flag = 0;
										if (a.size() != 2){
											err_msg.push_back(make_pair(temp.pos,"Error : Incorrect argument quantity."));
											a.push_back(make_symb("null",7,-1,-1,0));
											flag = 1;
										}
										if (flag || !(a[0].typ == 6 || a[0].typ == 9) || !(a[1].typ == 6 || a[1].typ == 9)){
											err_msg.push_back(make_pair(temp.pos,"Error : Incorrect argument type."));
											a.push_back(make_symb("null",7,-1,-1,0));
											flag = 1;
										}
										if (!flag){
											ic = a[0].dat.find(a[1].dat);
											if (ic == (int)a[0].dat.npos)
												ic = -1;
											a.clear();
											a.push_back(make_symb("$number",0,-1,-1,ic));
										}
									}
									else if (temp.sym == "TakeChar"){
										flag = 0;
										if (a.size() != 2){
											err_msg.push_back(make_pair(temp.pos,"Error : Incorrect argument quantity."));
											a.push_back(make_symb("null",7,-1,-1,0));
											flag = 1;
										}
										if (flag || a[0].typ != 0 || !(a[1].typ == 6 || a[1].typ == 9)){
											err_msg.push_back(make_pair(temp.pos,"Error : Incorrect argument type."));
											a.push_back(make_symb("null",7,-1,-1,0));
											flag = 1;
										}
										if (!flag){
											if (a[0].val >= a[1].dat.size()){
												err_msg.push_back(make_pair(temp.pos,"Error : Out of range."));
												a.push_back(make_symb("null",7,-1,-1,0));
											}else{
												sa.clear();
												sa.push_back(a[1].dat[(int) a[0].val]);
												a.clear();
												a.push_back(make_symb(sa,6,-1,-1,0));
											}
										}
									}
									else if (temp.sym == "Reduce"){
										d.clear();
										for(int j = 0;j < (int)a.size();j++)
											if (a[j].typ != 7)
												d.push_back(a[j]);
										a.clear();
										for(int j = 0;j < (int)d.size();j++)
											a.push_back(d[j]);
									}
									else if (temp.sym == "Sum"){
										flag = 0;
										if (a.size() != 4){
											err_msg.push_back(make_pair(temp.pos,"Error : Incorrect argument quantity."));
											a.push_back(make_symb("null",7,-1,-1,0));
											flag = 1;
										}
										if (a[0].typ != 5 ||
											a[1].typ == 6 || a[2].typ == 6 || a[3].typ == 6 ||
											a[1].typ == 9 || a[2].typ == 9 || a[3].typ == 9 ||
											!is_integer(a[1].val) || !is_integer(a[2].val) || 
											(a[1].val > a[2].val) || flag){
												err_msg.push_back(make_pair(temp.pos,"Error : Incorrect argument type."));
												a.push_back(make_symb("null",7,-1,-1,0));
												flag = 1;
											}
										if (!flag){
											if (sum_state.top() == 0){
												vars[vars.size() - 1][a[0].sym].val = a[1].val;
												sum_state.pop();
												sum_state.push(1);
												i = loop_pos.top();
												continue;
											}
											else{
												tc = sum_result.top();
												sum_result.pop();
												tc += a[3].val;
												if (a[0].val >= a[2].val){
													sum_state.pop();
													loop_pos.pop();
													a.clear();
													a.push_back(make_symb("$number",0,-1,-1,tc));
												}
												else{
													sum_result.push(tc);
													vars[vars.size() - 1][a[0].sym].val += 1;
													i = loop_pos.top();
													continue;
												}
											}
										}
									}
									else if (temp.sym == "Quadrature"){
										flag = 0;
										if (a.size() != 4){
											err_msg.push_back(make_pair(temp.pos,"Error : Incorrect argument quantity."));
											a.push_back(make_symb("null",7,-1,-1,0));
											flag = 1;
										}
										if (a[0].typ != 5 ||
											a[1].typ == 6 || a[2].typ == 6 || a[3].typ == 6 ||
											a[1].typ == 9 || a[2].typ == 9 || a[3].typ == 9 ||
											!is_integer(a[1].val) || !is_integer(a[2].val) || 
											(a[1].val > a[2].val) || flag){
												err_msg.push_back(make_pair(temp.pos,"Error : Incorrect argument type."));
												a.push_back(make_symb("null",7,-1,-1,0));
												flag = 1;
											}
										if (!flag){
											if (quadrature_state.top() == 0){
												vars[vars.size() - 1][a[0].sym].val = a[1].val;
												quadrature_state.pop();
												quadrature_state.push(1);
												i = loop_pos.top();
												continue;
											}
											else{
												tc = quadrature_result.top();
												quadrature_result.pop();
												tc *= a[3].val;
												if (a[0].val >= a[2].val){
													quadrature_state.pop();
													loop_pos.pop();
													a.clear();
													a.push_back(make_symb("$number",0,-1,-1,tc));
												}
												else{
													quadrature_result.push(tc);
													vars[vars.size() - 1][a[0].sym].val += 1;
													i = loop_pos.top();
													continue;
												}
											}
										}
									}
									else if (temp.sym == "Integrate"){
										flag = 0;
										if (a.size() != 4){
											err_msg.push_back(make_pair(temp.pos,"Error : Incorrect argument quantity."));
											a.push_back(make_symb("null",7,-1,-1,0));
											flag = 1;
										}
										if (a[0].typ != 5 ||
											a[1].typ == 6 || a[2].typ == 6 || a[3].typ == 6 ||
											a[1].typ == 9 || a[2].typ == 9 || a[3].typ == 9 || flag){
												err_msg.push_back(make_pair(temp.pos,"Error : Incorrect argument type."));
												a.push_back(make_symb("null",7,-1,-1,0));
												flag = 1;
											}
										if (!flag){
											ic = integrate_loop.top();
											integrate_loop.pop();
											if (ic <= 0){
												loop_pos.pop();
												ta = integrate_result.top();
												integrate_result.pop();
												a.clear();
												a.push_back(make_symb("$number",0,-1,-1,ta));
											}
											else{
												if (ic == max_integrate_loop){
													tc = 0;
													vars[vars.size() - 1][a[0].sym].val = a[1].val;
												}
												else{
													tc = integrate_result.top();
													integrate_result.pop();
													ta = (a[2].val - a[1].val) / (max_integrate_loop - 1);
													vars[vars.size() - 1][a[0].sym].val += ta;
													td = a[3].val;
													if (!integrate_last.empty()){
														tc += (td + integrate_last.top()) * ta * real_t(0.5);
														integrate_last.pop();
													}
													integrate_last.push(td);
												}
												ic--;
												integrate_result.push(tc);
												integrate_loop.push(ic);
												i = loop_pos.top();
												continue;
											}
										}
									}
									else if (temp.sym == "Diff"){
										flag = 0;
										if (a.size() != 3){
											err_msg.push_back(make_pair(temp.pos,"Error : Incorrect argument quantity."));
											a.push_back(make_symb("null",7,-1,-1,0));
											flag = 1;
										}
										if (a[0].typ != 5 ||
											a[1].typ == 6 || a[2].typ == 6 ||
											a[1].typ == 9 || a[2].typ == 9 || flag){
												err_msg.push_back(make_pair(temp.pos,"Error : Incorrect argument type."));
												a.push_back(make_symb("null",7,-1,-1,0));
												flag = 1;
											}
										if (!flag){
											if (diff_state.top() == 1){
												vars[vars.size() - 1][a[0].sym].val = a[1].val;
												diff_state.pop();
												diff_state.push(2);
												i = loop_pos.top();
												continue;
											}
											else if (diff_state.top() == 2){
												diff_result.push(a[2].val);
												vars[vars.size() - 1][a[0].sym].val += delt;
												diff_state.pop();
												diff_state.push(3);
												i = loop_pos.top();
												continue;
											}
											else if (diff_state.top() == 3){
												ta = diff_result.top();
												ta = (a[2].val - ta) / delt;
												a.clear();
												a.push_back(make_symb("$number",0,-1,-1,ta));
												diff_result.pop();
												diff_state.pop();
												loop_pos.pop();
											}
										}
									}
									else if (temp.sym == "Print"){
										for(int j = 0;j < (int)a.size();j++)
											if (a[j].typ == 6 || a[j].typ == 9)
												logger << a[j].dat;
											else
												logger << a[j].val;
									}
									else if (temp.sym == "Input"){
										for(int j = 0;j < (int)a.size();j++){
											if (a[j].typ != 9 && a[j].typ != 5)
												err_msg.push_back(make_pair(temp.pos,"Error : Mismatch argument type."));
											else{
												getline(logger, sa);
												ia = 0;
												if (is_number(sa[0])){
													vars[vars.size() - 1][a[j].sym].val = get_number(sa,ia);
													vars[vars.size() - 1][a[j].sym].typ = 5;
												}
												else if (sa.size() >= 2 && (is_number(sa[1]) && (sa[0] == '-' || sa[0] == '+'))){
													ia++;
													vars[vars.size() - 1][a[j].sym].val = get_number(sa,ia) * (sa[0] == '-' ? -1.0_r : 1.0_r);
													vars[vars.size() - 1][a[j].sym].typ = 5;
												}
												else{
													vars[vars.size() - 1][a[j].sym].dat = sa;
													vars[vars.size() - 1][a[j].sym].typ = 9;
												}
											}
										}
									}
									else if (temp.sym == "Length"){
										tv.clear();
										for(int j = 0;j < (int)a.size();j++){
											if (a[j].typ == 9 || a[j].typ == 6)
												tv.push_back(a[j].dat.size());
											else
												err_msg.push_back(make_pair(temp.pos,"Error : Mismatch argument type."));
                                        }
										a.clear();
										for(int j = 0;j < (int)tv.size();j++){
											a.push_back(make_symb("$number",0,-1,-1,tv[j]));
                                        }
									}
									else if (temp.sym == "Erase"){
										flag = 0;
										if (a.size() != 3){
											flag = 1;
											err_msg.push_back(make_pair(temp.pos,"Error : Incorrect argument quantity."));
											a.push_back(make_symb("null",7,-1,-1,0));
										}
										if (flag || !(a[0].typ == 0 || a[0].typ == 5) ||
												    !(a[1].typ == 0 || a[1].typ == 5) ||
													!(a[2].typ == 6 || a[2].typ == 9)){
											flag = 1;
											err_msg.push_back(make_pair(temp.pos,"Error : Mismatch argument type."));
											a.push_back(make_symb("null",7,-1,-1,0));
										}
										if (!flag){
											sa = a[2].dat;
											ia = int(a[0].val);
											ib = int(a[1].val);
											a.clear();
											if (ia >= 0 && ib >= 0 && ia + ib <= (int)sa.size()){
												sa.erase(sa.begin() + ia,sa.begin() + ia + ib);
												a.push_back(make_symb(sa,6,-1,-1,0));
											}else{
												err_msg.push_back(make_pair(temp.pos,"Error : Out of range."));
												a.push_back(make_symb("null",7,-1,-1,0));
											}
										}
									}
									else if (temp.sym == "Type"){
										for(int j = 0;j < (int)a.size();j++){
											a[j].val = a[j].typ;
											a[j].sym = "$number";
											a[j].typ = 0;
										}
									}
									else{
										for(int j = 0;j < (int)a.size();j++){
											if (!a[j].typ || a[j].typ == 4 || a[j].typ == 5){
												ta = a[j].val;
												if (temp.sym == "Sin")
													ta = sin(ta);
												else if (temp.sym == "Cos")
													ta = cos(ta);
												else if (temp.sym == "Tan")
													ta = tan(ta);
												else if (temp.sym == "ASin")
													ta = asin(ta);
												else if (temp.sym == "ACos")
													ta = acos(ta);
												else if (temp.sym == "ATan")
													ta = atan(ta);
												else if (temp.sym == "Ln")
													ta = log(ta);
												else if (temp.sym == "Log")
													ta = log(ta)/log(10.0_r);
												else if (temp.sym == "Exp")
													ta = exp(ta);
												else if (temp.sym == "Sqrt")
													ta = sqrt(ta);
												else if (temp.sym == "Sgn")
													ta = ta >= 0 ? 1 : -1;
												else if (temp.sym == "Abs")
													ta = abs(ta);
												else if (temp.sym == "Sec")
													ta = 1.0_r/cos(ta);
												else if (temp.sym == "Csc")
													ta = 1.0_r/sin(ta);
												else if (temp.sym == "Cot")
													ta = 1.0_r/tan(ta);
												else if (temp.sym == "Sh")
													ta = (exp(ta)+exp(-ta))/2.0_r;
												else if (temp.sym == "Ch")
													ta = (exp(ta)-exp(-ta))/2.0_r;
												else if (temp.sym == "Th")
													ta = (exp(ta)+exp(-ta))/(exp(ta)-exp(-ta));
												else if (temp.sym == "Int")
													ta = (int) ta;
												else if (temp.sym == "Floor")
													ta = floor(ta);
                                                else if (temp.sym == "Frac")
													ta = frac(ta);
												else if (temp.sym == "Ceiling")
													ta = ceil(ta);
												else err_msg.push_back(make_pair(temp.pos,temp.sym + " : Unknow symbol."));
												a[j].val = ta;
												a[j].typ = 0;
												a[j].sym = "$number";
											}else if (a[j].typ == 6 || a[j].typ == 9){
												sa = a[j].dat;
												if (temp.sym == "StringReverse"){
													reverse(sa.begin(),sa.end());
												}else err_msg.push_back(make_pair(temp.pos,temp.sym + " : Unknow symbol."));
												a[j].dat = sa;
												a[j].sym = "$string";
												a[j].typ = 6;
											}
										}
									}
									val_stack.push(a);
								}
							}
							if (tokens[i].sym == ")"){
								if (opr_stack.empty()){
									err_msg.push_back(make_pair(-1,"Error : Operator stack error."));
									continue;
								}
								if (opr_stack.top().sym == "("){
									opr_stack.pop();
									continue;
								}
							}
							opr_stack.push(tokens[i]);
						}
					}//operators and functions and ()
				}
				
				if (opr_stack.size() != 1)
					err_msg.push_back(make_pair(-1,"Error : Operator stack error."));
				if (val_stack.size() != 1)
					err_msg.push_back(make_pair(-1,"Error : Number stack error."));
				if (!val_stack.empty())
					res = val_stack.top();
				else
					res.push_back(make_symb("null",7,-1,-1,0));
					
			}
		
        Interpreter(OutputStream & _logger):logger(_logger){
            	map <string,SYMBOL> temp;
				init_buildin_symbols();
				vars.push_back(temp);
				test_mode = 0;
				err_strict = 0;
				prec = (1e-12_r);
				max_solve_loop = 1000;
				max_integrate_loop = 1000;
				delt = 1e-8_r;
				rnd.init();
        }

        Interpreter(Interpreter & other) = delete;
        Interpreter(Interpreter && other) = delete;

		void
			calc_formula(string expr){
				vector <SYMBOL>
					a;
				vector <SYMBOL>
					t;
				lexer(a,expr);
				executer(a,t);
				print_list(t);
				logger << endl;
				if (err_msg.size())
					print_err();
			}
		
		void
			test_mode_on(){
				test_mode = 1;
			}
		
		void
			test_mode_off(){
				test_mode = 0;
			}
		
		void
			err_strict_on(){
				err_strict = 1;
			}
			
		void
			err_strict_off(){
				err_strict = 0;
			}
			

		~Interpreter(){
				registered_symbols.clear();
				usr_func.clear();
				err_msg.clear();
				vars.clear();
			}
	};
		
static void calc_tb(OutputStream & logger){
    logger.println("calc entry");
    Interpreter intp{logger};
    string inpstr;
    bool flag = 0;
    logger << "TinyCalc kernel [1.0.4] By Cansanel\n\n";


    while(1){
        if (!flag)
            logger << ">>> ";
        else
            flag = 0;
        
        getline(logger, inpstr);
        logger << "you input:" << inpstr << endl;

        if (inpstr == "exit")
            break;
        intp.calc_formula(inpstr);
    }

}

void calc_main(){
    // auto & logger = uart2;
    uart2.init(576000, CommStrategy::Blocking);
	DEBUGGER.retarget(&uart2);
    DEBUGGER.set_eps(4);
    calc_tb(DEBUGGER);
    while(true);
}