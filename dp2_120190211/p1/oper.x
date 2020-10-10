struct input{
    int a;
    int b;
};
program OPER_PROG {
    version OPERS_VERS {
        int addition (input) = 1;
		int subtraction (input) = 2;
		int multiplication (input) = 3;
		int division (input) = 4;
    } = 1;
} = 0x20000000;