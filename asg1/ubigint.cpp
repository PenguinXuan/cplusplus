// $Id: ubigint.cpp,v 1.14 2016-06-23 17:21:26-07 - - $

#include <cctype>
#include <algorithm>
#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
using namespace std;

#include "ubigint.h"
#include "debug.h"

/* Constructors */
//ubig_value = vector <unsigned char>

ubigint::ubigint (unsigned long that) {
   while (that > 0) {
      ubig_value.push_back(that % 10);
      that = that/10;
   }

   DEBUGF ('~', this << " -> " << that)
}

ubigint::ubigint (const string& that) {
   for (char digit : that) {
      ubig_value.push_back(digit - '0'); // a char -'0' makes the digit its correct number
   }

   reverse(ubig_value.begin(), ubig_value.end());
}

/************************************ Arithmetic Operations ***************************************/

// operator+
// this function adds two ubigint types 
ubigint ubigint::operator+ (const ubigint& that) const {
   ubigint addition(0); // addition will contain the addition of the values

   unsigned int vectorSize = (ubig_value.size() < that.ubig_value.size() ? ubig_value.size() : that.ubig_value.size());
   unsigned int i = 0; // i will be the index pointer for the vector
   int carry = 0;    // carry represents the carry number during a additionation, it will be reset to 0.
   int digitAddition = 0; // digitAddition represents the addition of two digits (eg, ubig_value.at(0) + that.ubig_value.at(0))

   if (ubig_value.size() == 0) { // If left == 0, then addition = right
      for (unsigned int i = 0; i < that.ubig_value.size(); i++) {
         addition.ubig_value.push_back(that.ubig_value.at(i)); // addition = right
      }
      return addition;
   }else if (that.ubig_value.size() == 0) { // If right == 0, then addition = left
      for (unsigned int i = 0; i < ubig_value.size(); i++) {
         addition.ubig_value.push_back(ubig_value.at(i)); // addition = left
      }
      return addition;
   }

   while (i < vectorSize or carry > 0) {
      if (i < ubig_value.size()) {
         digitAddition += ubig_value.at(i);
      }
      if (i < that.ubig_value.size()) {
         digitAddition += that.ubig_value.at(i);
      }
      digitAddition += carry; // this part adds the carry to the solution
      carry = 0; // carry gets set back to 0 after it's added to the result

      // check if we need to carry, the number is larger than 9.
      if (digitAddition > 9) {
         carry = 1; // carry gets set to one if the digit is larger than 9.
         digitAddition = digitAddition % 10; // doing the digit % 10 makes sure we get the correct result
      }                                      // after we consider the carry.

      addition.ubig_value.push_back(digitAddition);
      digitAddition = 0;
      i++;
   }

   // after the addition of the two vetors, one might be larger than the other, so
   // this part takes care of the extra digits when one operand is larger than the other
   if (ubig_value.size() > that.ubig_value.size()) { //left operand is larger in size than right operand
      while (i < ubig_value.size()) {
         addition.ubig_value.push_back(ubig_value.at(i)); // push back the extra digits back to the addition vector.
         i++;
      }
   } else { // right operand is larger in size than left operand
      while (i < that.ubig_value.size()) {
         addition.ubig_value.push_back(that.ubig_value.at(i)); // push back the extra digits back to the addition vector.
         i++;
      }
   }
   // this part of the code takes care of the extra 0s (eg. 00043 = 43).
   while (addition.ubig_value.size() > 0 and addition.ubig_value.back() == 0) { // delete the extra 0s (eg, 00043 = 43).
      addition.ubig_value.pop_back(); //pop_back() the extra 0s out of the vector
   }
   return addition;
}

// operator-
// this function subtracts two ubigint types 
ubigint ubigint::operator- (const ubigint& that) const {
   if (*this < that) {
      throw domain_error ("ubigint::operator-(a<b)");
   }
   
   ubigint subtraction(0);

   unsigned int i = 0;        // i will be the index pointer for the vector
   int borrowingNum = 0;      // The borrowingNum value represents the borrow variable of the subtraction
   int temp_ubig_value = 0;   // we need to have a temp value holder since we can't modify the ubig_value
                              // just in case we need to take away the borrowed number

   // this part of the code will deal with the borrow number
   while (i < that.ubig_value.size()) {
      temp_ubig_value = ubig_value.at(i) - borrowingNum; // remove the borrow number
      borrowingNum = 0;

      // Check if we need to borrow on the next iteration
      if (temp_ubig_value < that.ubig_value.at(i)) {
         temp_ubig_value += 10;
         borrowingNum = 1;
      }

      subtraction.ubig_value.push_back(temp_ubig_value - that.ubig_value.at(i));
      i++;
   }

   // after the addition of the two vetors, one might be larger than the other, so
   // this part takes care of the extra digits when one operand is larger than the other
   while (i < ubig_value.size()) {
      if (borrowingNum > 0) { // check if there is any more borrowed numbers
         subtraction.ubig_value.push_back(ubig_value.at(i)-1);
         borrowingNum = 0;
      } else {
         subtraction.ubig_value.push_back(ubig_value.at(i));
      }

      i++;
   }
   // this part of the code takes care of the extra 0s (eg. 00043 = 43).
   while (subtraction.ubig_value.size() > 0 and subtraction.ubig_value.back() == 0) { // delete the extra 0s (eg, 00043 = 43).
      subtraction.ubig_value.pop_back(); //pop_back() the extra 0s out of the vector
   }
   
   return subtraction;
}

// operator*
// this function multiplies two ubigint types 
ubigint ubigint::operator* (const ubigint& that) const {
   ubigint mult(0);

   for (unsigned int i = 0; i < ubig_value.size(); i++) { // loop to go through this vector
      ubigint tempMult(0); // tempt ubigint holder for the result before adding the carry
      int digitMult = 0; // holds the result of the mult operation in int
      int carry = 0; // carry the numbers when an int > 10
      for (unsigned int k = 0; k < that.ubig_value.size(); k++) { // loop to go through that vector
         digitMult = (ubig_value.at(i) * that.ubig_value.at(k)) + carry; // compute left*right + carry
         carry = 0;

         if (digitMult > 9) { // check if digitMult > 9, to add the carry to the next number
            carry = digitMult / 10; // if digitMult > 9, then carry = digitMult / 10.
            digitMult = digitMult % 10; 
         }

         tempMult.ubig_value.push_back(digitMult);
      }

      // the left over carry number gets pushed at the back of the vector
      if (carry > 0) {
         tempMult.ubig_value.push_back(carry); // push the carry number for the final result
         carry = 0;
      }

      // this part appends 0 to the base of the ubig_value when adding the results together 
      unsigned int offset = 0;
      while (offset < i) {
         tempMult.ubig_value.insert(tempMult.ubig_value.begin(), 0); // Appends 0s to base of ubig_value
         offset++;
      }

      mult = mult + tempMult;
      tempMult.ubig_value.clear();
   }
   
   // this part of the code takes care of the extra 0s (eg. 00043 = 43).
   while (mult.ubig_value.size() > 0 and mult.ubig_value.back() == 0) {
      mult.ubig_value.pop_back();
   }
   return mult;
}

// multiply_by_2()
// this function multiplies a ubigint by 2 (simpler version of mutiply from above) 
void ubigint::multiply_by_2() {
   int carry = 0;
   for (unsigned int i = 0; i < ubig_value.size(); i++) {
      int digitMult = (ubig_value.at(i) * 2) + carry;
      carry = 0;

      if (digitMult > 9) {           // check if digitMult > 9, to add the carry to the next number
         carry = digitMult / 10;     // if digitMult > 9, then carry = digitMult / 10.
         digitMult = digitMult % 10;
      }

      ubig_value.at(i) = digitMult; //ubig_value = digitMult
   }

   // the left over carry number gets pushed at the back of the vector
   if (carry > 0) {
      ubig_value.push_back(carry);
   }

   // this part of the code takes care of the extra 0s (eg. 00043 = 43).
   while (ubig_value.size() > 0 and ubig_value.back() == 0) {
      ubig_value.pop_back();
   }
}

// divide_by_2()
// this function divides a ubigint by 2 
void ubigint::divide_by_2() {
   int rem = 0; // variable to keep track of the remainder
   // we are going to start from the back of the vector, the last index i.
   for (int i = ubig_value.size()-1; i >= 0; i--) {  
      int divQuotient = 0;  // variable that contains the division_by_2                       
      if (rem > 0) {        // check if there is a rem to add to the quotient
         divQuotient += rem; // add the remainder to the quotient
         rem = 0;
      }
      divQuotient += ubig_value.at(i) / 2; // compute the value[i] division by 2
                                           // from the vector
      if (ubig_value.at(i) % 2) {          // check if we need a remainder
         rem = 5;                          // remainder can only be 0 or 5
      }
      ubig_value.at(i) = divQuotient;      // store the correct result back 
   }                                       // to the vector.value    

   // this part of the code takes care of the extra 0s (eg. 00043 = 43).
   while (ubig_value.size() > 0 and ubig_value.back() == 0) {
   	ubig_value.pop_back();
   }
}

// operatordivide
// this function divides two ubigint types 
ubigint::quot_rem ubigint::divide (const ubigint& that) const {
   static const ubigint zero(0);
   if (that == zero) throw domain_error ("ubigint::divide: by 0");
   ubigint power_of_2 = ubigint(1);
   ubigint divisor = that;    // right operand, divisor

   while (divisor < *this) {
      divisor.multiply_by_2();
      power_of_2.multiply_by_2();
   }
	
		ubigint quotient = ubigint(0);
    ubigint rem = *this; // left operand, dividend
    
    while (power_of_2 > zero) {
       if ((divisor < rem) or (divisor == rem)) {
         rem = rem - divisor;
         quotient = quotient + power_of_2;
      }
      divisor.divide_by_2();
      power_of_2.divide_by_2();
   }

   return {quotient, rem};
}

// operator/
// this function divides two ubigint types 
ubigint ubigint::operator/ (const ubigint& that) const {
   return divide(that).first;
}

// operator%
// this function divides two ubigint types 
ubigint ubigint::operator% (const ubigint& that) const {
   return divide(that).second;
}

// operator==
// this function defines if two ubigints are equal
bool ubigint::operator== (const ubigint& that) const {
   if (ubig_value.size() == that.ubig_value.size()) { // first check if they are at least the same size()
      for (unsigned int i = 0; i < ubig_value.size(); i++) { // loop to go through both vectors
         if (ubig_value.at(i) != that.ubig_value.at(i)) { // compare the values of this and that
            return 0; // if any of the values are different, return 0, false.
         }
      }
      return 1; // if the program walks though both vectors and there is no value different
                // then return 1. true.
   } else return 0; // if both ubigints have different size, then they are different
}

// operator<
// this function defines if this < that
bool ubigint::operator< (const ubigint& that) const {
   if (*this == that) {
   		return 0; // if this == that, return false
 	 }
   if (ubig_value.size() < that.ubig_value.size()) {
   		return 1; // if this.size() < that.size(), then we know that that is bigger and return true 
   }
   if (ubig_value.size() == that.ubig_value.size()) { // both this and that have the same size
      if (ubig_value.size() == 0) { // check if this.size() = 0, then just return 0
      	return 0;
      }
      for (int i = ubig_value.size()-1; i >= 0; i--) { // start comparing from the back of the vector
         if (ubig_value.at(i) > that.ubig_value.at(i)) { //compare if any of this > that
            return 0; // if this > that, then return false
         }
      }
      return 1; // if none of the values of this > that, then return true.
   }
   // if the size of this is bigger than the size of that, then return false
   return 0;
}

// operator<<
// this function prints to stream
ostream& operator<< (ostream& out, const ubigint& that) {
   int outLength = 70;
   if (that.ubig_value.size() > 0) {
      int charCount = 0;
      for (int i = that.ubig_value.size()-1; i >= 0; i--) {
         if (charCount%(outLength-1) == 0 and charCount != 0) out << "\\" << endl;
         out << static_cast<char>(that.ubig_value.at(i) + '0');
         charCount++;
      }
   } else {
      out << '0';
   }

   return out;
}