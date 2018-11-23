@md dumps 2> nul
@clang-check -ast-dump testfile.cpp -- > dumps\testfile.cpp.ast.txt
@clang-check -ast-dump testfile.hpp -- > dumps\testfile.hpp.ast.txt
@clang-check -ast-list testfile.cpp -- > dumps\testfile.cpp.list.txt
@clang-check -ast-list testfile.hpp -- > dumps\testfile.hpp.list.txt
@clang-check -ast-print testfile.cpp -- > dumps\testfile.cpp.print.txt
@clang-check -ast-print testfile.hpp -- > dumps\testfile.hpp.print.txt