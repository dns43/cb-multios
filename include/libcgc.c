/* Copyright 2015 Peter Goodman (peter@trailofbits.com), all rights reserved. */

#define LIBCGC_IMPL
#include "libcgc.h"
#include "ansi_x931_aes128.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/select.h>
//#include <sys/types.h>
#include <unistd.h>
#include <err.h>
//#include <sanitizer/dfsan_interface.h>




#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) < (b)) ? (b) : (a))

/* Terminates the process. */
void cgc__terminate(unsigned int status) {
  exit(status);
  __builtin_unreachable();
}

/* Transmits data from one CGC process to another. */
int cgc_transmit(int fd, const void *buf, cgc_size_t count, cgc_size_t *tx_bytes) {
    const cgc_ssize_t ret = write(fd, buf, count);

    #ifdef LOG_COMM
    /*
     * Creates a file with all transmitted data
     * Helps debugging communication between CB and POV
     * E.g. if the negotiation is in sync
     */
      FILE *fp;
      //char buff[255]
      fp = fopen("cb_comm.csv", "ab");
      write(fileno(fp), buf, count);
      close(fp);
    #endif

    if (ret < 0) {
        return errno;
    } else if (tx_bytes != NULL) {
        *tx_bytes = ret;
    }
	
    

    return 0;
}

typedef uint16_t dfsan_label;
dfsan_label dfsan_create_label(const char *, float); 
void dfsan_set_label(dfsan_label, void *, unsigned long);


/* Receives data from another CGC process. */
int cgc_receive(int fd, void *buf, cgc_size_t count, cgc_size_t *rx_bytes) {
    const cgc_ssize_t ret = read(fd, buf, count);

	float init = 1.0;
	dfsan_label lbla = dfsan_create_label("input_char", init); 
	dfsan_set_label(lbla, buf, sizeof(char));
/*
	dfsan_label lblb = dfsan_create_label("&buf", init); 
	dfsan_set_label(lblb, &buf, sizeof(char));

	//dfsan_label albl = dfsan_get_label(buf); 
	const struct dfsan_label_info *info = dfsan_get_label_info(lblb); 
	printf("\n In Function \n pos %f, neg: %f \n \n ", info->pos_dydx, info->neg_dydx);
*/

    if (ret < 0) {
        return errno;
    } else if (rx_bytes != NULL) {
        *rx_bytes = ret;
    }
/*
	dfsan_label lbl = dfsan_create_label("rx_char", init); 
	dfsan_set_label(lbl, rx_bytes, sizeof(rx_bytes));
*/
    return 0;
}

/* Marshal a CGC fd set into an OS fd set. */
static int cgc_copy_cgc_fd_set(const cgc_fd_set *cgc_fds, fd_set *os_fds, int *num_fds) {
  for (unsigned fd = 0; fd < CGC__NFDBITS; ++fd) {
    if (CGC_FD_ISSET(fd, cgc_fds)) {
      // Shouldn't be using an fd greater than the allowed values
      if (fd >= EXPECTED_MAX_FDS) {
          return CGC_EBADF;
      }

      if (fd > NFDBITS) {
        continue;  /* OS set size is too small. */
      }
      FD_SET(fd, os_fds);
      ++*num_fds;
    }
  }
  return 0;
}

/* Marshal an OS fd set into a CGC fd set. */
static void cgc_copy_os_fd_set(const fd_set *os_fds, cgc_fd_set *cgc_fds) {
  for (unsigned fd = 0; fd < MIN(NFDBITS, CGC__NFDBITS); ++fd) {
    if (FD_ISSET(fd, os_fds)) {
      CGC_FD_SET(fd, cgc_fds);
    }
  }
}
int cgc_fdwait(int nfds, cgc_fd_set *readfds, cgc_fd_set *writefds,
               const struct cgc_timeval *timeout, int *readyfds) {

  int ret = cgc_check_timeout(timeout);
  int actual_num_fds = 0;
  struct timeval max_wait_time = {0, 0};
  fd_set read_fds;
  fd_set write_fds;

  if (ret) {
    return ret;
  } else if (0 > nfds || CGC__NFDBITS < nfds) {
    return CGC_EINVAL;
  }

  FD_ZERO(&read_fds);
  FD_ZERO(&write_fds);

  if (readfds) {
    if (0 != (ret = cgc_copy_cgc_fd_set(readfds, &read_fds, &actual_num_fds))) {
      return ret;
    }
  }

  if (writefds) {
    if (0 != (ret = cgc_copy_cgc_fd_set(writefds, &write_fds, &actual_num_fds))) {
      return ret;
    }
  }

  if (actual_num_fds != nfds) {
    return EINVAL;  /* Not actually specified, but oh well. */
  }

  if (readfds)  CGC_FD_ZERO(readfds);
  if (writefds) CGC_FD_ZERO(writefds);

  if (timeout) {
    max_wait_time.tv_sec = timeout->tv_sec;
    max_wait_time.tv_usec = timeout->tv_usec;
  }

  int num_selected_fds = select(
          nfds,
          (readfds ? &read_fds : NULL),
          (writefds ? &write_fds : NULL),
          NULL,
          (timeout ? &max_wait_time : NULL));

  if (num_selected_fds < 0)
    return errno;

  if (readfds) {
    cgc_copy_os_fd_set(&read_fds, readfds);
  }

  if (writefds) {
    cgc_copy_os_fd_set(&write_fds, writefds);
  }

  if (readyfds) {
    *readyfds = num_selected_fds;
  }

  return 0;
}

/* Going to ignore `is_executable`. It's not really used in the official CGC
 * challenges, and if it were used, then JITed code would likely be 32-bit, and
 * ideally, this code will also work on 64-bit.
 */
int cgc_allocate(cgc_size_t length, int is_executable, void **addr) {
  int page_perms = PROT_READ | PROT_WRITE;
  if (is_executable)
    page_perms |= PROT_EXEC;

  void *return_address = mmap(NULL, length, page_perms, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);

  if (return_address == MAP_FAILED) {
    return errno;
  }

  if (addr)
    *addr = return_address;

  memset(return_address, 0, length);

  return 0;
}

/* Deallocate some range of memory and mark the pages as free. */
int cgc_deallocate(void *addr, cgc_size_t length) {

  const int ret = munmap(addr, length);

  if (ret < 0) {
    return errno;
  }

  return 0;
}


static cgc_prng *cgc_internal_prng = NULL;
/**
 * Initializes the prng for use with cgc_random and the flag page
 */
static void cgc_try_init_prng() {
    // Don't reinitialize
    if (cgc_internal_prng != NULL) return;

    uint8_t prng_seed[BLOCK_SIZE * 3] = {
        0x73, 0x65, 0x65, 0x64, 0x73, 0x65, 0x65, 0x64, 0x73, 0x65, 0x65, 0x64,
        0x73, 0x65, 0x65, 0x64, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    }; // Default seed, definitely not random

    // This will be hex encoded
    const char *prng_seed_hex = getenv("seed");
    if (prng_seed_hex != NULL && strlen(prng_seed_hex) == (BLOCK_SIZE * 3) * 2) {
        // Convert the hex encoded seed to a normal string
        const char *pos = prng_seed_hex;
        for(int i = 0; i < BLOCK_SIZE * 3; ++i) {
            sscanf(pos, "%2hhx", &prng_seed[i]);
            pos += 2;
        }
    }


    // Create the prng
    cgc_internal_prng = (cgc_prng *) malloc(sizeof(cgc_prng));
    cgc_aes_state *seed = (cgc_aes_state *) prng_seed;
    cgc_init_prng(cgc_internal_prng, seed);
}

int cgc_random(void *buf, cgc_size_t count, cgc_size_t *rnd_bytes) {
    // Get random bytes from the prng
    cgc_try_init_prng();
    cgc_aes_get_bytes(cgc_internal_prng, count, buf);

    if (rnd_bytes)
      *rnd_bytes = count;

    return 0;
}

static void __attribute__ ((constructor)) cgc_initialize_flag_page(void) {
  void *mmap_addr = mmap(CGC_FLAG_PAGE_ADDRESS, PAGE_SIZE,
                         PROT_READ | PROT_WRITE,
                         MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS,
                         -1, 0);

  if (mmap_addr != CGC_FLAG_PAGE_ADDRESS) {
    err(1, "[!] Failed to map the flag page");
  }

  // Fill the flag page with bytes from the prng
  cgc_try_init_prng();
  cgc_aes_get_bytes(cgc_internal_prng, PAGE_SIZE, mmap_addr);

  /*
   * This constructor is run at the beginning of each execution
   * It initializes the flag pages with random variables by
   * Running AES over a random seed
   * In the future, random values can be "generated" by picking values from this page
   * 
   * We allow for printing it to file
   * such that we can compare if tester.py's random seed generator and AES implementation match
   * 
   */
  #ifdef LOG_RANDOM_PAGE
    FILE *fp;
    fp = fopen("cb_random_page.csv", "ab");
    write(fileno(fp), mmap_addr, PAGE_SIZE);
    close(fp);
  #endif
}
