/* Authors: Gavin Baker
 *          Paul Sullivan
 *
 * Email:   gabaker@calpoly.edu
 *          paulsulli92@gmail.com
 */

// c/c++ libraries
#include<iostream>   // std::cout
#include<fstream>    // std::ifstream

//libraries required for gmp
#include<stdio.h>
#include<stdarg.h>
#include<obstack.h>
#include<gmp.h>

//multithreading
#include<omp.h>

//#include "primes.h"

/*mpz_t gcd(mpz_t a, mpz_t b) {

   while (a != b) {
      if (a > b)
         a=a-b;
      else
         b=b-a;
   }

   return a;
}
*/

int main(int argc, char *argv[]) {
   mpz_t a, b, aTemp, bTemp;

   mpz_init(a);
   mpz_init(aTemp);
   mpz_init(b);
   mpz_init(bTemp);

   mpz_set_si(a, 15);
   mpz_set_si(aTemp, 15);
   mpz_set_si(b, 10);
   mpz_set_si(bTemp, 10);

   //std::cout << a << b << aTemp << bTemp;

   //std::cout << "gcd of " << a << " and " << b << " is " << gcd(a,b) << std::endl;

   mpz_clear(a);
   mpz_clear(aTemp);
   mpz_clear(b);
   mpz_clear(bTemp);

   return 0;
}
