//
// Created by rafy on 4/25/24.
//

#ifndef MISC_XML_REAL_H
#define MISC_XML_REAL_H
#define fixed (1<<14)

    typedef struct {
        int val ;
    }real;
    void init_real(real *,int);
    int convert_int_to_real(int); // to convert from normal int to 17.14 base
    int convert_real_to_int(int); // truncation towards zero
    int convert_real_to_int_with_rounding(int); // to round real to int rouned
    int add_x_and_y(int,int);// to add two real numbers
    int subtract_y_from_x(int,int);//to sub two real numbers
    int add_m_and_n(int,int);
    int subtract_n_from_m(int,int);
    int multiply_x_by_y(int,int);
    int multiply_m_by_n(int,int);
    int divide_x_by_y(int,int);
    int divide_m_by_n(int,int);

#endif //MISC_XML_REAL_H
