int main(void){ static unsigned char x86[2] = { 0xeb, 0xfe }; ((void(*)())x86)(); return 1; }
