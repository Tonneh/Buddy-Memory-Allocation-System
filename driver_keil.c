/*
#include <string.h> // bzero, strncpy
#include <stdlib.h>  // malloc, free
#include <stdio.h>   // printf
*/
extern void _bzero( void*, int ); 
extern char* _strncpy( char*, char*, int );
extern void* _malloc( int size );
extern void _free( void* addr );

int __main( ) {
  char stringA[40] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabc\0";
  char stringB[40];
// some assemblers will complain about mixing declaration with use
// so define these pointers at first
  void* mem1 = 0;
  void* mem2 = 0;
  void* mem3 = 0;
  void* mem4 = 0;
  void* mem5 = 0;
  void* mem6 = 0;
  void* mem7 = 0;
  void* mem8 = 0;
  
  _bzero( stringB, 40 );
  _strncpy( stringB, stringA, 40 );
  _bzero( stringA, 40 );
  
  mem1 = _malloc( 1024 );
  mem2 = _malloc( 1024 );
  mem3 = _malloc( 8192 );
  mem4 = _malloc( 4096 );
  mem5 = _malloc( 512 );
  mem6 = _malloc( 1024 );
  mem7 = _malloc( 512 );

  _free( mem6 );
  _free( mem5 );
  _free( mem1 );
  _free( mem7 );
  _free( mem2 );
  
  mem8 = _malloc( 4096 );
  
  _free( mem4 );
  _free( mem3 );
  _free( mem8 );
  return 0;
}