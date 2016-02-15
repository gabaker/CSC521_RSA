/* Authors: Gavin Baker
 *          Paul Sullivan
 *
 * Email:   gabaker@calpoly.edu
 *          paulsulli92@gmail.com
 */

// c/c++ libraries
#include<iostream>   // std::cout
#include<fstream>    // std::ifstream
#include<vector>
#include<sys/time.h>
#include<time.h>
#include<sstream>


//libraries required for gmp
#include<stdio.h>
#include<stdarg.h>
#include<obstack.h>
#include<gmp.h>
#include<gmpxx.h> //this header requires library compilation with the flag --enable-cxx

//multithreading
#include<omp.h>

void basic_gcd(mpz_class &a, mpz_class &b, mpz_class &c); 
void print_str_2d_vector(std::vector<std::vector<std::string> > &str_vector, bool formatOutput); 
int read_key_file(std::ifstream &key_file, std::vector<std::string> &key_vector);
void print_str_2d_vector(std::vector<std::vector<std::string> > &str_vector, bool formatOutput);
void str_to_gmp_1d_vector(std::vector<std::string> &str_vector, std::vector<mpz_class> &gmp_vector); 
void print_str_1d_vector(std::vector<std::string> &str_vector);
void gmp_to_str_2d_vector(std::vector<std::vector<std::string> > &str_vector, std::vector<std::vector<mpz_class> > &gmp_vector);
void gmp_to_str_1d_vector(std::vector<std::string> &str_vector, std::vector<mpz_class> &gmp_vector);
void batch_gcd(std::vector<mpz_class> &gmp_pub_keys, std::vector<std::vector<mpz_class> > &primes); 
void basic_batch_gcd(std::vector<mpz_class> &gmp_pub_keys, std::vector<std::vector<mpz_class> > &primes); 
void basic_gcd(mpz_class &a, mpz_class &b, mpz_class &c);


#define MICRO_TO_BASE (1 / 100000.0)
int BASE_USED = 16;


int main(int argc, char *argv[]) {
   std::vector<std::string> str_keys;
   std::vector<mpz_class> gmp_keys;
   std::vector<std::vector<mpz_class> > gmp_co_primes;
   std::vector<std::vector<std::string> > str_co_primes;
   int num_keys = 0;

   if (argc == 3 ) {
      BASE_USED = std::stoi(argv[2]);
      std::cout << "Using Base: " << BASE_USED << std::endl; 
   }

   //check for user provided keylist
   if (argc < 2 || argc > 3) {
      std::cout << "Usage: " << argv[0] << " file_name" << std::endl;
      exit(-1);
   }

   std::ifstream pub_key_file(argv[1]);
   
   //open file, check if open successful, read file into string vector
   //convert string vector to gmp variable precision integer
   if (pub_key_file.fail()) {
      std::cout << "Bad input file name...exiting." << std::endl;
      exit(-1);
   } else {
      num_keys = read_key_file(pub_key_file, str_keys);
      std::cout << "Running common factor test on " << num_keys << " keys:" << std::endl;
      print_str_1d_vector(str_keys);
      str_to_gmp_1d_vector(str_keys, gmp_keys);
   }

   // Find common primes; time execution duration
   struct timeval stop_t, start_t, total_t;
   
   gettimeofday(&start_t, NULL);
   #ifndef TREE_GCD
   basic_batch_gcd(gmp_keys, gmp_co_primes);
   #else
   batch_gcd(gmp_keys, gmp_co_primes);
   #endif
   gettimeofday(&stop_t, NULL);
   timersub(&stop_t, &start_t, &total_t);

   float total_time = ((float) total_t.tv_sec) + ((float) total_t.tv_usec * MICRO_TO_BASE);

   std::cout << "Total GCD calc time (seconds):  " << total_time << std::endl;

   gmp_to_str_2d_vector(str_co_primes, gmp_co_primes);

   if (str_co_primes.size() > 0) {
      print_str_2d_vector(str_co_primes, true);
   } else {
      std::cout << "\nNo shared common factors found\n" << std::endl; 
   }
   //clean up the memory allocated by the gmp init functions for each objecy in array
   str_co_primes.clear();
   gmp_co_primes.clear();
   gmp_keys.clear();
   str_keys.clear();

   return 0;
}

// print out a 2D vector of std::string objects
// formatted as if it is a common factor/key vector
void print_str_2d_vector(std::vector<std::vector<std::string> > &str_vector, bool formatOutput) {
   for (auto iter_i = str_vector.begin(); iter_i != str_vector.end(); iter_i++) {
      bool is_factor = true;

      for (auto iter_j = (*iter_i).begin(); iter_j != (*iter_i).end(); iter_j++) {
         if (formatOutput && is_factor) {
            std::cout << "\nCommon Factor:" << *iter_j << std::endl;
            std::cout << "Keys Sharing Factor: " << std::endl;
            is_factor = false;
         } else {
            std::cout << *iter_j << std::endl;
         }
      }
   }
}

// product/ remainder tree method as demonstrated on
// http://facthacks.cr.yp.to/batchgcd.html
void batch_gcd(std::vector<mpz_class> &gmp_pub_keys, std::vector<std::vector<mpz_class> > &primes) {
   //std::vector<mpz_class> factors();
   //factoresize(gmp_pub_keys.size());
   mpz_class key_idx;
   mpz_class product = gmp_pub_keys[0];
   mpz_class gcd_result = 0;
   //calculate the product (tree)
   for (auto iter = gmp_pub_keys.begin() + 1; iter != gmp_pub_keys.end(); iter++) {
      product *= *iter;
   }

   std::cout << product.get_str(10) << std::endl;

   for (int i = 0; i < gmp_pub_keys.size(); i++) {
      std::vector<mpz_class> temp_vec;
      gcd_result = 0;
      key_idx = (product % (gmp_pub_keys[i] * gmp_pub_keys[i])) / gmp_pub_keys[i];
      mpz_gcd(gcd_result.get_mpz_t(), key_idx.get_mpz_t(), gmp_pub_keys[i].get_mpz_t());
      if (gcd_result > 1) {
         temp_vec.push_back(gcd_result);
         temp_vec.push_back(gmp_pub_keys[i]);
         primes.push_back(temp_vec);
      }
   }
   
   /*for (auto iter = factors.begin(); iter != factors.begin(); iter++) {
      if (*iter > 1) {
         //std::vector<mpz_class> temp_vec;
         primes.push_back(std::vector<mpz_class>(*iter));
      }
   }*/

}

// print out a vector of strings, one string per line
void print_str_1d_vector(std::vector<std::string> &str_vector) {
   for (auto iter = str_vector.begin(); iter != str_vector.end(); iter++) 
      std::cout << *iter << std::endl;
}

// convert the key vector into an array of mpz_class objects
void str_to_gmp_1d_vector(std::vector<std::string> &str_vector, std::vector<mpz_class> &gmp_vector) {
   for (auto iter = str_vector.begin(); iter < str_vector.end(); iter++) 
      gmp_vector.push_back(mpz_class(*iter, BASE_USED));
}

//convert 2D vector of gmp values to 2D vector of std::string objects
void gmp_to_str_2d_vector(std::vector<std::vector<std::string> > &str_vector, std::vector<std::vector<mpz_class> > &gmp_vector) {
   
   for (auto iter_i = gmp_vector.begin(); iter_i != gmp_vector.end(); iter_i++) {
      std::vector<std::string> inner_vector;
      for (auto iter_j = (*iter_i).begin(); iter_j != (*iter_i).end(); iter_j++) {
         inner_vector.push_back((*iter_j).get_str(BASE_USED));
      }
      str_vector.push_back(inner_vector);
      inner_vector.clear();
   }
}

//convert 1D vector of gmp values to 1D vector of std::string objects
void gmp_to_str_1d_vector(std::vector<std::string> &str_vector, std::vector<mpz_class> &gmp_vector) {
   for (auto iter = gmp_vector.begin(); iter < gmp_vector.end(); iter++)
      str_vector.push_back((*iter).get_str(BASE_USED));
}

void basic_batch_gcd(std::vector<mpz_class> &gmp_pub_keys, std::vector<std::vector<mpz_class> > &primes) { 
   mpz_class c = 0;
   for (auto iter_i = gmp_pub_keys.begin(); iter_i != gmp_pub_keys.end(); iter_i++) {
      for (auto iter_j = iter_i + 1; iter_j != gmp_pub_keys.end(); iter_j++) {
         c = 0;
         std::vector<mpz_class> one_gcd;
         basic_gcd(*iter_i, *iter_j, c);
         
         if (c > 1) {  
            one_gcd.push_back(mpz_class(c));
            one_gcd.push_back(mpz_class(*iter_i));
            one_gcd.push_back(mpz_class(*iter_j));   
            primes.push_back(one_gcd);
         }
      }
   }
}

void basic_gcd(mpz_class &a, mpz_class &b, mpz_class &c) {
   mpz_class a_t = a, b_t = b;
   int cmp_result;
   while ((cmp_result = cmp(a_t, b_t)) != 0) {
      if (cmp_result > 0) 
         a_t -= b_t;
      else 
         b_t -= a_t; 
   }
   c = a_t;
}

// read RSA keys from file into a vector string for later conversion to mpz_class objects
int read_key_file(std::ifstream &key_file, std::vector<std::string> &key_vector) {
   std::string key_str;

   while (std::getline(key_file, key_str)) 
      key_vector.push_back(key_str);  

   return key_vector.size();
}

