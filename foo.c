main(){
char *foo = malloc(1); if (foo) { *foo = 'A'; (*foo)++; printf("foo=%c\n", *foo);  }
}
