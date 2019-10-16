// $Id: ubigint.cpp,v 1.16 2019-04-02 16:28:42-07 - - $

#include <cctype>
#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
#include <iostream>

#include <math.h>
using namespace std;

#include "ubigint.h"
#include "debug.h"

ubigint::ubigint (unsigned long that): uvalue (that) {
    DEBUGF ('~', this << " -> " << uvalue)
    string s = to_string(uvalue);
    ubig_value = vector<udigit_t>();
    for (char digit: s) {
        if (not isdigit(digit)) {
            throw invalid_argument("ubigint::ubigint(" + s + ")");
        }
        uvalue = uvalue * 10 + digit - '0';
        ubig_value.push_back(digit - '0');
    }
}

ubigint::ubigint (const string& that): uvalue(0) {
   DEBUGF ('~', "that = \"" << that << "\"");
   for (char digit: that) {
      if (not isdigit (digit)) {
          throw invalid_argument ("ubigint::ubigint(" + that + ")");
      }
      uvalue = uvalue * 10 + digit - '0';
      ubig_value.push_back(digit - '0');

   }
}

ubigint::ubigint (vector<udigit_t> that): ubig_value(that) {

}

ubigint ubigint::operator+ (const ubigint& that) const {
    ubigint res(0);
    unsigned int min_length = 0;

    if (ubig_value.size() > that.ubig_value.size()) {
        min_length = that.ubig_value.size();

    } else if (ubig_value.size() < that.ubig_value.size()) {
        min_length = ubig_value.size();
    } else {
        min_length = ubig_value.size();
    }
    unsigned int i = 0;
    int carry = 0;

    while (i < min_length or carry > 0) {
        carry += ubig_value.at(i) + that.ubig_value.at(i);
        res.ubig_value.push_back(carry % 10);
        carry /= 10;
    }

    if (ubig_value.size() > that.ubig_value.size()) {
        while (i < ubig_value.size()) {
            res.ubig_value.push_back(ubig_value.at(i));
            i++;
        }
    } else {
        while (i < that.ubig_value.size()) {
            res.ubig_value.push_back(that.ubig_value.at(i));
            i++;
        }
    }
    while (res.ubig_value.size() > 0) {
        res.ubig_value.pop_back();
    }

   return res;
}

ubigint ubigint::operator- (const ubigint& that) const {
    if (*this < that) throw domain_error ("ubigint::operator-(a<b)");

    vector<udigit_t> result;
    int min_length = 0;
    unsigned int i = 0;
    int borrow = 0;
    int min_length = 0;
    if (ubig_value.size() > that.ubig_value.size()) {
        min_length = that.ubig_value.size();
    } else if (ubig_value.size() < that.ubig_value.size()) {
        min_length = ubig_value.size();
    } else {
        min_length = ubig_value.size();
    }

    while (i < min_length) {
       int curr = ubig_value.at(i) - borrow;
       if (curr < that.ubig_value.at(i)) {
           diff += 10;
           borrow = 1;
       }
       result.insert(,curr - that.ubig_value.at(i));
       i++;
    }

    while (i < ubig_value.size()) {
       if (borrow > 0) {
           res.ubig_value.push_back(ubig_value.at(i) - 1);
           borrow = 0;
       } else {
           res.ubig_value.push_back(ubig_value.at(i));
       }
       i++;
    }
    while (res.ubig_value.size() > 0) {
        res.ubig_value.pop_back();
    }
    return res;
}

ubigint ubigint::operator* (const ubigint& that) const {
    vector<udigit_t> res(that.ubig_value.size() + ubig_value.size(), 0);
    int index_n1 = 0;
    int index_n2 = 0;
    int sum = 0;
    for (int i = that.ubig_value.size() - 1; i >= 0; i--) {
        int carry = 0;
        index_n2 = 0;
        for (int j = ubig_value.size() - 1; j >= 0; j--) {
            sum = (ubig_value.at(j) * that.ubig_value.at(i)) + res.at(index_n1 + index_n2) + carry;
            carry = sum / 10;
            res.at(index_n1 + index_n2) = sum % 10;
            index_n2++;
        }
        if (carry > 0)
            res.at(index_n1 + index_n2) += carry;
        index_n1++;
    }

    int i = res.size() - 1;
    while (i >= 0 && res.at(i) == 0) {
        i--;
    }
    return ubigint(res);
}

void ubigint::multiply_by_2() {
    vector<udigit_t> result;
    int sum = 0;
    int carry = 0;
    for (int i = ubig_value.size() - 1; i >= 0; i--) {
        sum = (ubig_value.at(i) * 2) + carry;
        carry = sum / 10;
        result.insert(result.begin(), 1, sum % 10);
    }

    if (carry > 0) {
        result.insert(result.begin(), 1, carry);
    }
    ubig_value = result;
}

void ubigint::divide_by_2() {
    vector<udigit_t> result;
    unsigned int i = 0;
    int temp = ubig_value.at(i);
    while (ubig_value.at(i) < 2) {
        temp = temp * 10 + ubig_value.at(++i);
    }

    while (ubig_value.size() > i) {
        result.insert(result.end(), 1, temp / 2);
        temp = (temp % 2) * 10 + ubig_value.at(++i);
    }
    ubig_value = result;
}

struct quo_rem { ubigint quotient; ubigint remainder; };
quo_rem udivide (const ubigint& dividend, const ubigint& divisor_) {
   // NOTE: udivide is a non-member function.
   ubigint divisor {divisor_};
   ubigint zero {0};
   if (divisor == zero) throw domain_error ("udivide by zero");
   ubigint power_of_2 {1};
   ubigint quotient {0};
   ubigint remainder {dividend}; // left operand, dividend
   while (divisor < remainder) {
      divisor.multiply_by_2();
      power_of_2.multiply_by_2();
   }
   while (power_of_2 > zero) {
      if (divisor <= remainder) {
         remainder = remainder - divisor;
         quotient = quotient + power_of_2;
      }
      divisor.divide_by_2();
      power_of_2.divide_by_2();
   }
   return {.quotient = quotient, .remainder = remainder};
}

ubigint ubigint::operator/ (const ubigint& that) const {
    return udivide (*this, that).quotient;
}

ubigint ubigint::operator% (const ubigint& that) const {
   return udivide (*this, that).remainder;
}

bool ubigint::operator== (const ubigint& that) const {
    if (that.ubig_value.size() != ubig_value.size()) {
        return false;
    }
    if (that.ubig_value.size() == ubig_value.size()) {
        for (unsigned int i = 0; i < ubig_value.size(); i++) {
            if (that.ubig_value.at(i) != ubig_value.at(i)) {
                return false;
            }
        }
    }
    return true;
}

bool ubigint::operator< (const ubigint& that) const {
    if (ubig_value.size() > that.ubig_value.size()) {
        return false;
    } else if (ubig_value.size() < that.ubig_value.size()) {
        return true;
    } else {
        for (unsigned int i = 0; i < ubig_value.size(); i++) {
            if (ubig_value.at(i) > that.ubig_value.at(i)) {

            }

        }
    }
   return uvalue < that.uvalue;
}


ostream& operator<< (ostream& out, const ubigint& that) {
    string output;
    for(unsigned int i = 0; i < that.ubig_value.size(); i++)
    {
        output.append(1, static_cast <char> (that.ubig_value.at(i) + '0'));
    }
    return out << output;
}
