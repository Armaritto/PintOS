//
// Created by rafy on 4/25/24.
//

#include "real.h"

void init_real(real* r , int n){
    r->val = convert_int_to_real(n);
}
int convert_int_to_real(int n){
    return n*fixed;
}

int convert_real_to_int(int x){
    return x / fixed ; //round down to zero
}
int convert_real_to_int_with_rounding(int x){
    if(x >= 0)
        return (x + fixed/2) / fixed ; // if +ve round up
    return (x - fixed/2) / fixed ; // -ve round down
}
int add_x_and_y(int x , int y){
    return x + y ;
}
int subtract_y_from_x(int x , int y) {
    return x - y ;
}
int add_m_and_n(int m,int n){
    return convert_int_to_real(m) + convert_int_to_real(n) ;
}
int subtract_n_from_m(int m,int n){
    return convert_int_to_real(m) + convert_int_to_real(n) ;
}
int multiply_x_by_y(int x , int y){
    return (int32_t)(((int64_t)x * y) / fixed);
}
int multiply_m_by_n(int m,int n){
    m = convert_int_to_real(m) ;
    n = convert_int_to_real(n) ;
    return multiply_x_by_y(n,m);
}
int divide_x_by_y(int x , int y) {
    return (int32_t)(((int64_t)x * fixed) / y);
}
int divide_m_by_n(int m , int n){
    m = convert_int_to_real(m) ;
    n = convert_int_to_real(n) ;
    return divide_x_by_y( m,n);
}
