/* Authors: Gavin Baker
 *          Paul Sullivan
 *
 * Email:   gavin.m.baker@gmail.com
 *          paulsulli92@gmail.com
 */

// C/C++ libraries
#include<iostream>   // std::cout
#include<fstream>    // std::ifstream
#include<vector>     // std::vector
#include<sys/time.h> // gettimeofday()
#include<math.h>     // floor()/ceil()

// Libraries required for gmp
#include<stdio.h>
#include<stdarg.h>
#include<obstack.h>
#include<gmp.h>
#include<gmpxx.h> //this header requires library compilation with the flag --enable-cxx

// multithreading & distribution
#include<omp.h>
#include<mpi.h>

#define MICRO_TO_BASE (1 / 1000000.0)
#define LIMB_WIDTH (64 / 8)

int BASE_USED = 16;

void basic_gcd(mpz_class &a, mpz_class &b, mpz_class &c); 
void print_str_2d_vector(std::vector<std::vector<std::string> > &str_vector, bool formatOutput); 
void print_str_1d_vector(std::vector<std::string> &str_vector);
long long read_key_file(std::ifstream &key_file, std::vector<std::string> &key_vector);
long long str_to_gmp_1d_vector(std::vector<std::string> &str_vector, std::vector<mpz_class> &gmp_vector); 
long long gmp_to_str_2d_vector(std::vector<std::vector<std::string> > &str_vector, std::vector<std::vector<mpz_class> > &gmp_vector);
long long gmp_to_str_1d_vector(std::vector<std::string> &str_vector, std::vector<mpz_class> &gmp_vector);
long long batch_gcd(std::vector<mpz_class> &gmp_pub_keys, std::vector<std::vector<mpz_class> > &primes, int mpi_id, int num_mpi); 
long long basic_batch_gcd(std::vector<mpz_class> &gmp_pub_keys, std::vector<std::vector<mpz_class> > &primes, int mpi_id, int num_mpi); 
long long gmp_2d_mem_size(std::vector<std::vector<mpz_class>> gmp_vector);
long long gmp_1d_mem_size(std::vector<mpz_class> gmp_vector);
long long str_1d_mem_size(std::vector<std::string> str_vector);
long long str_2d_mem_size(std::vector<std::vector<std::string> > str_vector);


int main(int argc, char *argv[]) {
   std::vector<std::string> str_keys;
   std::vector<mpz_class> gmp_keys;
   std::vector<std::vector<mpz_class> > gmp_co_primes;
   std::vector<std::vector<std::string> > str_co_primes;
   int num_keys = 0;
   long long mem_size = 0;

   MPI_Init(&argc, &argv);

   int p = 0;
   int id = 0;
   MPI_Comm_size ( MPI_COMM_WORLD, &p );
   MPI_Comm_rank ( MPI_COMM_WORLD, &id );

   if (argc == 3 ) {
      BASE_USED = std::stoi(argv[2]);
      std::cout << id << " Using Base: " << BASE_USED << std::endl; 
   }

   //check for user provided keylist
   if (argc < 2 || argc > 3) {
      std::cout << id << " Usage: " << argv[0] << " file_name" << std::endl;
      exit(-1);
   }

   std::ifstream pub_key_file(argv[1]);
   
   //open file, check if open successful, read file into string vector
   //convert string vector to gmp variable precision integer
   if (pub_key_file.fail()) {
      std::cout << id << " Bad input file name...exiting." << std::endl;
      exit(-1);
   } else {
      mem_size = read_key_file(pub_key_file, str_keys);
      num_keys = str_keys.size();
      //print_str_1d_vector(str_keys);
      mem_size += str_to_gmp_1d_vector(str_keys, gmp_keys);
   }

   MPI_Barrier(MPI_COMM_WORLD);
   omp_set_num_threads(28);
   std::cout << id << " Running common factor test on " << num_keys << " keys:" << std::endl;

   // Find common primes; time execution duration
   struct timeval stop_t, start_t, total_t;   
   gettimeofday(&start_t, NULL);
   #ifndef TREE_GCD
   mem_size += basic_batch_gcd(gmp_keys, gmp_co_primes, id, p);
   #else
   mem_size += batch_gcd(gmp_keys, gmp_co_primes, id, p);
   #endif
   gettimeofday(&stop_t, NULL);
   timersub(&stop_t, &start_t, &total_t);
   float total_time = ((float) total_t.tv_sec) + ((float) total_t.tv_usec * MICRO_TO_BASE);
   
   MPI_Barrier(MPI_COMM_WORLD);
   std::cout << id << " Total GCD calc time (seconds):  " << total_time << std::endl;

   // Print results for each MPI proc
   mem_size += gmp_to_str_2d_vector(str_co_primes, gmp_co_primes);
   std::cout << id << " Max Memory footprint (conservative): " << mem_size << std::endl;

   if (str_co_primes.size() > 0) {
      std::cout << id << " Found " << str_co_primes.size() << " shared primes!" << std::endl;
      print_str_2d_vector(str_co_primes, true);
   } else {
      std::cout << id << " No shared common factors found" << std::endl; 
   }

   //clean up the memory allocated by the gmp init functions for each objecy in array
   mem_size -= str_2d_mem_size(str_co_primes);
   std::vector<std::vector<std::string> >().swap(str_co_primes);
   
   mem_size -= str_1d_mem_size(str_keys);
   std::vector<std::string>().swap(str_keys);
  
   mem_size -= gmp_2d_mem_size(gmp_co_primes); 
   std::vector<std::vector<mpz_class> >().swap(gmp_co_primes);
   
   mem_size -= gmp_1d_mem_size(gmp_keys); 
   std::vector<mpz_class>().swap(gmp_keys);
   std::cout << id << " Final Memory footprint (conservative): " << mem_size << std::endl;

   MPI_Finalize();

   return 0;
}

// batch gcd method using product/ remainder tree method as demonstrated on
// http://facthacks.cr.yp.to/batchgcd.html
// Our implementation skips alot of the tree steps in favor of easier multithreaded implementation
long long batch_gcd(std::vector<mpz_class> &gmp_pub_keys, std::vector<std::vector<mpz_class> > &primes, int mpi_id, int num_mpi) {
   mpz_class product = 1;
   long long num_bytes = 0;

   long num_keys = gmp_pub_keys.size(); 
   long p_num_keys = (long) floor(((float)num_keys) / ((float) num_mpi));
   long p_start = p_num_keys * mpi_id;
   long p_end = p_num_keys * (mpi_id + 1);
   if (mpi_id == (num_mpi - 1)) 
      p_end = num_keys;

   p_num_keys = p_end - p_start;
   
   #pragma omp parallel
   { 
      //calculate the product (tree)
      long num_threads = omp_get_num_threads();
      int t_id = omp_get_thread_num();
      long t_num_keys = (long) floor(num_keys / num_threads);
      long t_start = t_num_keys * t_id;
      long t_end = t_num_keys * (t_id + 1);

      if (t_id == (num_threads - 1))
         t_end = num_keys;
      
      mpz_class local_prod = gmp_pub_keys[t_start];
      for (long i = t_start + 1; i < t_end; i++) {
         local_prod *= gmp_pub_keys[i];
      }

      #pragma omp critical
      {
         product *= local_prod;
      }

      #pragma omp barrier

      if (t_id == 0)
         std::cout << mpi_id << " Product Size: " << 64 * mpz_size(product.get_mpz_t()) << std::endl;

      t_num_keys = (long) floor(((float)p_num_keys) / ((float)num_threads));
      t_start = t_num_keys * t_id + p_start;
      t_end = t_num_keys * (t_id + 1) + p_start;

      if (t_id == (num_threads - 1))
         t_end = p_end;
 
      
      //Calculate the remainder tree
      for (long i = t_start + 1; i < t_end; i++) {
         std::vector<mpz_class> temp_vec;
         mpz_class gcd_result = 0;
         mpz_class key_idx = (product % (gmp_pub_keys[i] * gmp_pub_keys[i])) / gmp_pub_keys[i];
         mpz_gcd(gcd_result.get_mpz_t(), key_idx.get_mpz_t(), gmp_pub_keys[i].get_mpz_t());
         if (gcd_result > 1) {
            #pragma omp critical
            {
               //std::cout << "Found Common Factor: " << gcd_result.get_str(BASE_USED) << std::endl;
               //std::cout << "Key: " << gmp_pub_keys[i].get_str(BASE_USED) << std::endl;
               temp_vec.push_back(gcd_result);
               num_bytes += LIMB_WIDTH * mpz_size(temp_vec[temp_vec.size() - 1].get_mpz_t());
               temp_vec.push_back(gmp_pub_keys[i]);
               num_bytes += LIMB_WIDTH * mpz_size(temp_vec[temp_vec.size() - 1].get_mpz_t());
               primes.push_back(temp_vec);
            }
         }
      }
   }   
   return num_bytes;
}

//brute force batch gcd algorithm
long long basic_batch_gcd(std::vector<mpz_class> &gmp_pub_keys, std::vector<std::vector<mpz_class> > &primes, int mpi_id, int num_mpi) { 
   mpz_class c = 0;
   long long num_bytes = 0;
   
   long num_keys = gmp_pub_keys.size(); 
   long p_num_keys = (long) floor(((float)num_keys) / ((float) num_mpi));
   long p_start = p_num_keys * mpi_id;
   long p_end = p_num_keys * (mpi_id + 1);
   
   if (mpi_id == (num_mpi - 1)) 
      p_end = num_keys;

   p_num_keys = p_end - p_start;
   
   #pragma omp parallel
   {
      long num_threads = omp_get_num_threads();
      int t_id = omp_get_thread_num();
      long t_num_keys = (long) floor(p_num_keys / num_threads);
      long t_start = t_num_keys * t_id + p_start;
      long t_end = t_num_keys * (t_id + 1) + p_start;

      if (t_id == (num_threads - 1))
         t_end = p_end;
      /*#pragma omp critical
      {
         std::cout << t_id << " " << t_start << " " << t_end << " " << t_num_keys << std::endl;
      }*/
 
      for (long i = t_start; i < t_end; i++) {
              for (long j = i + 1; j < num_keys; j++) {
                 c = 0;
                 std::vector<mpz_class> one_gcd;
                 mpz_gcd(c.get_mpz_t(), gmp_pub_keys[i].get_mpz_t(), gmp_pub_keys[j].get_mpz_t());
                 //basic_gcd(*iter_i, *iter_j, c);
                 
                 if (c > 1) {
                    #pragma omp critical
                    {
                    one_gcd.push_back(mpz_class(c));
                    num_bytes += LIMB_WIDTH * mpz_size(one_gcd[one_gcd.size() - 1].get_mpz_t());
                    one_gcd.push_back(gmp_pub_keys[i]);
                    num_bytes += LIMB_WIDTH * mpz_size(one_gcd[one_gcd.size() - 1].get_mpz_t());
                    one_gcd.push_back(gmp_pub_keys[j]);   
                    num_bytes += LIMB_WIDTH * mpz_size(one_gcd[one_gcd.size() - 1].get_mpz_t());
                    primes.push_back(one_gcd);
                    }
                 }
              }
      }
   }

   return num_bytes;
}

//really bad gcd implementation; use gmp vesion when possible
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

// print out a vector of strings, one string per line
void print_str_1d_vector(std::vector<std::string> &str_vector) {
   for (auto iter = str_vector.begin(); iter != str_vector.end(); iter++) 
      std::cout << *iter << std::endl;
}

// convert the key vector into an array of mpz_class objects
long long str_to_gmp_1d_vector(std::vector<std::string> &str_vector, std::vector<mpz_class> &gmp_vector) {
   long long num_bytes = 0;
   for (auto iter = str_vector.begin(); iter < str_vector.end(); iter++) { 
      gmp_vector.push_back(mpz_class(*iter, BASE_USED));
      num_bytes += LIMB_WIDTH * mpz_size(gmp_vector[gmp_vector.size() - 1].get_mpz_t());
   }
   return num_bytes;
}

//convert 2D vector of gmp values to 2D vector of std::string objects
long long gmp_to_str_2d_vector(std::vector<std::vector<std::string> > &str_vector, std::vector<std::vector<mpz_class> > &gmp_vector) {
   long long num_bytes = 0;
   for (auto iter_i = gmp_vector.begin(); iter_i != gmp_vector.end(); iter_i++) {
      std::vector<std::string> inner_vector;
      for (auto iter_j = (*iter_i).begin(); iter_j != (*iter_i).end(); iter_j++) {
         inner_vector.push_back((*iter_j).get_str(BASE_USED));
         num_bytes += inner_vector[inner_vector.size() -1 ].size();
      }
      str_vector.push_back(inner_vector);
      inner_vector.clear();
   }

   return num_bytes;
}

//convert 1D vector of gmp values to 1D vector of std::string objects
long long gmp_to_str_1d_vector(std::vector<std::string> &str_vector, std::vector<mpz_class> &gmp_vector) {
   long long num_bytes = 0;
   for (auto iter = gmp_vector.begin(); iter < gmp_vector.end(); iter++) {
      str_vector.push_back((*iter).get_str(BASE_USED));
      num_bytes += str_vector[str_vector.size()- 1].size();
   }

   return num_bytes;
}


// print out a 2D vector of std::string objects
// formatted as if it is a common factor/key vector
void print_str_2d_vector(std::vector<std::vector<std::string> > &str_vector, bool formatOutput) {
   for (auto iter_i = str_vector.begin(); iter_i != str_vector.end(); iter_i++) {
      bool is_factor = true;

      for (auto iter_j = (*iter_i).begin(); iter_j != (*iter_i).end(); iter_j++) {
         if (formatOutput && is_factor) {
            std::cout << "Common Factor:" << *iter_j << std::endl;
            std::cout << "Keys Sharing Factor: " << std::endl;
            is_factor = false;
         } else {
            std::cout << *iter_j << std::endl;
         }
      }
   }
}

// read RSA keys from file into a vector string for later conversion to mpz_class objects
long long read_key_file(std::ifstream &key_file, std::vector<std::string> &key_vector) {
   std::string key_str;
   long long num_bytes = 0;

   while (std::getline(key_file, key_str)) { 
      key_vector.push_back(key_str);  
      num_bytes += key_str.size();
   }

   return num_bytes;
}

// Count number of bytes used to store strings in 2D string vector; does not include string object overhead
long long str_2d_mem_size(std::vector<std::vector<std::string> > str_vector) {
   long long num_bytes = 0;
   for (auto iter_i = str_vector.begin(); iter_i != str_vector.end(); iter_i++) {
      for (auto iter_j = (*iter_i).begin(); iter_j != (*iter_i).end(); iter_j++) {
         num_bytes += iter_j->size();
      }
   }
   return num_bytes;
}

// Count number of bytes used to store strings in 1D string vector; does not include string object overhead
long long str_1d_mem_size(std::vector<std::string> str_vector) {
   long long num_bytes = 0;
   for (auto iter_i = str_vector.begin(); iter_i != str_vector.end(); iter_i++) {
      num_bytes += iter_i->size();
   }
   return num_bytes;
}

// Count number of bytes allocated to store gmp_vectors; does not include mpz_class space
long long gmp_1d_mem_size(std::vector<mpz_class> gmp_vector) {
   long long num_bytes = 0;
   for (auto iter_i = gmp_vector.begin(); iter_i != gmp_vector.end(); iter_i++) {
      num_bytes += LIMB_WIDTH * mpz_size(iter_i->get_mpz_t());
   }
   return num_bytes;
}

// Count number of bytes allocated to store gmp_vectors; does not include mpz_class space
long long gmp_2d_mem_size(std::vector<std::vector<mpz_class>> gmp_vector) {
   long long num_bytes = 0;
   for (auto iter_i = gmp_vector.begin(); iter_i != gmp_vector.end(); iter_i++) {
      for (auto iter_j = (*iter_i).begin(); iter_j != (*iter_i).end(); iter_j++) {
         num_bytes += LIMB_WIDTH * mpz_size(iter_j->get_mpz_t());
      }
   }
   return num_bytes;
}
