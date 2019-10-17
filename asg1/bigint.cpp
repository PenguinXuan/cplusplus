// $Id: bigint.cpp,v 1.78 2019-04-03 16:44:33-07 - - $
//By: Zhuoxuan Wang (zwang437@ucsc.edu)
//and Xiong Lou (xlou2@ucsc.edu)

#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
using namespace std;

#include "bigint.h"
#include "debug.h"
#include "relops.h"

bigint::bigint (long that): uvalue (that), is_negative (that < 0) {
   DEBUGF ('~', this << " -> " << uvalue)
}

bigint::bigint (const ubigint& uvalue, bool is_negative):
                uvalue(uvalue), is_negative(is_negative) {
}

bigint::bigint (const string& that) {
   is_negative = that.size() > 0 and that[0] == '_';
   uvalue = ubigint (that.substr (is_negative ? 1 : 0));
}

bigint bigint::operator+ () const {
   return *this;
}

bigint bigint::operator- () const {
   return {uvalue, not is_negative};
}

bigint bigint::operator+ (const bigint& that) const {
   if (is_negative == that.is_negative) {
      return {uvalue + that.uvalue, is_negative};
   }
   if (uvalue == that.uvalue) {
      return {ubigint(0), false};
   } else if (uvalue < that.uvalue) {
      return {that.uvalue - uvalue, that.is_negative};
   } else {
      return {uvalue - that.uvalue, is_negative};
   }
}

bigint bigint::operator- (const bigint& that) const {
   if (is_negative == that.is_negative) {
       return {uvalue + that.uvalue, is_negative};
   }
   if (is_negative == that.is_negative) {
      if (uvalue == that.uvalue) {
         return {ubigint(0), false};
      } else if (uvalue < that.uvalue) {
         return {that.uvalue - uvalue, not is_negative};
      } else {
          return {uvalue - that.uvalue, is_negative};
      }
   }
}


bigint bigint::operator* (const bigint& that) const {
    if (is_negative == that.is_negative) {
        return {uvalue * that.uvalue, false};
    }
    return {uvalue * that.uvalue, true};
}

bigint bigint::operator/ (const bigint& that) const {
   if (is_negative == that.is_negative) {
       return {uvalue / that.uvalue, false};
   }
   return {uvalue / that.uvalue, true};
}

bigint bigint::operator% (const bigint& that) const {
   if (is_negative == that.is_negative) {
      return {uvalue % that.uvalue, false};
   }
   return {uvalue % that.uvalue, true};
}

bool bigint::operator== (const bigint& that) const {
   if (is_negative == that.is_negative){
      return uvalue == that.uvalue;
   }else
      return false;

}

bool bigint::operator< (const bigint& that) const {
    if (is_negative != that.is_negative) {
      return is_negative;
    }
    if (is_negative) {
        return !(uvalue < that.uvalue);
    } else {
        return uvalue < that.uvalue;
    }
}

ostream& operator<< (ostream& out, const bigint& that) {
   return out << (that.is_negative ? "-" : "") << that.uvalue;
}

