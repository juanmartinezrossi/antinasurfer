#include <stdio.h>
#include <stdlib.h>

#include "phal_hw_api.h"


#define PACKET_SZ	1024
#define PACKET_X_TSLOT	1

char buffer[PACKET_SZ];

void main() 
{
  int fd_r,fd_w;
  int i,j,checksum;
  
  if (!hwapi_init()) {
    printf("Error initiaitng hwapi\n");
    exit(0);
  }
  
  if(!utils_create_attach_ext_bi(0x20,1024,&fd_w,&fd_r)) {
    printf("errror creating ext itf\n");
    exit(0);
  }
  
  while(1) 
  {
    for (j=0;j<PACKET_X_TSLOT;j++) {
      checksum=0;
      for (i=0;i<PACKET_SZ;i++) {
        buffer[i] = (char) rand();
        checksum += buffer[i];
      }
    
      if (hwapi_itf_snd(fd_w,buffer,1024)<0) {
        printf("ERror sending\n");
        exit(0);
      }
    
      if (hwapi_itf_snd(fd_w,&checksum,4)<0) {
        printf("error sending\n");
        exit(0);
      }
    
    }
    hwapi_relinquish_daemon(1);
  }
  
}
    
    
    