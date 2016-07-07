#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/if_alg.h>
#include <linux/socket.h>

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <openssl/sha.h>

#ifndef SOL_ALG
#define SOL_ALG 279
#endif

int main(void)
{
  int opfd;
  int tfmfd;
  struct sockaddr_alg sa = {
    .salg_family = AF_ALG,
    .salg_type = "hash",
    .salg_name = "sha256"
  };
  int i;
  unsigned char buf[32];

  tfmfd = socket(AF_ALG, SOCK_SEQPACKET, 0);

  bind(tfmfd, (struct sockaddr *)&sa, sizeof(sa));

  opfd = accept(tfmfd, NULL, 0);

  int buffer_size = 4096;
  unsigned char* init_input = malloc(buffer_size);
  unsigned char* input = init_input;

  int iteration = 10;
  int total = 0;
  int j;
  int ret;

  for (j = 0; j < iteration; ++j) {
    int input_size = rand() % (buffer_size / iteration);
     for (i = 0; i < input_size; ++i) input[i] = rand(); 


#if 0
    ret = write(opfd, input, input_size); 
    assert(ret > 0 && "write failed");
#else
  struct msghdr msg = {};
  struct iovec iov;
  msg.msg_control = NULL;
  msg.msg_controllen = 0;
  iov.iov_base = input;
  iov.iov_len = input_size;

  msg.msg_flags = MSG_MORE;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  sendmsg(opfd, &msg, 0);

#endif

    unsigned char last_digest[32];
    SHA256(input, input_size, last_digest);
    printf("last digest   ");
    for (i = 0; i < 32; i++) {
      printf("%02x", (unsigned char)last_digest[i]);
    }
    printf("\n");

    input += input_size;
    total += input_size;

  }
  // computing expected 
  unsigned char expected_digest[32];
  SHA256(init_input, total, expected_digest);

  ret = read(opfd, buf, 32);
  assert(ret > 0 && "read failed");

  printf("result   ");
  for (i = 0; i < 32; i++) {
    printf("%02x", (unsigned char)buf[i]);
  }
  printf("\n");
  printf("expected ");
  for (i = 0; i < 32; i++) {
    printf("%02x", (unsigned char)expected_digest[i]);
  }
  printf("\n");

  close(opfd);
  close(tfmfd);

  assert(!memcmp(expected_digest, buf, 32) && "digest differs from expected");
  free(init_input);

  return 0;
}