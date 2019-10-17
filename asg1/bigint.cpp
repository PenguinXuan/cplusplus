// $Id: bigint.cpp,v 1.76 2016-06-14 16:34:24-07 - - $

#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
using namespace std;

#include "bigint.h"
#include "debug.h"
#include "relops.h"

bigint::bigint (long that): uvalue (that), is_negative (that < 0) {
   DEBUGF ('~', this << " -> " << uvalue) //this constructor remains the same
}

bigint::bigint (const ubigint& uvalue, bool is_negative):
                uvalue(uvalue), is_negative(is_negative) {
} //this constructor remains the same

bigint::bigint (const string& that) {
   is_negative = that.size() > 0 and that[0] == '_';
   uvalue = ubigint (that.substr (is_negative ? 1 : 0));
} //this constructor remains the same

/* Arithmetic Operations */

// operator+
// this function adds two bigint objects 
bigint bigint::operator+() const {
   return *this;
}

// operator-
// this function subtracts two bigint objects 
bigint bigint::operator-() const {
   return {uvalue, not is_negative};
}

// operator+
// this function adds two bigint objects 
bigint bigint::operator+ (const bigint& that) const {
   // if signs are the same
   if (is_negative == that.is_negative) {
      return {uvalue + that.uvalue, is_negative};
   }   
   // if signs are different
   if (uvalue == that.uvalue) {// if left = right, just return 0;
      return {ubigint(0), false};
   }
   if (uvalue < that.uvalue) { // if right > left 
      return {that.uvalue - uvalue, that.is_negative};
   } else { // if left  >= right 
      return {uvalue - that.uvalue, is_negative};
   }
}

// operator-
// this function subtracts two bigint objects 
bigint bigint::operator- (const bigint& that) const {
   // if signs are the same
   if (is_negative == that.is_negative) {
      if (uvalue == that.uvalue) { // if left = right
         return {ubigint(0), false}; // return 0 
      }
      if (uvalue < that.uvalue) { // if right > left
         return {that.uvalue - uvalue, not is_negative}; // right - left 
      }
      return {uvalue - that.uvalue, is_negative}; // if left > right, then left - right
   }
   // if signs are different
   return {uvalue + that.uvalue, is_negative}; // we can just add them
}

// operator*
// this function multiplies two bigint objects
bigint bigint::operator* (const bigint& that) const {
   if (is_negative == that.is_negative) { // if signs are the same
      return {uvalue * that.uvalue, false}; // uvalue * that.uvalue, is_negative = false
   }
   return {uvalue * that.uvalue, true}; //if signs are different, is_negative = true
}                                       // because a negative# * positive# = negative#

// operator/
// this function divides two bigint objects
bigint bigint::operator/ (const bigint& that) const {
   if (is_negative == that.is_negative) { // if signs are the same
      return {uvalue / that.uvalue, false}; // uvalue / that.uvalue, is_negative = false
   }
   return {uvalue / that.uvalue, true}; //if signs are different, is_negative = true
}                                       // because a negative# / positive# = negative# 

// operator%
// this function find the modulus of two bigint objects
bigint bigint::operator% (const bigint& that) const {
   if (is_negative == that.is_negative) { // if signs are the same
      return {uvalue % that.uvalue, false}; // uvalue % that.uvalue, is_negative = false
   }
   return {uvalue % that.uvalue, true}; //if signs are different, is_negative = true
}                                       // because a negative# % positive# = negative# 

// operator==
// this function finds if two bigint objects are equal
bool bigint::operator== (const bigint& that) const {
   if (is_negative == that.is_negative){ // if signs are the same
      return uvalue == that.uvalue; // check if left == right, if its true, return true.
   }else { //if signs are different, return false because -2 and 2 are two different numbers.
      return false;
   }
}

// operator<
// this function finds if the right bigint is bigger than the left bigint
bool bigint::operator< (const bigint& that) const {
   if (is_negative != that.is_negative) { // e.g. if left side is positive and right side is negative
      return is_negative; // return false if left is positive, or true othersiwe. e.g. -3 < 2 true
   }
   if (is_negative) { // if left side and right side have the same signs (negative sign)
      return !(uvalue < that.uvalue) and !(uvalue == that.uvalue); 
   }
   return uvalue < that.uvalue;  // both values are positive 
}

// operator<<
// this function prints out the results
ostream& operator<< (ostream& out, const bigint& that) {
   return out << (that.is_negative ? "-" : "") << that.uvalue;
}