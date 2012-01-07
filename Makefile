CXX			=	clang++
CXXFLAGS	=	-march=native -std=c++98 -pedantic -O0 -g
#CXXFLAGS	=	-O2 -ansi -pedantic -W -Wall -Wextra -Wshadow -Wformat -Winit-self -Wunused -Wfloat-equal -Wcast-qual -Wwrite-strings -Winline -Wstack-protector -Wunsafe-loop-optimizations -Wlogical-op -Wjump-misses-init -Wmissing-include-dirs -Wconversion -Wmissing-prototypes -Wmissing-declarations
LDFLAGS		=	-lm -L/usr/tools/lib

SRC			=	dlx.cpp
.PHONY:		clean all doc

all: $(SRC:%.cpp=%)
	ctags -R --c++-kinds=+p --fields=+iaS --extra=+q .
	echo $? | sed 's/ /\n/g' > .gitignore
	cat .gitignore.base >> .gitignore

doc: $(SRC)
	doxygen

clean:
	rm -rf *.o *.so *.a *~ *swp .*swp *.tmp core core.* $(SRC:%.cpp=%) *.out tags TAGS

