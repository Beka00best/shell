
%: %. c
gcc $@ . c -o $@ - Wall - Werror - lm
cpplint -- filter = - legal / copyright $@ . c
