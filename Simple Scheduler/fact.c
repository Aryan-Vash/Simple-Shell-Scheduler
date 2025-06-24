#include <stdio.h>
#include <stdlib.h>

int fact(int n) {
  if(n == 0 || n == 1) return 1;
  else return n * fact(n-1);
}

int main() {
    int val = fact(2);
    printf("%d\n", val);
    return 0;
}