automake
autoconf
./scripts/update_modules.pl
./configure LIBS='-lpthread -lboost_system' CFLAGS='-O3 -march=native -mfpmath=sse'

