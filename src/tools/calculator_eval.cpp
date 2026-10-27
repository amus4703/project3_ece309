#include "tools/calculator.h"
#include <stack>
namespace {
using Kind=CalcToken::Kind;
int precedence(Kind k){
 if(k==Kind::Times||k==Kind::Divide)return 2;
 if(k==Kind::Plus||k==Kind::Minus)return 1;
 return 0;
}
bool apply(std::stack<double>& values,std::stack<Kind>& ops,std::string* error){
 if(ops.empty()||values.size()<2){*error="invalid expression stack";return false;}
 const Kind op=ops.top();ops.pop();
 const double right=values.top();values.pop();
 const double left=values.top();values.pop();
 double result=0;
 switch(op){
 case Kind::Plus:result=left+right;break;
 case Kind::Minus:result=left-right;break;
 case Kind::Times:result=left*right;break;
 case Kind::Divide:
  if(right==0){*error="division by zero";return false;}
  result=left/right;break;
 default:*error="invalid operator";return false;
 }
 values.push(result);return true;
}
}
bool calc_evaluate(const std::vector<CalcToken>& tokens,double* out,std::string* error){
 std::stack<double> values;
 std::stack<Kind> ops;
 for(const auto& token:tokens){
  const Kind k=token.kind;
  if(k==Kind::Number){values.push(token.value);continue;}
  if(k==Kind::LParen){ops.push(k);continue;}
  if(k==Kind::RParen){
   while(!ops.empty()&&ops.top()!=Kind::LParen)if(!apply(values,ops,error))return false;
   if(ops.empty()){*error="unmatched closing parenthesis";return false;}
   ops.pop();continue;
  }
  if(precedence(k)==0){*error="unknown token";return false;}
  while(!ops.empty()&&ops.top()!=Kind::LParen&&precedence(ops.top())>=precedence(k))
   if(!apply(values,ops,error))return false;
  ops.push(k);
 }
 while(!ops.empty()){
  if(ops.top()==Kind::LParen){*error="unmatched opening parenthesis";return false;}
  if(!apply(values,ops,error))return false;
 }
 if(values.size()!=1){*error="invalid expression stack";return false;}
 *out=values.top();error->clear();return true;
}
