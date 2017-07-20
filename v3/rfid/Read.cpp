#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

void delay(int ms){
#ifdef WIN32
  Sleep(ms);
#else
  usleep(ms*1000);
#endif
}

#include "MFRC522.h"

int main(){
  MFRC522 mfrc;

  mfrc.PCD_Init();
printf("init \n");

//mfrc.PCD_Reset();

  while(1){
    // Look for a card
    if(!mfrc.PICC_IsNewCardPresent()){
delay(10);     
//return 0;
continue;
     }

printf("new card\n");

    if( !mfrc.PICC_ReadCardSerial())
      continue;

    // Print UID
    for(byte i = 0; i < mfrc.uid.size; ++i){
      if(mfrc.uid.uidByte[i] < 0x10){
	printf(" 0");
	printf("%X",mfrc.uid.uidByte[i]);
      }
      else{
	printf(" ");
	printf("%X", mfrc.uid.uidByte[i]);
      }
      
    }
    printf("\n");
    delay(1000);
  }
  return 0;
}
