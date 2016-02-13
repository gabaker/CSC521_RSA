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

//libraries required for gmp
#include<stdio.h>
#include<stdarg.h>
#include<obstack.h>
#include<gmp.h>
#include<gmpxx.h> //this header requires library compilation with the flag --enable-cxx

//multithreading
#include<omp.h>

void sample_gcd(mpz_t a, mpz_t b, mpz_t c);
int read_key_file(std::ifstream &key_file, std::vector<std::string> &key_list);
void print_key_list(std::vector<std::string> &key_list); 
void str_to_gmp_list(std::vector<std::string> &str_pub_keys, mpz_t gmp_pub_keys[]);
void find_common_primes(mpz_t gmp_pub_keys[], int num_keys);
void clean_gmp_list(mpz_t gmp_pub_keys[], int num_keys);

int main(int argc, char *argv[]) {
   std::vector<std::string> str_pub_keys;
   mpz_t *gmp_pub_keys;

   int num_keys = 0;

   //check for user provided keylist
   if (argc != 2) {
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
      num_keys = read_key_file(pub_key_file, str_pub_keys);
      std::cout << "Running common factor test on " << num_keys << " keys:" << std::endl;
      print_key_list(str_pub_keys);
      gmp_pub_keys = new mpz_t[num_keys]; 
      str_to_gmp_list(str_pub_keys, gmp_pub_keys);
   }

   //use common prime algorithm
   //TODO: Change this algorithm to product/remainder tree calc
   find_common_primes(gmp_pub_keys, num_keys);

   //clean up the memory allocated by the gmp init functions for each objecy in array
   //clean_gmp_list(gmp_pub_keys, num_keys);

   return 0;
}

void find_common_primes(mpz_t gmp_pub_keys[], int num_keys) {
   mpz_t a, b, c;
   mpz_init(a);
   mpz_init(b);
   mpz_init(c);

   for (int i = 0; i < num_keys; i++) {
      for (int j = i + 1; j < num_keys; j++) {
         mpz_set(a, gmp_pub_keys[i]);
         mpz_set(b, gmp_pub_keys[j]);
         mpz_set_si(c, 0);
         sample_gcd(a, b, c);

         char *a_str = mpz_get_str(NULL, 10, a);
         char *b_str = mpz_get_str(NULL, 10, b);
         char *c_str = mpz_get_str(NULL, 10, c);
         std:: cout << "gcd("<< i << "," << j << ")=" << c_str << std::endl;
         free(a_str);
         free(b_str);
         free(c_str);
      }
   }

   mpz_clear(a);
   mpz_clear(b);
   mpz_clear(c);
}

void sample_gcd(mpz_t a, mpz_t b, mpz_t c) {
   int cmp_result = mpz_cmp(a,b);
   while (cmp_result != 0) {
      if (cmp_result > 0) {
         mpz_sub(c, a, b);
         mpz_set(a, c);
      } else {
         mpz_sub(c, b, a);
         mpz_set(b,c);
      }
         
      cmp_result = mpz_cmp(a,b);
   }

   mpz_set(c,a);
}

// read RSA keys from file into a vector string for later conversion to mpz_t objects
int read_key_file(std::ifstream &key_file, std::vector<std::string> &key_list) {
   std::string key_str;

   while (std::getline(key_file, key_str)) 
      key_list.push_back(key_str);  

   return key_list.size();
}

// print to terminal the entire key set
void print_key_list(std::vector<std::string> &key_list) {
   for (auto it = key_list.begin();it != key_list.end(); it++) 
      std::cout << *it << std::endl;
}

// convert the key vector into an array of mpz_t objects
void str_to_gmp_list(std::vector<std::string> &str_pub_keys, mpz_t gmp_pub_keys[]) {
   //TODO Change the base value to hex when reading the actual files provided by allen
   for (int i = 0; i < str_pub_keys.size(); i++) {
      const char *key_str = str_pub_keys[i].c_str();
      mpz_init_set_str(gmp_pub_keys[i], key_str, 10);
   }
}

//clearn up an array of gmp objects
void clean_gmp_list(mpz_t gmp_pub_keys[], int num_keys) {
   for (int i = 0; i < num_keys; i++)
      mpz_clear(gmp_pub_keys[i]);
}
